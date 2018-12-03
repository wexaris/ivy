#pragma once
#include <string>
#include "error.hpp"

/* A small wrapper around formatting and printing error strings. */
class Emitter {

public:
	/* Emit the given 'Error'.
	 * The error's 'format()' is called to get the final string that will be printed. */
	static void emit(const Error& err) {
		printf(err.format().c_str());
		
		if (err.is_fatal())
			throw std::exception();
	}
};