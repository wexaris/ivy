#include "parser/parser.hpp"
#include "session.hpp"
#include <iterator>

SysConfig Session::cfg;
bool Session::_verbose = false;
bool Session::_format = true;
std::string Session::indent_prefix;

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
	// FIXME: all of the input files need to be parsed
	Parser parser = Parser(input[0]);

	Session::msg("output set to " + output);

	// FIXME: Store the AST
	parser.parse();
}

inline std::string dir_from_path(const std::string& path) {
	size_t last_slash_index = 0;
	for (size_t i = 0; i < path.length(); i++)
		if (path[i] == '/')
			last_slash_index = i;
	return path.substr(0, last_slash_index + 1);
}

#if (false)

int main(int argc, char* argv[]) {

	// TODO: Improve Config auto-generation
	Session::setup(SysConfig());

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

			// Enable verbose logging
			if (arg == "-v") {
				Session::set_verbose(true);
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

int main() {

	Lexer::test_print_tokens();
	Lexer::test_read_without_source();
}

#endif