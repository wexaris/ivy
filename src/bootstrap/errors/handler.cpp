#include "handler.hpp"

size_t ErrorHandler::emit_delayed() const {
	size_t failures = 0;
	// Emit all of the delayed errors
	// Count how many of them fail the compilation session
	for (const auto& err : delayed_errors) {
		if (!err.is_canceled()) {
			try {
				emit(err);
			} catch (const std::exception& e) {
				failures++;
			}
		}
	}
	return failures;
}

size_t ErrorHandler::recount_errors() {
	if (!has_errors())
		return 0;

	std::list<Error> checked_erros;
	for (auto& err : delayed_errors) {
		if (!err.is_canceled())
			checked_erros.push_back(std::move(err));
	}
	delayed_errors = checked_erros;
	return delayed_errors.size();
}

Error* ErrorHandler::error_spanned(const std::string& msg, const Span& sp, int code) {
	delayed_errors.push_back(new_error(ERROR, msg, sp, code));
	return &*(--delayed_errors.end());
}
Error* ErrorHandler::error_higligted(const std::string& msg, const Span& sp, int code) {
	delayed_errors.push_back(new_error(ERROR, msg, sp, code).add_highlight());
	return &*(--delayed_errors.end());
}
Error* ErrorHandler::fatal_spanned(const std::string& msg, const Span& sp, int code) {
	delayed_errors.push_back(new_error(FATAL, msg, sp, code));
	return &*(--delayed_errors.end());
}
Error* ErrorHandler::fatal_higligted(const std::string& msg, const Span& sp, int code) {
	delayed_errors.push_back(new_error(FATAL, msg, sp, code).add_highlight());
	return &*(--delayed_errors.end());
}