#include "handler.hpp"

size_t ErrorHandler::emit_delayed() const {
	size_t failures = 0;
	// Emit all of the delayed errors
	// Count how many of them fail the compilation session
	for (const auto& err : delayed_errors) {
		if (!err.is_canceled()) {
			try {
				emit(err);
			} catch (const ErrorException& e) {
				failures++;
			} // Fatal exceptions should have been emitted during parsing
			catch (const FatalException& e) {
				emit(Error(BUG, "delayed fatal error; fatal errors should be emitted duiring the parse session", 0));
			} // Don't catch 'InternalExpection' as it's a sign of someting going wring in the compiler itself
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


///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Warnings //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

Error* ErrorHandler::make_warning(const std::string& msg, int code) {
	auto err = new_error(WARNING, msg, code);
	// Return pushed back error
	delayed_errors.push_back(std::move(err));
	return &*(--delayed_errors.end());
}
Error* ErrorHandler::make_warning_spanned(const std::string& msg, const Span& sp, int code) {
	auto err = new_error(WARNING, msg, sp, code);
	err.add_span();
	// Return pushed back error
	delayed_errors.push_back(std::move(err));
	return &*(--delayed_errors.end());
}
Error* ErrorHandler::make_warning_higligted(const std::string& msg, const Span& sp, int code) {
	auto err = new_error(WARNING, msg, sp, code);
	err.add_span();
	err.add_highlight();
	// Return pushed back error
	delayed_errors.push_back(std::move(err));
	return &*(--delayed_errors.end());
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// Errors //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

Error* ErrorHandler::make_error(const std::string& msg, int code) {
	auto err = new_error(ERROR, msg, code);
	// Return pushed back error
	delayed_errors.push_back(std::move(err));
	return &*(--delayed_errors.end());
}
Error* ErrorHandler::make_error_spanned(const std::string& msg, const Span& sp, int code) {
	auto err = new_error(ERROR, msg, sp, code);
	err.add_span();
	// Return pushed back error
	delayed_errors.push_back(std::move(err));
	return &*(--delayed_errors.end());
}
Error* ErrorHandler::make_error_higligted(const std::string& msg, const Span& sp, int code) {
	auto err = new_error(ERROR, msg, sp, code);
	err.add_span();
	err.add_highlight();
	// Return pushed back error
	delayed_errors.push_back(std::move(err));
	return &*(--delayed_errors.end());
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Fatal //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

Error ErrorHandler::make_fatal(const std::string& msg, int code) {
	auto err = new_error(FATAL, msg, code);
	return std::move(err);
}
Error ErrorHandler::make_fatal_spanned(const std::string& msg, const Span& sp, int code) {
	auto err = new_error(FATAL, msg, sp, code);
	err.add_span();
	return std::move(err);
}
Error ErrorHandler::make_fatal_higligted(const std::string& msg, const Span& sp, int code) {
	auto err = new_error(FATAL, msg, sp, -code);
	err.add_span();
	err.add_highlight();
	return std::move(err);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// Bug ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

Error ErrorHandler::make_bug(const std::string& msg) {
	auto err = new_error(BUG, msg, 0);
	return std::move(std::move(err));
}