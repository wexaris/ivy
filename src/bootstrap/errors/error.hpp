#pragma once
#include "util/span.hpp"
#include <optional>
#include <vector>
#include <string>

/* The type of sub-error message. */
enum SubErrorType {
	SPAN,
	HIGHLIGHT,
	HELP,
	NOTE,
};

/* A miniature message added to errors.
 * Acts basically like a flag for what extra info we want added.
 * e.g. a preview of the bad code, a note or help message.*/
struct SubError {
	SubErrorType type;
	std::string msg;
};

/* The severity of the error message.
 * 'FATAL' and 'BUG' automatically stop the compilation process. */
enum ErrSeverity {
	CANCELED,

	WARNING,
	ERROR,
	FATAL,
	BUG,
};

/* An 'Error' could be any structured error message.
 * An error has a main error message and optional notes, help messages, etc.
 * An error code should be added where possible. */
class Error {
	
private:
	ErrSeverity sev;
	std::string msg;
	int id;
	Span sp = Span(TranslationUnit(""), 0, 0, 0, 0, 0, 0);

	std::vector<SubError> sub_err;

	/* Add a sub-error to the error.
	 * The sub-error type specific wrapper functions should be used instead. */
	inline void sub(SubErrorType ty, const std::string& msg = "") {
		sub_err.push_back(SubError { ty, std::move(msg) });
	}

public:
	Error(ErrSeverity lvl, std::string msg, int code = 0)
		: sev(lvl), msg(std::move(msg)), id(code)
	{}

	/* Disables the error.
	 * Will not be emitted. */
	inline void cancel()				{ sev = CANCELED; }
	inline bool is_canceled() const		{ return sev == CANCELED; }

	/* Returns a full formatted error message.
	 * Compiles the sub-messages and adds coloring. */
	std::string format() const;

	/* Add a span to the error. */
	inline Error& add_span(const Span& span) {
		this->sp = span;
		sub(SPAN);
		return *this;
	}

	/* Add a preview of the bad line of code.
	 * Expects the span to also be set. */
	inline Error& add_highlight() {
		sub(HIGHLIGHT);
		return *this;
	}

	/* Add a 'help:' message to the error. */
	inline Error& add_help(const std::string& msg) {
		sub(HELP, msg);
		return *this;
	}

	/* Add a 'note:' message to the error. */
	inline Error& add_note(const std::string& msg) {
		sub(NOTE, msg);
		return *this;
	}

	/* True if the error will stop the compilation session. */
	inline bool is_fatal() const {
		return sev == FATAL || sev == BUG;
	}

	/* Get the main error message. */
	inline const std::string& message() const	{ return msg; }
	/* Get the error code of the error. */
	inline int code() const						{ return id; }
	inline ErrSeverity severity() const			{ return sev; }
	inline const Span& span() const				{ return sp; }

	/* Emits the error via the 'Emitter'. */
	void emit() const;
};