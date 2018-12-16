#pragma once
#include "error.hpp"
#include "exceptions.hpp"
#include <string>

/* A small wrapper around formatting and printing error strings. */
class Emitter {

private:
	/* Keep track of how many errors have been emitted. */
	size_t emitted_err_count;

public:
	Emitter() = default;
	virtual ~Emitter() {}

	/* Emit the given 'Error'.
	 * The error's 'format()' is called to get the final string that will be printed.
	 * Some 'CompilerException' might be thrown if the error is of type 'ERROR', 'FATAL' or 'BUG'. */
	virtual void emit(const Error& err) {
		// Cenceled errors aren't emitted
		if (err.is_canceled())
			return;

		// Format and print error
		printf("%s\n", format_error(err).c_str());

		// Throw exceptions for different error types
		if (err.is_error()) { emitted_err_count++; throw ErrorException(); }
		else if (err.is_fatal()) { emitted_err_count++; throw FatalException(); }
		else if (err.is_bug()) { emitted_err_count++; throw InternalException(); }
	}

	inline size_t num_err_emitted() const { return emitted_err_count; }

	/* Returns a fully formatted error message.
	 * Compiles the sub-messages and adds coloring. */
	std::string format_error(const Error&);
};