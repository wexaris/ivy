#pragma once
#include <unordered_set>
#include <string>
#include "emitter.hpp"

struct HandlerFlags {
	bool no_warnings;
	bool warnings_as_err;

	HandlerFlags() = default;
};

/* Takes care of making and emitting errors.
 * Binds together the 'Error' and 'Emitter' systems. */
class ErrorHandler {

private:
	size_t err_count = 0;

	/* A hash of all of the error strings that will be emitted.
	 * Used to avoid emitting the same error multiple times. */
	std::unordered_set<std::string> msg_hashes;

public:
	HandlerFlags flags;

	ErrorHandler(const HandlerFlags& flags = HandlerFlags())
		: msg_hashes(), flags(flags)
	{}

	/* Create a new basic error. */
	inline Error new_error(ErrSeverity sev, const std::string& msg) {
		return Error(sev, msg);
		err_count++;
		// TODO:  Hash and save error message
	}

	/* Emit the given error. */
	inline void emit(const Error& err) const {
		Emitter::emit(err);
	}

	/* Emit all of the backed up errors if there are any. */
	inline void emit_delayed() const {
		for (const auto& err : delayed_errors)
			Emitter::emit(err);
	}

	/* True is there have been any errors. */
	inline bool has_errors() const					{ return err_count > 0; }

	/* Creates and emits an internal compiler failure error message.
	 * Will be fatal. */
	inline void emit_fatal_bug(const std::string& msg) {
		auto err = new_error(BUG, msg);
		err.emit();
	}

	/* Creates and emits an internal compiler failure error message about
	 * a missing feature.
	 * Will be fatal.
	 * A wrapper around 'fatal_bug()'. */
	inline void emit_fatal_unimpl(const std::string& msg) {
		emit_fatal_bug(msg + " not implemented yet");
	}
};