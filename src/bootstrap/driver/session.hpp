#pragma once
#include "parser/invalid_token_except.hpp"
#include "source/source_file.hpp"
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

namespace env {

#ifdef _WIN64				// Check for Windows and it's architecture
	constexpr const OS _OS = OS::Windows;
	constexpr const Arch _ARCH = Arch::x64;
#elif _WIN32
	constexpr const OS _OS = OS::Windows;
	constexpr const Arch _ARCH = Arch::x86;
#elif __APPLE__				// Check for Apple MacOS
	#if TARGET_OS_MAC
		constexpr const OS _OS = OS::MacOS;
	#else
    	#error "Unrecognised OS"
	#endif
#elif __linux__				// Check for Linux
	constexpr const OS _OS = OS::Linux;
#else
	#error "Unrecognised OS"
#endif

// Check GNU architecture
#if __GNUC__
	#if __x86_64__ || __ppc64__
		constexpr const Arch _ARCH = Arch::x64;
	#else 
		constexpr const Arch _ARCH = Arch::x86;
	#endif
#else
	#ifndef _WIN32
		#error "Unrecognized architecture"
	#endif
#endif

}

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

struct System {
	static constexpr inline OS get_os()				{ return env::_OS; }
	static constexpr inline Arch get_arch() 		{ return env::_ARCH; }
	static constexpr inline SysType get_isize() 	{ return env::_ARCH == Arch::x64 ? SysType::i64 : SysType::i32; }
	static constexpr inline SysType get_usize() 	{ return env::_ARCH == Arch::x64 ? SysType::u64 : SysType::u32; }
	static constexpr inline SysType get_fsize() 	{ return env::_ARCH == Arch::x64 ? SysType::f64 : SysType::f32; }
};

/* The full system specification.
 * Contains OS, architecture and default variable sizes.
 */
struct SysConfig {
	OS os;
	Arch arch;
	SysType isize;
	SysType usize;
	SysType fsize;

	SysConfig()
		: os(System::get_os()),
		arch(System::get_arch()),
		isize(System::get_isize()),
		usize(System::get_usize()),
		fsize(System::get_fsize())
	{}

	SysConfig(OS os, Arch arch, SysType isize, SysType usize, SysType fsize) 
		: os(os),
		arch(arch),
		isize(isize),
		usize(usize),
		fsize(fsize)
	{}
};

/* The levels of severity that a logged message might have.
 * The CRITICAL level is the only one that will throw an exception.
 */
enum Severity {
	TRACE = 0,
	MESSAGE,
	WARNING,
	ERROR,
	CRITICAL	// fails
};

/* The current compile session.
 * Stores the system's specification.
 * Provides output for compiler messages.
 * */
class Session {
	
	/* Internal system configuration info */
	static SysConfig cfg;

	/* The required severity level to print messages.
	 * The requred level is MESSAGE by default. */
	static Severity _req_severity;
	/* Should trace messages be formatted.
	 * True by default.  */
	static bool _format;

	/* Indentation amount on scoping */
	static constexpr char INDENT[] = "  ";
	/* Current indentation level */
	static std::string indent_prefix;

	/* Checks is the provided severity level is over the */
	static inline bool should_write(const Severity& sev) {
		return sev >= _req_severity;
	}

	/* The main message logger. */
	static inline void log(const Severity& sev, const std::string& msg) {
		switch (sev) {
			case Severity::MESSAGE:
				printf("----%s\n", msg.c_str());
				break;
			case Severity::WARNING:
				printf("\033[33m----%s\n\033[0m", msg.c_str());
				break;
			case Severity::ERROR:
				printf("\033[1,31m----%s\n\033[0m", msg.c_str());
				break;
			case Severity::CRITICAL:
				throw InvalidToken(msg);

			default:
				printf("    %s%s\n", INDENT, msg.c_str());
		}
	}

	Session() {}

public:
	static void setup(const SysConfig& conf) {
		cfg = conf;
	}

	/* Emit a trace message.
	 * Will not write messages unless tracing is enabled.
	 * Will indent messages unless formatting is disabled.
	 */
	static inline void trace(const std::string& msg) {
		if (should_write(TRACE)) {
			if (_format)
				indent_prefix += INDENT;
			printf("%s%s\n", indent_prefix.c_str(), msg.c_str());
		}
	};

	/* Removes the last level of indentation from trace messages. 
	 * Should be called at the end of a trace level.
	 */
	static inline void end_trace() {
		if (should_write(TRACE) && _format)
			indent_prefix.resize(indent_prefix.length() - strlen(INDENT));
	};

	/* Emit a compile time message. */
	static inline void msg(const std::string& msg) {
		if (should_write(MESSAGE))
			log(MESSAGE, msg);
	};

	/* Emit an error with the provided message. */
	static inline void err(const std::string& msg) {
		if (should_write(ERROR))
			log(ERROR, std::string("error: ") + msg);
	};

	/* Emit an error with the provided message. */
	[[noreturn]] static inline void failure(const std::string& msg) {
		if (should_write(ERROR)) {
			trace("FAILED");
			end_trace();

			log(ERROR, std::string("failure: ") + msg);
			
		}
	};

	/* Emit an error with the provided position and message.
	 * Stops the compilation process.
	 */
	static inline void span_err(const std::string& msg, const Span& sp) {
		if (should_write(ERROR)) {
			trace("FAILED");
			end_trace();
		
			std::string err = sp.sf.filepath() + ":" + std::to_string(sp.lo.line) + ":" + std::to_string(sp.lo.col)
				 + " error: " + msg;
			log(ERROR, err);
		}
	};

	/* Emit an error about an internal compiler malfunction.
	 * Stops the compilation process.
	 */
	[[noreturn]] static inline void bug(const std::string& msg) {
		throw std::runtime_error("internal compiler error: " + msg);
	};

	/* Emit an error about an unimplemented state.
	 * Stops the compilation process.
	 */
	[[noreturn]] static inline void unimpl(const std::string& msg) {
		throw std::runtime_error(msg + " not implemented yet");
	};

	/* Limit the required severity for message writing. */
	static inline void require_severity(const Severity& sev)	{ _req_severity = sev; };
	static inline void enable_trace()							{ _req_severity = TRACE; };
	static inline void enable_fmt(bool i)						{ _format = i; };

	/* Returns a static reference to the Session's SysConfig. */
	static inline const SysConfig& conf() { return cfg; }
};