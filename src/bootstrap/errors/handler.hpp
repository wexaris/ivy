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
	inline Error new_error(ErrSeverity sev, const std::string& msg) {
		return Error(sev, msg);
		err_count++;
		// TODO:  Hash and save error message
	}

	/* Emit the given error. */
	inline void emit(const Error& err) const {
		Emitter::emit(err);
	}

	/* Emit all of the backed up errors if there are any. */
	inline void emit_delayed() const {
		for (const auto& err : delayed_errors)
			Emitter::emit(err);
	}

	/* True is there have been any errors. */
	inline bool has_errors() const					{ return err_count > 0; }

	Error error_spanned(const std::string& msg, const Span& sp, int code = 0);
	Error error_higligted(const std::string& msg, const Span& sp, int code = 0);
	Error fatal_spanned(const std::string& msg, const Span& sp, int code = 0);
	Error fatal_higligted(const std::string& msg, const Span& sp, int code = 0);

	/* Creates and emits an internal compiler failure error message.
	 * Will be fatal. */
	void emit_fatal_bug(const std::string& msg);
	void emit_fatal_bug(const std::string& msg, int code);

	/* Creates and emits an internal compiler failure error message about
	 * a missing feature.
	 * Will be fatal.
	 * A wrapper around 'fatal_bug()'. */
	void emit_fatal_unimpl(const std::string& msg);
	void emit_fatal_unimpl(const std::string& msg, int code);
};