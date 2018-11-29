#include "session.hpp"

SysConfig Session::cfg = SysConfig();
Severity Session::_req_severity = Severity::MESSAGE;
bool Session::_format = true;
std::string Session::indent_prefix;

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

struct System {
	static inline OS get_os()				{ return _OS; }
	static inline Arch get_arch() 		{ return _ARCH; }
	static inline SysType get_isize() 	{ return _ARCH == Arch::x64 ? SysType::i64 : SysType::i32; }
	static inline SysType get_usize() 	{ return _ARCH == Arch::x64 ? SysType::u64 : SysType::u32; }
	static inline SysType get_fsize() 	{ return _ARCH == Arch::x64 ? SysType::f64 : SysType::f32; }
};

SysConfig::SysConfig()
	: os(System::get_os()),
	arch(System::get_arch()),
	isize(System::get_isize()),
	usize(System::get_usize()),
	fsize(System::get_fsize())
{}