// The Driver takes care of managing all of the compiler's parts
// and letting them work together.
// It currently handles responding to command line parameters,
// interfacing with the current Session and error systems and
// driving the compilation process forward.

// To use the main compiler entry point, set this to 'true'
// To use a miniature testing suite, set this to 'false'
#define MAIN_ENTRY true

#if (MAIN_ENTRY)

#include "parser/parser.hpp"

inline void warn_bad_compiler() { 
	printf("This is the ivy-lang bootstrap compiler.\n");
	printf("While we are thankful that you have decided to use it,\nwe must inform you that it is very much work in progress, and isn't going to be functional any time soon!\n");
	printf("If you still want to try it out, you can use the '-force' option. Though, we can't give any guarantees that the build will be successful.\n");
	printf("You are welcome to wait, or partake in it's development on GitHub!\n");
	std::exit(0);
}

inline void usage(char* arg0) {
	printf("usage: %s [options] <input_files>\n", arg0);
	printf("options:\n");
	printf("    -o <filename>   write output to <filename>\n");
	printf("    -nowarn         suppress bad_compiler warning\n");
	printf("    -h              display help menu\n\n");
	std::exit(0);
}

bool compile(const std::vector<std::string>& input, const std::string& output) {
	// TODO: all of the input files need to be parsed

	printf("-- output set to %s\n", output.c_str());

	try {
		// Must exist outside the Parser
		SourceMap src_map = SourceMap(Session::handler);

		// TODO:  Store the AST
		// FIXME:  This is in a try-catch because 'expressions not implemented yet'
		try {
			Parser parser = Parser(src_map, input[0]);
			parser.parse();
		}
		catch (const InternalException& e) {}

		if (Session::handler.recount_errors() > 0)
			Session::handler.emit_delayed();

		if (Session::emitter.num_err_emitted() > 0) {
			// Build error count string
			std::string err_count = Session::emitter.num_err_emitted() != 1 ?
				std::to_string(Session::emitter.num_err_emitted()) + " errors" :
				"1 error";

			Session::handler.emit_fatal("build failed due to " + err_count + "\n");
		}
	}
	// If anything throwed an internal exception,
	// Return compilation failure
	catch (const FatalException& e) {
		return false;
	}
	// If anything throwed an internal exception, it was most likely a 'bug' or 'unimpl'
	// Return compilation failure
	catch (const InternalException& e) {
		return false;
	}
	
	// No unhandled exceptions
	// Return success
	return true;
}

int main(int argc, char* argv[]) {

	std::vector<std::string> input_files;
	std::string output_file;
	bool bad_comp = true;

	const std::string cwd = Session::get_cwd();

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		// Handle options
		if (arg[0] == '-') {
			// Show help menu
			if (arg == "-h") {
				usage(argv[0]);
				return 0;
			}

			// Disable bad-compiler warning
			if (arg == "-force") {
				bad_comp = false;
				continue;
			}

			// Enable trace messages
			if (arg == "-trace") {
				Session::handler.flags.trace = true;
				continue;
			}

			// Enable trace messages
			if (arg == "-nowarn") {
				Session::handler.flags.no_warnings = true;
				continue;
			}

			// Enable trace messages
			if (arg == "-Werr") {
				Session::handler.flags.warnings_as_err = true;
				continue;
			}

			// Set output file
			if (arg == "-o") {
				// If there are more options and the next one doesn't start with '-',
				// set the output file
				if (i + 1 <= argc && argv[i+1][0] != '-') {
					output_file = argv[++i];
					continue;
				}
				else {
					printf("-o requires an argument");
					return EXIT_FAILURE;
				}
			}

			// Invalid option
			else {
				printf("unrecognised option: %s\n", arg.c_str());
				return EXIT_FAILURE;
			}
		}
		// Collect input files
		else {
			input_files.push_back(argv[i]);
		}
	}

	// Display warning about this being a non-functional compiler
	if (bad_comp)
		warn_bad_compiler();

	// Error if there is no input file
	if (input_files.size() == 0) {
		printf("input files missing\n");
		return EXIT_FAILURE;
	}

	// In case no output was specified or it's just '.',
	// use the cwd and an 'a.out' file
	if (output_file.empty() || output_file == ".")
		output_file = cwd + "/a.out";
	// In case the given output is relative by './',
	// use the cwd and an 'a.out' file
	else if (output_file[0] == '.' && output_file[1] == '/') {
		if (output_file.length() != 2)
			output_file = cwd + output_file.substr(1);
		else
			output_file = cwd + "/a.out";
	}
	// In case the given output doesn't start from the root
	// prepend ir with the cwd
	else if (output_file[0] != '/')
		output_file = cwd + output_file;
	// In case we are given an output directory but no file,
	// append an 'a.out' file
	else if (output_file[output_file.length() - 1] == '/')
		output_file += "/a.out";

	return compile(input_files, output_file);
}


#else // MAIN_ENTRY

#include "tests/lexer_tests.hpp"
#include "parser/parser.hpp"
#include "util/token_info.hpp"

void print_main() {
	Emitter emitter;
	ErrorHandler handler(emitter);
	SourceMap sm(handler);
	Lexer lex(sm.load_file("tests/main.ivy"), handler);

	Token tk = lex.next_token();
	while (tk != TokenType::END) {
		printf("%s\n", translate::tk_info(tk).c_str());
		tk = lex.next_token();
	}
}

int main() {

	// Prints the tokens in the 'main.ivy' test file
	print_main();

	// Run thests
	// TODO:  There should be more but we're short on time
	tests::lexer::token_has_correct_absolute_pos();
	tests::lexer::token_has_correct_line_pos();
	tests::lexer::token_has_correct_column_pos();
	tests::lexer::return_eof_without_translation_unit();

	// Check error handling
	// TODO:  Should be moved to a test function at some point
	SourceMap sm(Session::handler);
	Lexer lex(sm.load_file("tests/main.ivy"), Session::handler);
	Token tk = lex.next_token();

	Error err = Session::handler.new_error(Severity::ERROR, std::string(tk.raw()), tk.span(), 0);
	err.add_span();
	err.add_highlight();
	err.add_help("if you wanted to import a module, use 'import mod'");
	err.add_note("this is a fake error");

	try { Session::handler.emit(err); }
	catch (...) {}
}

#endif