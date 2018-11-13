#include <utility>

#pragma once
#include "token_type.hpp"
#include "util/span.hpp"
#include <cstdint>
#include <string>
#include <optional>

#include "source/source_file.hpp"

/* The token class.
 * Has a type and might contain a value.
 * Also stores the token's location.
 */
class Token {
	/* The type of the token.
	 * Assign with a character for simple types, e.g. '.' '+' '-'
	 * Assign with a TokenType enumerator for complex types, e.g TokenType::ID
	 */
	int ty = (int)TokenType::END;

	/* The literal string of text that the token was built from.
	 * If the token is a keyword, there's no point in storing it's literal.
	 * Only identifiers and literal types are stored.
	 * Will be empty otherwise.
	 */
	std::string str = "";

	/* Information about the location of the token. */
	std::optional<Span> tk_span = std::nullopt;

public:
	/* Create an incomplete token.
	 * Has a type, but no specified span.
	 * Used in the Lexer, so we don't have to pass span positions around.
	 * Span can and should be added later.
	 */
	explicit Token(TokenType type = TokenType::END) : ty((int)type) {}
	/* Create a token from it's type and location.
	 * Literal value will be left empty.
	 */
	Token(TokenType type, const Span& sp)
		: ty((int)type),
		tk_span(sp)
	{}
	/* Create an incomplete token.
	 * Has a type and string, but no specified span.
	 */
	Token(TokenType type, const std::string& str)
		: ty((int)type),
		str(str)
	{}
	/* Create a token from it's type, location
	 * and store it's string literal.
	 */
	Token(TokenType type, const std::string& str, const Span& sp)
		: ty((int)type),
		str(str),
		tk_span(sp)
	{}

	/* Create an incomplete token.
	 * Has a type, but no specified span.
	 * Used in the Lexer, so we don't have to pass span positions around.
	 * Span can and should be added later.
	 */
	explicit Token(int type = (int)TokenType::END) : ty(type) {}
	/* Create a token from it's type.
	 * Literal value will be left empty.
	 */
	Token(int type, const Span& sp) 
		: ty(type),
		tk_span(sp)
	{}
	/* Create an incomplete token.
	 * Has a type and string, but no specified span.
	 */
	Token(int type, const std::string& str)
		: ty(type),
		str(str)
	{}
	/* Create a token from it's type, location and
	 * store it's string literal.
	 */
	Token(int type, const std::string& str, const Span& sp)
		: ty(type),
		str(str),
		tk_span(sp)
	{}

	/* Returns the type of the token.
	 * If the return is 0, the file has reached EOF.
	 * If the return is under 256, it can be cast to a character.
	 * If the return is over 256, it can be cast to a TokenType.
	 * The type can be translated into a string
	 * with 'translate::tk_str()'
	 */
	inline int type() const { return ty; }

	/* Returns the span of the token.
	 * Contains information about the origin file, absolute position,
	 * as well as start and end position- line and col.
	 * If the token's span hasn't been set, an exception is thrown.
	 */
	inline const Span& span() const { if (!tk_span.has_value()) throw std::exception(); return tk_span.value(); }

	/* Set the location of the token.
	 * Contains information about the origin file, absolute position,
	 * as well as start and end position- line and col,
	 */
	inline void set_span(Span sp) { tk_span.emplace(sp); }

	/* Returns the literal string of text that the token was built from.
	 * Only identifiers and literal types are stored.
	 * Will be empty otherwise.
	 */
	inline const std::string& lit() const { return str; }

	Token& operator=(const Token& other) {
		ty = other.ty;
		str = other.str;
		if (other.tk_span.has_value()) tk_span.emplace(other.tk_span.value());

		return *this;
	}

	bool operator==(const TokenType& other) const {
		return ty == (int)other;
	}
	bool operator!=(const TokenType& other) const {
		return ty != (int)other;
	}
};