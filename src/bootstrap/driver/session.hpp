#pragma once
#include "source/translation_unit.hpp"
#include "util/span.hpp"
#include <cstring>
#include <string>

/* Possible operating systems. */
enum class OS {
	Windows,
	MacOS,
	Linux,
};
/* Possible system architectures. */
enum class Arch {
	x86,
	x64,
};

/* Possible default variable sizes. */
enum class SysType {
	i8,
	i16,
	i32,
	i64,

	u8,
	u16,
	u32,
	u64,

	f32,
	f64
};

/* The full system specification.
 * Contains OS, architecture and default variable sizes. */
struct SysConfig {
	OS os;
	Arch arch;
	SysType isize;
	SysType usize;
	SysType fsize;

	SysConfig();

	SysConfig(OS os, Arch arch, SysType isize, SysType usize, SysType fsize) 
		: os(os), arch(arch),
		isize(isize),
		usize(usize),
		fsize(fsize)
	{}
};

/* The levels of severity that a logged message might have.
 * The CRITICAL level is the only one that will throw an exception. */
enum class Severity {
	TRACE = 0,
	MESSAGE,
	WARNING,
	ERROR,
	CRITICAL	// fails
};

/* The current compile session.
 * Stores the system's specification.
 * Provides output for compiler messages. */
class Session {
	
	/* Internal system configuration info */
	static SysConfig cfg;

	/* The required severity level to print messages.
	 * The requred level is MESSAGE by default. */
	static Severity _req_severity;
	/* Should trace messages be formatted.
	 * True by default. */
	static bool _format;

	/* CUrrent indentation amount for trace scoping */
	static constexpr char INDENT[] = "  ";
	/* Current indentation level */
	static std::string indent_prefix;

	/* Checks is the provided severity level is over the */
	static inline bool should_write(const Severity& sev) {
		return sev >= _req_severity;
	}

	/* The main message logger. */
	static void log(const Severity& sev, const std::string& msg) {
		switch (sev) {
			case Severity::MESSAGE:
				printf("---- %s\n", msg.c_str());
				break;
			case Severity::WARNING:
				printf("\033[33m---- %s\n\033[0m", msg.c_str());
				break;
			case Severity::ERROR:
				printf("\033[1;31m---- %s\n\033[0m", msg.c_str());
				break;
			case Severity::CRITICAL:
				printf("\033[1;35m---- %s\n\033[0m", msg.c_str());
				std::exit(1);
				break;

			default:
				printf("     %s\n", msg.c_str());
		}
	}

	Session() {}

public:
	static inline void set_sysconfig(const SysConfig& conf) {
		cfg = conf;
	}

	/* Emit a trace message.
	 * Will not write messages unless tracing is enabled.
	 * Will indent messages unless formatting is disabled. */
	static inline void trace(const std::string& msg) {
		if (should_write(Severity::TRACE)) {
			if (_format)
				indent_prefix += INDENT;
			printf("%s%s\n", indent_prefix.c_str(), msg.c_str());
		}
	};

	/* Removes the last level of indentation from trace messages. 
	 * Should be called at the end of a trace level. */
	static inline void end_trace() {
		if (should_write(Severity::TRACE) && _format)
			indent_prefix.resize(indent_prefix.length() - strlen(INDENT));
	};

	/* Emit a compile time message. */
	static inline void msg(const std::string& msg) {
		if (should_write(Severity::MESSAGE))
			log(Severity::MESSAGE, msg);
	};

	/* Emit a compile time message. */
	static inline void warn(const std::string& msg) {
		if (should_write(Severity::WARNING))
			log(Severity::WARNING, msg);
	};

	/* Emit an error with the provided message. */
	static inline void err(const std::string& msg) {
		if (should_write(Severity::ERROR))
			log(Severity::ERROR, std::string("error: ") + msg);
	};

	/* Emit an error with the provided position and message.
	 * Stops the compilation process. */
	static inline void span_err(const std::string& msg, const Span& sp) {
		if (should_write(Severity::ERROR)) {
			trace("FAILED");
			end_trace();
		
			std::string err = sp.tu->filepath() + ":" + std::to_string(sp.lo.line) + ":" + std::to_string(sp.lo.col)
				 + " error: " + msg;
			log(Severity::ERROR, err);
		}
	};

	/* Emit a critical error message and stop the compiler. */
	[[noreturn]] static inline void failure(const std::string& msg) {
		if (should_write(Severity::CRITICAL)) {
			trace("FAILED");
			end_trace();

			log(Severity::CRITICAL, std::string("failure: ") + msg);
		}
	};

	/* Emit an error about an internal compiler malfunction.
	 * Stops the compilation process. */
	[[noreturn]] static inline void bug(const std::string& msg) {
		log(Severity::CRITICAL, "internal compiler error: " + msg);
		// Critical log should already exit, but we need to exit from here
		// to make the [[noreturn]] work without errors
		std::exit(1);
	};

	/* Emit an error about an unimplemented state.
	 * Stops the compilation process. */
	[[noreturn]] static inline void unimpl(const std::string& msg) {
		log(Severity::CRITICAL, msg + " not implemented yet");
		// Critical log should already exit, but we need to exit from here
		// to make the [[noreturn]] work without errors
		std::exit(1);
	};

	/* Limit the required severity for message writing. */
	static inline void require_severity(const Severity& sev)	{ _req_severity = sev; };
	static inline void enable_trace()							{ _req_severity = Severity::TRACE; };
	static inline void enable_fmt(bool i)						{ _format = i; };

	/* Returns a reference to the Session's SysConfig. */
	static inline const SysConfig& conf() { return cfg; }
};