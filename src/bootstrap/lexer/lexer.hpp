#pragma once
#include "driver/session.hpp"
#include "token/token.hpp"

/* General Translation Unit reader. 
 * Tracks current reading position.
 * Use 'curr_c()' to get the current character.
 * Use 'next_c()' to peek the next character.
 * Bump the characters by calling 'bump()'. */
class SourceReader {

private:
	/* Current absolute position in the read file.
	 * Coincides with the position of the character 'next'.
	 * If no source file has been set, the index is invalid,
	 * so source code reading will fail. */
	size_t index = 0;

	/* The current line position in the source file */
	int curr_ln = 1;
	/* The current column position in the source file */
	int curr_col = 1;

protected:
	/* The file that is being read  */
	const TranslationUnit& translation_unit;
	std::string_view src;

	/* The current character in the source file */
	char curr;
	/* The next character in the source file */
	char next;

	/* Gets the next character.
	 * Once EOF has been reached, '\0' will be returned. */
	char read_next_char();

public:
	/* Construct a new SourceReader given a Translation Unit. */
	explicit SourceReader(const TranslationUnit& tu) : translation_unit(tu), src(tu.source()) {
		// Set up the next character
		next = read_next_char();
		// 'read_next_char()' doesn't increase the index anymore,
		// since that cut off the last two characters of the file
		// because the 'curr' char was two positions behind the real read position
		index++;
		// Bump the next character to the current one
		// Bump instead of reading again because we want positioning to be updated
		bump();
	}

	virtual ~SourceReader() = default;

	/* Bump characters.
	 * The reader will move forward by 'n' amount of characters.
	 * The 'curr' character is set to the value of the 'next' character.
	 * If no argument is provided, characters are bumped by one. */
	void bump(int n = 1);

	/* A pointer to the current source file */
	inline const TranslationUnit* trans_unit() const { return &translation_unit; }

	/* The current character in the source file */
	inline char curr_c() const { return curr; }
	/* The next character in the source file */
	inline char next_c() const { return next; }

	/* Current absolute position in the Translation Unit.
	 * Coincides with the position of the character 'curr'. */
	inline size_t bitpos() const { return index - 2; }
	/* Current line number.
	 * Coincides with the position of the character 'curr'. */
	inline int lineno() const { return curr_ln; }
	/* Current column number.
	 * Coincides with the position of the character 'curr'. */
	inline int colno() const { return curr_col; }
};

/* Translation Unit lexer and tokenizer.
 * Inherits from the SourceReader class.
 * Use method 'next_token()' for getting the next token from the source. */
class Lexer : protected SourceReader {

private:
	/* The main identification pattern in the tokenization process.
	 * Accumulates characters and builds tokens according to the language's syntax. */
	Token next_token_inner();

	/* Throws a spanned error through the current Session.
	 * Does not return. */
	[[noreturn]] inline void err(const std::string& msg, const Span& sp) const {
		Session::span_err(msg, sp);
		std::exit(1);
	}

protected:
	/* Bumps past whitespace and comments. */
	void consume_ws_and_comments();

	/* Lexes and checks any following number.
	 * Returns a 'LIT_INTEGER' or 'LIT_FLOAT' token. */
	Token lex_number();

	/* Read through any digits.
	 * Check if they correspont to the given base.
	 * Any value outside of the 'base' but inside the 'full_base'
	 * is considered an invalid value.
	 * Any value outside of the 'full_base' is considered not part
	 * of the number. */
	void scan_digits(int base, int full_base);

	/* Reads a float's exponent if any. */
	void scan_exponent();

	/* Validate hexadecimal escape characters.
	 * Scans 'num' amount of characters until the 'delim' character is reached.
	 * Returns wether the escape is valid, */
	bool scan_hex_escape(uint num, char delim);

	/* Saves the current Span position. */
	inline void save_curr_pos() {
		curr_start.abs = bitpos();
		curr_start.ln = lineno();
		curr_start.col = colno();
	}

	/* Returns the current token's span.
	 * Relies on a correctly set 'curr_start' position. */
	inline Span curr_span() const {
		return Span(trans_unit(), curr_start.abs, curr_start.ln, curr_start.col, bitpos(), lineno(), colno());
	}

	/* The current tokens's absolute length. */
	inline size_t curr_length() { return bitpos() - curr_start.abs; }
	/* Extract's the current token's view from the TU string.  */
	inline std::string_view curr_src_view() { return src.substr(curr_start.abs, curr_length()); }

public:
	/* Saved start position of the current token. */
	struct TokenPos {
		size_t abs = 0;
		int ln, col;
	} curr_start;

public:
	/* Construct a lexer to work on the provided Translation Unit. */
	explicit Lexer(const TranslationUnit& file) : SourceReader(file) {}
	/* Copy constructor */
	Lexer(const Lexer& other) : SourceReader(other.translation_unit) {}

	/* Gets the next token.
	 * Tokens get marked with a type, location and value if necessary.
	 * Once EOF has been reached, '\0' will be returned. */
	Token next_token();
};