#include "error.hpp"
#include "driver/session.hpp"
#include "source/translation_unit.hpp"

void Error::emit() const {
	Session::handler.emit(*this);
}