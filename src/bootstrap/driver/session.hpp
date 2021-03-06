#pragma once
#include "errors/handler.hpp"

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

	SysConfig(OS os, Arch arch, SysType isize, SysType usize, SysType fsize) 
		: os(os), arch(arch),
		isize(isize),
		usize(usize),
		fsize(fsize)
	{}
};

/* The current compile session.
 * Stores the system's specs and objects that are needed in many
 * different parts of the compiler. */
class Session {
	
	/* Internal system configuration info */
	static SysConfig sysconf;

	/* The current working direcotry. */
	static std::string cwd;
	
public:
	static ErrorHandler handler;
	static Emitter emitter;
	
	/* Returns a reference to the Session's SysConfig. */
	static inline const SysConfig& get_sysconf() { return sysconf; }
	/* Override current system configuration. */
	static inline void set_sysconf(const SysConfig& cfg) { sysconf = cfg; }

	/* Returns the current working directory path. */
	static inline std::string get_cwd() { return cwd; }

};