#pragma once
#include "driver/session.hpp"
#include "source/translation_unit.hpp"
#include "token/token.hpp"
#include <unordered_map>

/* The SourceReader is a class meant to be inherited by the Lexer.
 * Used for reading characters from a Translation Unit object. */
class SourceReader {

private:
	/* Current absolute position in the read file.
	 * Coincides with the position of the character 'next'.
	 * If no source file has been set, the index is invalid,
	 * so source code reading will fail. */
	size_t index = 0;

protected:
	/* The file that is being read  */
	TranslationUnit* translation_unit;
	const std::string& src;

	/* The current character in the source file */
	char curr;
	/* The next character in the source file */
	char next;

	/* The current line position in the source file */
	int curr_ln = 1;
	/* The current column position in the source file */
	int curr_col = 1;

	/* Gets the next character.
	 * Returns the next character as an integer.
	 * Once EOF has been reached, '\0' will be returned. */
	char read_next_char();

public:
	/* Construct a new SourceReader given a Translation Unit. */
	explicit SourceReader(TranslationUnit* tu) : translation_unit(tu), src(tu->source()) {
		// Set the next character and bump it to the current one
		// Bump because that updates the newline indexes in the Translation Unit
		// Reset the column counter for it to correspond to the character 'curr'
		next = read_next_char();
		bump();
	}

	virtual ~SourceReader() = default;

	/* Swaps the current Translation Unit for a new one.
	 * All positioning is reset to the default. */
	void reset(TranslationUnit* trans_unit);

	/* Bump characters.
	 * The reader will move forward by 'n' amount of characters.
	 * The 'curr' character is set to the value of the 'next' character.
	 * If no argument is provided, characters are bumped by one. */
	void bump(int n = 1);

	/* A pointer to the current source file */
	inline const TranslationUnit* trans_unit() const { return translation_unit; }

	/* The current character in the source file */
	constexpr inline char curr_c() const { return curr; }
	/* The next character in the source file */
	constexpr inline char next_c() const { return next; }

	/* Current absolute position in the Translation Unit.
	 * Coincides with the position of the character 'curr'. */
	constexpr inline size_t bitpos() const { return index - 2; }
	/* Current line number.
	 * Coincides with the position of the character 'curr'. */
	constexpr inline int lineno() const { return curr_ln; }
	/* Current column number.
	 * Coincides with the position of the character 'curr'. */
	constexpr inline int colno() const { return curr_col; }
};

/* The object that is responsible for tokenizing source files.
 * Inherits the SourceReader class for character reading.
 * Use method 'next_token()' for building the next token from the source. */
class Lexer : public SourceReader {

private:
	/* The main identification pattern in the tokenization process.
	 * Accumulates characters and builds tokens according to the language's syntax. */
	Token next_token_inner();

public:
	/* Construct a lexer to work on the provided Translation Unit. */
	explicit Lexer(TranslationUnit* file) : SourceReader(file) {}

	Lexer(const Lexer& other) : SourceReader(other.translation_unit) {}

	/* Throws a spanned error through the current Session.
	 * Does not return. */
	[[noreturn]] inline void err(const std::string& msg, const Span& sp) const {
		Session::span_err(msg, sp);
		std::exit(1);
	}

	/* Gets the next token.
	 * Tokens get marked with a type, location and value if necessary.
	 * Once EOF has been reached, '\0' will be returned. */
	Token next_token();
};