#pragma once
#include <string>
#include "error.hpp"
#include "exceptions.hpp"

/* A small wrapper around formatting and printing error strings. */
class Emitter {

public:
	/* Emit the given 'Error'.
	 * The error's 'format()' is called to get the final string that will be printed.
	 * Some 'CompilerException' might be thrown if the error is of type 'ERROR', 'FATAL' or 'BUG'. */
	static void emit(const Error& err) {
		// Cenceled errors aren't emitted
		if (err.is_canceled())
			return;

		// Format and print error
		printf("%s\n", format_error(err).c_str());

		// Throw exceptions for different error types
		if (err.is_error()) throw ErrorException();
		else if (err.is_fatal()) throw FatalException();
		else if (err.is_bug()) throw InternalException();
	}

	/* Returns a fully formatted error message.
	 * Compiles the sub-messages and adds coloring. */
	static std::string format_error(const Error&);
};