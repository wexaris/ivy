#include "emitter.hpp"
#include "source/translation_unit.hpp"

std::string Emitter::format_error(const Error& err) {
	std::string build_err;

	// Build main message
	switch (err.severity()) {
		case WARNING:
			build_err = "\033[1;33mwarning: \033[22;33m" + err.message();
			break;
		case ERROR:
			build_err = "\033[1;31merror: \033[22;31m" + err.message();
			break;
		case FATAL:
			build_err = "\033[1;31merror: " + err.message();
			break;
		case BUG:
			build_err = "\033[1;31mcompiler error: " + err.message();
			break;
		default:
			build_err = err.message();
			break;
	}
	build_err += "\033[0m\n";

	// Build sub-messages
	for (const auto& sub : err.children()) {
		switch (sub.type) {
			case SPAN:
				{
					if (!err.span().has_value())
						continue;

					build_err += "\033[4;36m>> ";

					// Add <file>:<line>:<column> to error message
					if (!err.span()->tu->filepath().empty())
						build_err += err.span()->tu->filepath() + ":";
					build_err += std::to_string(err.span()->lo.line) + ":" + std::to_string(err.span()->lo.col);

					build_err += "\033[0m\n";
					break;
				}

			case HIGHLIGHT:
				{
					if (!err.span().has_value())
						continue;

					auto line = err.span()->tu->get_line(err.span()->lo.line, false);

					int line_prefix = 0;
					int tabbed_len = 0;
					{
						// Remove whitespace from front and back
						while (line[line_prefix] == ' ' || line[line_prefix] == '\t')
							line_prefix++;

						int line_postfix = line.length();
						while (line[line_postfix] == ' ' || line[line_postfix] == '\t')
							line_postfix--;

						line = line.substr(line_prefix, line_prefix - line_postfix);

						// Replace '\t' with four spaces
						// Makes debugging message lengths consistent 
						size_t pos = 0;
						while ((pos = line.find("\t")) != line.npos) {
							line.replace(pos, 1, "    ");
							if (err.span()->lo.col <= pos && pos <= err.span()->hi.col)
								tabbed_len += 4;
							pos += 4;
						}
					}

					// TODO:  Multi line spans still look quite stupid.
					auto linenum_str = std::to_string(err.span()->lo.line);
					auto linenum_ws = std::string(linenum_str.length(), ' ');
					build_err += linenum_ws + " |\n";
					build_err += linenum_str + " | " + line + "\n";
					build_err += linenum_ws + " | ";

					// The current span's start pos in the error message
					size_t index = build_err.length() + err.span()->lo.col - 1 - line_prefix;
					// The final length of the error message
					size_t new_len = err.span()->lo.line == err.span()->hi.line ?
						index + (err.span()->hi.bit - err.span()->lo.bit) + tabbed_len :	// TRUE
						index + line.length();												// FALSE
					
					build_err.resize(new_len, ' ');

					// Add arrows under the bad code
					for (; index <= new_len; index++)
						build_err[index] = '^';

					build_err += "\033[0m\n";
					break;
				}

			case HELP:
				build_err += "help: " + sub.msg;
				build_err += "\033[0m\n";
				break;

			case NOTE:
				build_err += "note: " + sub.msg;
				build_err += "\033[0m\n";
				break;

			default:
				Error err(BUG, "failed to create error message; invalid error type", 999);
				err.emit();
		}
	}
	return build_err;
}