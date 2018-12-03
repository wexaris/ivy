#include "error.hpp"
#include "emitter.hpp"

std::string Error::format() const {
	std::string build_err;

	// Build main message
	switch (sev) {
		case WARNING:
			build_err = "\033[1;33mwarning: \033[22;33m" + msg;
			break;
		case ERROR:
			build_err = "\033[1;31merror: \033[22;31m" + msg;
			break;
		case FATAL:
			build_err = "\033[1;31merror: " + msg;
			break;
		case BUG:
			build_err = "\033[1;31mcompiler error: " + msg;
			break;
		default:
			build_err = msg;
			break;
	}
	build_err += "\033[0m\n";

	// Build sub-messages
	for (const auto& err : sub_err) {
		switch (err.type) {
			case SPAN:
				build_err += "\033[4;36m>> ";

				// Add <file>:<line>:<column> to error message
				if (!sp.tu->filepath().empty())
					build_err += sp.tu->filepath() + ":";
				build_err += std::to_string(sp.lo.line) + ":" + std::to_string(sp.lo.col);

				build_err += "\033[0m\n";
				break;

			case HIGHLIGHT:
				{
					auto pos = sp.tu->pos_from_index(sp.lo.bit);

					build_err += "  |\n";
					build_err += "  | " + sp.tu->get_line(pos.line) + "\n";
					build_err += "  | ";

					// The current span start pos in the error message
					size_t index = build_err.length() + sp.lo.col;
					// The final length of the error message
					size_t new_len = index + sp.hi.col - sp.lo.col;
					build_err.resize(new_len - 1, ' ');
					
					// Add arrows under the bad code
					for (; index < new_len; index++)
						build_err[index-1] = '^';

					build_err += "\033[0m\n";
					break;
				}

			case HELP:
				build_err += "help: " + err.msg;
				build_err += "\033[0m\n";
				break;

			case NOTE:
				build_err += "note: " + err.msg;
				build_err += "\033[0m\n";
				break;

			default:
				break;
		}
	}
	return build_err;
}

void Error::emit() const {
	Emitter::emit(*this);
}