#include "handler.hpp"

Error ErrorHandler::error_spanned(const std::string& msg, const Span& sp, int code) {
	auto err = new_error(ERROR, msg);
	err.add_span(sp);
	err.set_code(code);
	return err;
}
Error ErrorHandler::error_higligted(const std::string& msg, const Span& sp, int code) {
	auto err = new_error(ERROR, msg);
	err.add_span(sp);
	err.add_highlight();
	err.set_code(code);
	return err;
}
Error ErrorHandler::fatal_spanned(const std::string& msg, const Span& sp, int code) {
	auto err = new_error(FATAL, msg);
	err.add_span(sp);
	err.set_code(code);
	return err;
}
Error ErrorHandler::fatal_higligted(const std::string& msg, const Span& sp, int code) {
	auto err = new_error(FATAL, msg);
	err.add_span(sp);
	err.add_highlight();
	err.set_code(code);
	return err;
}

void ErrorHandler::emit_fatal_bug(const std::string& msg) {
	auto err = new_error(BUG, msg);
	err.emit();
}
void ErrorHandler::emit_fatal_unimpl(const std::string& msg) {
	emit_fatal_bug(msg + " not implemented yet");
}