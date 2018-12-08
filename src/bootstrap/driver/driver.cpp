
#if (true)

#include "parser/parser.hpp"

inline void warn_bad_compiler() { 
	printf("This is the ivy-lang bootstrap compiler.\n");
	printf("While we are thankful that you have decided to use it,\nwe must inform you that it is very much work in progress, and isn't going to be functional any time soon!\n");
	printf("If you still want to try it out, you can use the '-nowarn' option. Though, we can't give any guarantees that the build will be successful.\n");
	printf("You are welcome to wait, or partake in it's development on GitHub!\n");
	std::exit(0);
}

inline void usage(char* arg0) {
	printf("usage: %s [options] <input_files>\n", arg0);
	printf("options:\n");
	printf("	-o <filename>	write output to <filename>\n");
	printf("	-nowarn			suppress bad_compiler warning\n");
	printf("	-h				display help menu\n\n");
	std::exit(0);
}

void compile(const std::vector<std::string>& input, const std::string& output) {
	// TODO: all of the input files need to be parsed
	Parser parser = Parser(input[0]);

	if (Session::handler.has_errors()) {
		Session::handler.emit_delayed();
	}

	Session::msg("output set to " + output);

	// TODO: Store the AST
	parser.parse();
}

inline std::string dir_from_path(const std::string& path) {
	size_t last_slash_index = 0;
	for (size_t i = 0; i < path.length(); i++)
		if (path[i] == '/')
			last_slash_index = i;
	return path.substr(0, last_slash_index + 1);
}

int main(int argc, char* argv[]) {

	// TODO: Improve Config auto-generation
	Session::set_system_cfg(SysConfig());

	std::vector<std::string> input_files;
	std::string output_file;
	bool warn = true;

	const std::string curr_path = argv[0];

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
			if (arg == "-nowarn") {
				warn = false;
				continue;
			}

			// Enable trace messages
			if (arg == "-trace") {
				Session::enable_trace();
				continue;
			}

			// Set required severity level for message writing
			if (arg == "-require_sev") {
				// If there are more options and the next one doesn't start with '-',
				// set the requred severity limit
				if (i + 1 <= argc && argv[i+1][0] != '-') {
					arg = argv[++i];
					if (arg == "trace")
						Session::require_severity(Severity::TRACE);
					else if (arg == "message")
						Session::require_severity(Severity::MESSAGE);
					else if (arg == "warning")
						Session::require_severity(Severity::WARNING);
					else if (arg == "error")
						Session::require_severity(Severity::ERROR);
					else 
						Session::err("-require_sev invalid argument");
					continue;
				}
				else Session::err("-require_sev requires an argument" + arg);
			}

			// Set output file
			if (arg == "-o") {
				// If there are more options and the next one doesn't start with '-',
				// set the output file
				if (i + 1 <= argc && argv[i+1][0] != '-') {
					output_file = argv[++i];
					continue;
				}
				else Session::err("-o requires an argument" + arg);
			}

			// Invalid option
			else Session::err("unrecognised option: " + arg);
		}
		// Handle input files
		else {
			// Input files should be the last things provided
			while (i < argc) {
				if (argv[i][0] == '-')
					Session::err(std::string("bad option '") + argv[i] + "'; options should be placed before input files");
				input_files.push_back(argv[i]);
				i++;
			}
		}
	}

	// Display warning about this being a non-functional compiler
	if (warn) warn_bad_compiler();

	// Error if there is no input file
	if (input_files.size() == 0)
		Session::err("no input files were specified");

	// No output file specified. Use current location and 'a.out'
	if (output_file.empty() || output_file == ".")
		output_file = dir_from_path(curr_path) + "a.out";
	// Output file is relative. Prefix it with the current location
	else if (output_file[0] != '/')
		output_file = dir_from_path(curr_path) + output_file;
	else if (output_file[0] == '.' && output_file[1] == '/')
		output_file = dir_from_path(curr_path) + output_file.substr(2);

	compile(input_files, output_file);
}

#else

#include "tests/lexer_tests.hpp"
#include "parser/parser.hpp"
#include "util/token_info.hpp"

void print_main() {
	ErrorHandler handler;
	SourceMap sm(handler);
	Lexer lex(sm.load_file("tests/main.ivy"));

	Token tk = lex.next_token();
	while (tk != TokenType::END) {
		printf("%s\n", translate::tk_info(tk).c_str());
		tk = lex.next_token();
	}
}

int main() {

	print_main();

	tests::lexer::token_has_correct_absolute_pos();
	tests::lexer::token_has_correct_line_pos();
	tests::lexer::token_has_correct_column_pos();
	tests::lexer::return_eof_without_translation_unit();

	ErrorHandler handler;
	SourceMap sm(handler);
	Lexer lex(sm.load_file("tests/main.ivy"));
	Token tk = lex.next_token();

	Error err = handler.new_error(ErrSeverity::ERROR, std::string(tk.raw()));
	err.add_span(tk.span());
	err.add_highlight();
	err.add_help("if you wanted to import a module, use 'import mod'");
	err.add_note("this is a fake error");
	handler.emit(err);
}

#endif