#pragma once
#include "util/span.hpp"
#include <optional>
#include <vector>
#include <string>

enum SubErrorType {
	SPAN,
	HIGHLIGHT,
	HELP,
	NOTE,
};

struct SubError {
	SubErrorType type;
	std::string msg;
};

/*  */
enum ErrSeverity {
	WARNING,
	ERROR,
	FATAL,
};

class Error {
	
private:
	ErrSeverity sev;
	std::string msg;
	int id;
	Span sp = Span(TranslationUnit(""), 0, 0, 0, 0, 0, 0);

	std::vector<SubError> sub_err;

	inline void sub(SubErrorType ty, std::string msg = "") {
		sub_err.push_back(SubError { ty, std::move(msg) });
	}

public:
	Error(ErrSeverity lvl, std::string msg, int code = 0)
		: sev(lvl), msg(std::move(msg)), id(code)
	{}

	std::string format() const {
		std::string build_err;

		// Build main message
		switch (sev) {
			case WARNING:
				build_err = "warning: ";
				break;
			case ERROR:
				build_err = "error: ";
				break;
			case FATAL:
				build_err = "error: ";
				break;
			default:
				break;
		}
		build_err += msg + "\n";

		// Build sub-messages
		for (const auto& err : sub_err) {
			switch (err.type) {
				case SPAN:
					build_err += "--> ";
					if (!sp.tu->filepath().empty())
						build_err += sp.tu->filepath() + ":";
					build_err += std::to_string(sp.lo.line) + ":" + std::to_string(sp.lo.col) + "\n";
					break;

				case HIGHLIGHT:
					{
						auto line = sp.tu->this_source_line(sp.lo.bit);
						build_err += "  |\n";
						build_err += "  | " + line + "\n";
						build_err += "  | ";
						for (size_t i = 1; i <= line.length(); i++) {
							if (i >= static_cast<size_t>(sp.lo.col) && i < static_cast<size_t>(sp.hi.col))
								build_err += "^";
							else
								build_err += " ";
						}
						build_err += "\n";
						break;
					}

				case HELP:
					build_err += "help: " + err.msg + "\n";
					break;

				case NOTE:
					build_err += "note: " + err.msg + "\n";
					break;

				default:
					break;
			}
		}

		return build_err;
	}

	inline Error& add_span(const Span& span) {
		sub(SPAN);
		this->sp = span;
		return *this;
	}

	inline Error& add_highlight() {
		sub(HIGHLIGHT);
		return *this;
	}

	inline Error& add_help(const std::string& msg) {
		sub(HELP, msg);
		return *this;
	}

	inline Error& add_note(const std::string& msg) {
		sub(NOTE, msg);
		return *this;
	}

	inline ErrSeverity severity() const			{ return sev; }
	inline const std::string& message() const	{ return msg; }
	inline int code() const						{ return id; }
	inline const Span& span() const				{ return sp; }
};