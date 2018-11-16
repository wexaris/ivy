#pragma once
#include "driver/session.hpp"
#include "source/source_file.hpp"
#include "token/token.hpp"
#include <unordered_map>

/* The SourceReader is a class meant to be inherited by the Lexer.
 * Used for reading characters from a SourceFile object.
 */
class SourceReader {

private:
	/* Current absolute position in the read file.
	 * Coincides with the position of the character 'next'.
	 * If no source file has been set, the index is invalid,
	 * so source code reading will fail.
	 */
	size_t index = SIZE_MAX;

protected:
	/* Contains the current SourceFile and a shorthand to it's source. */
	struct SFContext {
		SourceFile& file;
		const std::string& src;
	};
	/* A possible SFContext object.
	 * Could not exist, if the code doesn't originate from a file. */
	std::optional<SFContext> sf = std::nullopt;

	/* The current character in the source file */
	char curr = 0;
	/* The next character in the source file */
	char next = 0;

	/* The current line position in the source file */
	int curr_ln = 1;
	/* The current column position in the source file */
	int curr_col = 1;

	/* Gets the next character.
	 * Returns the next character as an integer.
	 * Once EOF has been reached, '\0' will be returned.
	 */
	char read_next_char();

public:
	/* Construct a new SourceReader given a SourceFile. */
	explicit SourceReader(SourceFile& file) : index(0), sf(SFContext{ file, file.source() }) {
		// Set the next character and bump it to the current one
		// Bump because that updates the newline indexes in the SourceFile
		// Reset the column counter for it to correspond to the character 'curr'
		next = read_next_char();
		bump();
	}

	virtual ~SourceReader() = default;

	/* Swaps the current SourceFile for a new one.
	 * All positioning is reset to the default and cannot be recovered.
	 * Makes the reading index valid.
	 */
	void reset(SourceFile& file);

	/* Bump characters.
	 * The reader will move forward by 'n' amount of characters.
	 * The 'curr' character is set to the value of the 'next' character.
	 * If no argument is provided, characters are bumped by one.
	 */
	void bump(int n = 1);

	/* A pointer to the current source file */
	inline SourceFile& src() const { if (!sf.has_value()) throw std::exception(); return sf.value().file; }

	/* The current character in the source file */
	constexpr inline char curr_c() const { return curr; }
	/* The next character in the source file */
	constexpr inline char next_c() const { return next; }

	/* Current absolute position in the sourcefile.
	 * Coincides with the position of the character 'curr'.
	 */
	constexpr inline size_t bitpos() const { return index - 2; }
	/* Current line number.
	 * Coincides with the position of the character 'curr'.
	 */
	constexpr inline int lineno() const { return curr_ln; }
	/* Current column number.
	 * Coincides with the position of the character 'curr'.
	 */
	constexpr inline int colno() const { return curr_col; }
};

/* The object that is responsible for tokenizing source files.
 * Inherits the SourceReader class for character reading.
 * Use method 'next_token()' for building the next token from the source. */
class Lexer : public SourceReader {

private:
	using KeywordMap = std::unordered_map<std::string, int>;

	/* A map of all the keywords recognized by the compiler.
	 * Maps a string to an integer that corresponds to a token type.
	 */
	KeywordMap keywords = KeywordMap();

	/* Maps all of the recognised keywords to their corresponding string literals. */
	void load_keywords();

	/* The main identification pattern in the tokenization process.
	 * Accumulates characters and builds tokens according to the language's syntax.
	 */
	Token next_token_inner();

public:
	/* Construct a lexer to work on the provided SourceFile. */
	explicit Lexer(SourceFile& file) : SourceReader(file) { load_keywords(); }

	Lexer(const Lexer& other) : SourceReader(other.sf.value().file) { load_keywords(); }

	/* Throws a spanned error through the current Session.
	 * Does not return.
	 */
	[[noreturn]] inline void err(const std::string& msg, const Span& sp) const {
		Session::span_err(msg, sp);
		std::exit(1);
	}

	/* Gets the next token.
	 * Tokens get marked with a type, location and value if necessary.
	 * Once EOF has been reached, '\0' will be returned.
	 */
	Token next_token();

	/* Tests for assessing the Lexer's functionality. */
	static void test_print_tokens();
	static void test_read_without_source();
};
