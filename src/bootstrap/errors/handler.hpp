#pragma once
#include "emitter.hpp"
#include <unordered_set>
#include <cstring>
#include <string>
#include <list>

struct HandlerFlags {
	bool trace;
	bool no_warnings;
	bool warnings_as_err;

	HandlerFlags() = default;
};

/* Takes care of making and emitting errors.
 * Binds together the 'Error' and 'Emitter' systems. */
class ErrorHandler {

private:
	Emitter& emitter;

	std::string trace_indent;
	static constexpr char INDENT_LEVEL[] = "  ";

	/* All of the errors that are still yet to be emitted. */
	std::list<Error> delayed_errors;

public:
	HandlerFlags flags;

	explicit ErrorHandler(Emitter& emitter, const HandlerFlags& flags = HandlerFlags())
		: emitter(emitter), flags(flags)
	{}

	/* Create a new basic error. */
	inline Error new_error(Severity sev, const std::string& msg, int code) {
		return Error(sev, msg, code);
	}
	/* Create a new spanned error. */
	inline Error new_error(Severity sev, const std::string& msg, const Span& sp, int code) {
		return Error(sev, msg, sp.into_wide(), code);
	}

	/* Emit a trace message.
	 * Will not write messages unless tracing is enabled.
	 * Every new trace will be indented further. */
	inline void trace(std::string msg) {
		if (flags.trace) {
			trace_indent += INDENT_LEVEL;
			printf("%s%s\n", trace_indent.c_str(), msg.c_str());
		}
	}

	/* Removes the last level of indentation from trace messages. 
	 * Should be called at the end of a trace level. */
	inline void end_trace() {
		if (flags.trace)
			trace_indent.resize(trace_indent.size() - strlen(INDENT_LEVEL));
	}

	/* Emit the given error. 
	 * Might not be emitted if '-nowarn' was set. */
	void emit(const Error& err) const;

	/* Emit all of the backed up errors if there are any. */
	void emit_delayed() const;

	/* Recounts the delayed errors.
	 * Removes errors that have been cancelled.
	 * Returns the new error count. */
	size_t recount_errors();

	/* Returns the last error that was pushed back. */
	inline Error& last() { return delayed_errors.back(); }

	/* True is there have been any errors.
	 * Does not recount the delayed errors. */
	inline bool has_errors() const	{ return delayed_errors.size() > 0; }

	Error* make_warning(const std::string& msg, int code = 0);
	Error* make_warning_spanned(const std::string& msg, const Span& sp, int code = 0);
	Error* make_warning_higligted(const std::string& msg, const Span& sp, int code = 0);
	Error* make_error(const std::string& msg, int code = 0);
	Error* make_error_spanned(const std::string& msg, const Span& sp, int code = 0);
	Error* make_error_higligted(const std::string& msg, const Span& sp, int code = 0);
	Error make_fatal(const std::string& msg, int code = 0);
	Error make_fatal_spanned(const std::string& msg, const Span& sp, int code = 0);
	Error make_fatal_higligted(const std::string& msg, const Span& sp, int code = 0);
	Error make_bug(const std::string& msg);

	void emit_fatal(const std::string& msg, int code = 0);
	void emit_fatal_spanned(const std::string& msg, const Span& sp, int code = 0);
	void emit_fatal_higligted(const std::string& msg, const Span& sp, int code = 0);
};