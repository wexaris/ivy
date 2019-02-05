#include "session.hpp"
#include <stdio.h>  /* defines FILENAME_MAX */

// Check for Windows x64
#ifdef _WIN64
	#include <direct.h>
	#define uni_getcwd _getcwd
	constexpr const OS _OS = OS::Windows;
	constexpr const Arch _ARCH = Arch::x64;

// Check for Windows x86
#elif _WIN32
	#include <direct.h>
	#define uni_getcwd _getcwd
	constexpr const OS _OS = OS::Windows;
	constexpr const Arch _ARCH = Arch::x86;

// Check for Apple MacOS
#elif __APPLE__
	#if TARGET_OS_MAC
		constexpr const OS _OS = OS::MacOS;
	#else
    	#error "Unrecognised OS"
	#endif

// Check for Linux
#elif __linux__
	#include <unistd.h>
	#define uni_getcwd getcwd
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

/* Get the current working directory via 'uni_getcwd()'. */
std::string get_curr_working_dir() {
  char buffer[FILENAME_MAX];
  uni_getcwd(buffer, FILENAME_MAX);
  return std::string(buffer);
}

/* Get a complete SysConfig with the current system configuration. */
SysConfig get_curr_system_config() {
	SysType isize = _ARCH == Arch::x64 ? SysType::i64 : SysType::i32;
	SysType usize = _ARCH == Arch::x64 ? SysType::u64 : SysType::u32;
	SysType fsize = _ARCH == Arch::x64 ? SysType::f64 : SysType::f32;
	return SysConfig(_OS, _ARCH, isize, usize, fsize);
}

// Initalize the Session's varaibles with correct system info 
SysConfig Session::sysconf = get_curr_system_config();
std::string Session::cwd = get_curr_working_dir();
Emitter Session::emitter = Emitter();
ErrorHandler Session::handler = ErrorHandler(emitter);