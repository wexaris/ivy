#include "error.hpp"
#include "emitter.hpp"
#include "source/translation_unit.hpp"

void Error::emit() const {
	Emitter::emit(*this);
}