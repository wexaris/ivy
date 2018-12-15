#pragma once
#include "token_type.hpp"
#include "util/span.hpp"
#include <string>

/* The token class.
 * Has a type and position.
 * Also stores the string of text it originated from. */
class Token {
	/* The type of the token.
	 * Assign with a character for simple types, e.g. '.' '+' '-'
	 * Assign with a TokenType enumerator for complex types, e.g TokenType::ID */
	int ty = (int)TokenType::END;

	/* The literal string of text that the token was built from. */
	std::string_view raw_str;

	/* Information about the location of the token. */
	Span tk_span;

public:
	/* Create a token from a symbol character and location.
	 * The token's string literal is set to the character.*/
	Token(char type, const Span& sp) : ty(type), raw_str(std::string(1, type)), tk_span(sp) {}

	/* Create a token from it's type, location
	 * and store it's string literal. */
	Token(TokenType type, std::string_view str, const Span& sp)
		: ty((int)type), raw_str(str), tk_span(sp) {}

	/* Create a token from it's type, location and
	 * store it's string literal. */
	Token(int type, std::string_view str, const Span& sp)
		: ty(type), raw_str(str), tk_span(sp) {}

	/* Returns the type of the token.
	 * If the return is 0, the file has reached EOF.
	 * If the return is under 256, it can be cast to a character.
	 * If the return is over 256, it can be cast to a TokenType.
	 * The type can be translated into a string
	 * with 'translate::tk_str()' */
	inline int type() const { return ty; }

	/* Returns the span of the token.
	 * Contains information about the origin file, absolute position,
	 * as well as start and end position- line and col.
	 * If the token's span hasn't been set, an exception is thrown. */
	inline const Span& span() const { return tk_span; }

	/* Returns the literal string of text that the token was built from. */
	inline std::string_view raw() const { return raw_str; }

	bool operator==(const TokenType& other) const {
		return ty == (int)other;
	}
	bool operator!=(const TokenType& other) const {
		return ty != (int)other;
	}
};