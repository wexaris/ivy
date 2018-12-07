#pragma once
#include <unordered_set>
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

	/* A hash of all of the error strings that will be emitted.
	 * Used to avoid emitting the same error multiple times. */
	std::unordered_set<std::string> msg_hashes;

public:
	HandlerFlags flags;

	ErrorHandler(const HandlerFlags& flags = HandlerFlags())
		: msg_hashes(), flags(flags)
	{}

	/* Create a new basic error. */
	static inline Error new_error(ErrSeverity sev, const std::string& msg) {
		return Error(sev, msg);
		// TODO:  Hash and save error message
	}

	/* Emit the given error. */
	inline void emit(const Error& err) const {
		Emitter::emit(err);
	}

	/* True is there have been any errors. */
	inline bool has_errors() const			{ return err_count > 0; }
};