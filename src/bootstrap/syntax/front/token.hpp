#pragma once
#include "token_type.hpp"
#include <cstdint>
#include <string>

/*	Structure containing the values that a token could hold. 
	Stores a string, int, uint and float. 
	The values are as large as the system(x86/x64) allows.
	This is not a union, so values are not exclusive. */
struct TokenVal {
	/*	The string value of a token.
	Empty if type is not STRING. */
	std::string str = "";
	/*	The signed integer value of a token.
	Zero if type is not a INT. */
	intmax_t sint = 0;
	/*	The unsigned integer value of a token.
	Zero if type is not a UINT. */
	uintmax_t uint = 0;
	/*	The floating point value of a token.
	Zero if type is not a FLOAT. */
	double real = 0;
};

/*	Structure containing the path to a file,
	as well as a start and end position in the text. */
struct Location {
	/*	Path to the file of origin. */
	std::string file;

	/*	Absolute position in a text file */
	int bitpos;
	int len;

	/*	Position in a text file- a line and column number. */
	struct { int ln, col; } pos;

	/*	Create location with a set filepath, absolute position, line, column and length. */
	Location(std::string filepath, int bit, int ln, int col, int len)
		: file(filepath), bitpos(bit), len(len), pos { ln, col } { }
};

/*	The token class. 
	Created by the lexer for use in parsing.
	The token has a type and might contain a value.
	It also stores the token's location. */
class Token {
	/*	The type of the token. 
		Assign with a character for simple types, e.g. '.' '+' '-'
		Assign with a TokenType enumerator for complex types, e.g. TokenType::ID */
	int ty;

	/*	The values that a token can store.
		Stores a string, int, uint and double.
		The values are as large as the system(x86/x64) allows.
		This is not a union, so values are not exclusive. */
	TokenVal value;

	/*	Information about the location of the token.
		Contains information about the origin file, absolute position,
		as well as start and end position- line and col, */
	Location location;

public:

	/*	Create a token from it's type and location. */
	Token(int type = 0, Location loc = Location("", 0, 0, 0, 0))
		: ty(type), location(loc) { }

	/*	Create a token from it's type, location and store a string. */
	Token(int type, std::string str, Location loc = Location("", 0, 0, 0, 0)) 
		: ty(type), location(loc) { value.str = str; }

	/*	Create a token from it's type, location and store an integer. */
	Token(int type, intmax_t sint, Location loc = Location("", 0, 0, 0, 0))
		: ty(type), location(loc) { value.sint = sint; }

	/*	Create a token from it's type, location and store an unsigned integer. */
	Token(int type, uintmax_t uint, Location loc = Location("", 0, 0, 0, 0))
		: ty(type), location(loc) { value.uint = uint; }

	/*	Create a token from it's type, location and store a float. */
	Token(int type, double real, Location loc = Location("", 0, 0, 0, 0))
		: ty(type), location(loc) { value.real = real; }

	/*	Returns the type of the token.
		If the return is 0, the file has reached EOF.
		If the return is under 256, it can be cast to a char.
		If the return is over 255, it can be translated into a string
		with 'translate::tk_str()' in 'token_translate.h' */
	inline int type() const { return ty; }

	/*	Returns the location of the token.
		Contains information about the origin file, absolute position,
		as well as start and end position- line and col, */
	inline const Location& loc() const { return location; }

	/*	Set the location of the token.
		Contains information about the origin file, absolute position,
		as well as start and end position- line and col, */
	inline Token& loc(std::string filepath, int bit, int ln, int col, int len) {
		location = Location(filepath, bit, ln, col, len);
		return *this;
	}

	/*	Returns the value of the token.
		The value can be a string, int, uint or float.
		The values are as large as the system(x86/x64) allows.
		This is not a union, so values are not exclusive. */
	inline const TokenVal& val() const { return value; }
};