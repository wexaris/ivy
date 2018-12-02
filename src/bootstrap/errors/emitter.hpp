#pragma once
#include <string>
#include "error.hpp"

class Emitter {

public:
	static void emit(const Error& err) {
		printf(err.format().c_str());
	}
};