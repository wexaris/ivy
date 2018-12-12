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
enum Severity {
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
	friend class ErrorHandler;
	
private:
	Severity sev;
	std::string msg;
	std::optional<Span> sp;
	int id;

	std::vector<SubError> sub_err;

	/* Add a sub-error to the error.
	 * The sub-error type specific wrapper functions should be used instead. */
	inline void sub(SubErrorType ty, const std::string& msg = "") {
		sub_err.push_back(SubError { ty, std::move(msg) });
	}

public:
	Error(Severity lvl, std::string msg, int code = 0)
		: sev(lvl), msg(std::move(msg)), id(code)
	{}
	Error(Severity lvl, std::string msg, const Span& sp, int code = 0)
		: sev(lvl), msg(std::move(msg)), sp(sp), id(code)
	{}

	/* Disables the error.
	 * Will not be emitted. */
	inline void cancel()				{ sev = CANCELED; }
	inline bool is_canceled() const		{ return sev == CANCELED; }

	/* Get the error code of the error. */
	inline int code() const				{ return id; }
	/* Give the error an error code. */
	inline void set_code(int code)		{ id = code; }

	/* Add info about the span of the error.
	* Expects the span to also be set. */
	inline Error& add_span() {
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

	/* True if this is a warning. */
	inline bool is_warning() const	{ return sev == WARNING; }
	/* True if this is a standard error. */
	inline bool is_error() const	{ return sev == ERROR; }
	/* True if this is a fatal error. */
	inline bool is_fatal() const	{ return sev == FATAL; }
	/* True if this is an internal compiler error. */
	inline bool is_bug() const		{ return sev == BUG; }

	/* Get the main error message. */
	inline const std::string& message() const		{ return msg; }
	inline Severity severity() const				{ return sev; }
	inline const std::optional<Span>& span() const	{ return sp; }

	/* Get a vector of all of the sub-errors of this 'Error'. */
	inline const std::vector<SubError>& children() const	{ return sub_err; }

	/* Emits the error via the 'Emitter'. */
	void emit() const;
};