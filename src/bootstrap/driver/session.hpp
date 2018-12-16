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

	SysConfig();

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
	static SysConfig cfg;
	
public:
	static ErrorHandler handler;
	static Emitter emitter;
	
	/* Set the system configuration. */
	static inline void set_system_cfg(const SysConfig& conf) {
		cfg = conf;
	}

	/* Returns a reference to the Session's SysConfig. */
	static inline const SysConfig& conf() { return cfg; }
};