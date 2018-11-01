#pragma once
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

// Check for Windows and it's architecture
#ifdef _WIN64
	#define _OS OS::Windows
	#define _ARCH Arch::x64
#elif _WIN32
	#define _OS OS::Windows
	#define _ARCH Arch::x84
// Check for Apple MacOS
#elif __APPLE__
	#if TARGET_OS_MACpP
		#define _OS OS::MacOS
	#else
    	#error "Unrecognised OS"
	#endif
// Check for Linux
#elif __linux__
	#define _OS OS::Linux
#else
	#error "Unrecognised OS"
#endif

// Check GNU architecture
#if __GNUC__
	#if __x86_64__ || __ppc64__
		#define _ARCH Arch::x64
	#else 
		#define _ARCH Arch::x84
	#endif
#else
	#error "Unrecognised architecture"
#endif

/* Possible default varaible sizes. */
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
	static inline OS get_os() 			{ return _OS; }
	static inline Arch get_arch() 		{ return _ARCH; }
	static inline SysType get_isize() 	{ return _ARCH == Arch::x64 ? SysType::i64 : SysType::i32; }
	static inline SysType get_usize() 	{ return _ARCH == Arch::x64 ? SysType::u64 : SysType::u32; }
	static inline SysType get_fsize() 	{ return _ARCH == Arch::x64 ? SysType::f64 : SysType::f32; }
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

/* The current compile session.
 * Stores the system's specification.
 * Provides output for compiler messages.
 * */
class Session {
	
	/* Internal system configuration info */
	static SysConfig cfg;

	/* Message verbosity */
	static bool _verbose;
	/* Message formating */
	static bool _format;

	/* Indentation amount on scoping */
	static constexpr char INDENT[] = "  ";
	/* Current indentation level */
	static std::string indent_prefix;

	Session() {}

public:
	static void setup(SysConfig conf) {
		cfg = conf;
	}

	/* Emit a compile time message.
	 * Depends on the Session's verbosity.
	 */
	static inline void msg(const std::string& msg) {
		if (_verbose)
			printf("%s\n", msg.c_str());
	};

	/* Emit a compile time message.
	 * Depends on the Session's verbosity.
	 * Adds a tab character every time it's called for formatting.
	 */
	static inline void trace(const std::string& msg) {
		if (_verbose) {
			if (_format) indent_prefix += INDENT;
			printf("%s%s\n", indent_prefix.c_str(), msg.c_str());
		}
	};

	/* Formatting
	 * Depends on the Session's verbosity.
	 */
	static inline void end_trace() {
		if (_verbose && _format)
			indent_prefix.resize(indent_prefix.length() - strlen(INDENT));
	};

	/* Emit an error with the provided message.
	 * Stops the compilation process.
	 */
	static inline void err(const std::string& msg) {
		//printf("error: %s\n", msg.c_str());
		//std::exit(1);
		trace("FAILED");
		end_trace();

		std::string err = "error: " + msg;
		throw std::runtime_error(err.c_str());
	};

	/* Emit an error with the provided position and message.
	 * Stops the compilation process.
	 */
	[[noreturn]] static inline void span_err(const std::string& msg, const Span& sp) {
		//printf("%s:%u:%u-%u:%u error: %s\n", 
		//	sp.sf.filepath().c_str(),
		//	sp.lo.line, sp.lo.col,
		//	sp.hi.line, sp.hi.col,
		//	msg.c_str());
		//std::exit(1);
		trace("FAILED");
		end_trace();
		
		std::string err = sp.sf.filepath() + ":" + std::to_string(sp.lo.line) + ":" + std::to_string(sp.lo.col) +
			"-" + std::to_string(sp.hi.line) + ":" + std::to_string(sp.hi.col) + "error: " + msg;
		throw std::runtime_error(err.c_str());
	};

	/* Emit an error about an internal compiler malfunction.
	 * Stops the compilation process.
	 */
	[[noreturn]] static inline void bug(const std::string& msg) {
		printf("error: internal compiler error: %s\n", msg.c_str());
		std::exit(1);
	};

	/* Emit an error about an unimplemented state.
	 * Stops the compilation process.
	 */
	[[noreturn]] static inline void unimpl(const std::string& msg) {
		printf("error: %s not implemented yet\n", msg.c_str());
		std::exit(1);
	};

	static inline void set_verbose(bool i)	{ _verbose = i; };
	static inline bool is_verbose()			{ return _verbose; }

	static inline void set_fmt(bool i)		{ _format = i; };
	static inline bool is_fmt()				{ return _format; }

	/* Returns a static reference to the Session's SysConfig. */
	static inline const SysConfig& conf() { return cfg; }
};