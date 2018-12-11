#pragma once
#include <unordered_set>
#include <list>
#include <string>
#include "emitter.hpp"

struct HandlerFlags {
	bool no_warnings;
	bool warnings_as_err;

	HandlerFlags() = default;
};

/* Takes care of making and emitting errors.
 * Binds together the 'Error' and 'Emitter' systems. */
class ErrorHandler {

private:
	size_t err_count = 0;

	/* All of the errors that are still yet to be emitted. */
	std::list<Error> delayed_errors;

public:
	HandlerFlags flags;

	ErrorHandler(const HandlerFlags& flags = HandlerFlags())
		: flags(flags)
	{}

	/* Create a new basic error. */
	inline Error new_error(ErrSeverity sev, const std::string& msg, int code) {
		err_count++;
		return Error(sev, msg, code);
	}
	/* Create a new spanned error. */
	inline Error new_error(ErrSeverity sev, const std::string& msg, const Span& sp, int code) {
		err_count++;
		return Error(sev, msg, sp, code);
	}

	/* Emit the given error. */
	inline void emit(const Error& err) const {
		Emitter::emit(err);
	}

	/* Emit all of the backed up errors if there are any.
	 * Return the cumber of errors that would have failed the compilation. */
	size_t emit_delayed() const;

	/* Recounts the delayed errors.
	 * Removes errors that have been cancelled.
	 * Returns the new error count. */
	size_t recount_errors();

	/* True is there have been any errors.
	 * Does not recount the delayed errors. */
	inline bool has_errors() const			{ return err_count > 0; }

	Error* error_spanned(const std::string& msg, const Span& sp, int code = 0);
	Error* error_higligted(const std::string& msg, const Span& sp, int code = 0);
	Error* fatal_spanned(const std::string& msg, const Span& sp, int code = 0);
	Error* fatal_higligted(const std::string& msg, const Span& sp, int code = 0);
};