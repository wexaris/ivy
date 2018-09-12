#pragma once
#include "token.hpp"
#include "../source_file.hpp"
#include <unordered_map>

/*	The SourceReader is a class meant to be inherited by the Lexer.
	Used for reading characters from a SourceFile object. */
class SourceReader {

private:
	/*	The source file that is being read. 
		Do not delete. Managed by SourceMap. */
	SourceFile* source;

	/*	Current absolute position in the read file.
		Coincides with the position of the character 'next'.
		If no source file has been set, the index is invalid
		so reading will fail. */
	int index = -1;

protected:
	/*	The current character in the source file */
	char curr = 0;
	/*	The next character in the source file */
	char next = 0;

	/*	Gets the next character.
		Returns the next character as an integer.
		Once EOF has been reached, '\0' will be returned. */
	int read_next_char();

	/*	Mutable pointer to the internal SourceFile. 
		Contains a path to the file of origin,
		as well as information about the source code.
		Use with care and don't delete. */
	inline SourceFile* src_file() { return source; }

public:
	/*	Construct a new SourceReader.
		The reading process will fail if no SourceFile will be supplied. */
	SourceReader() {}
	/*	Construct a new SourceReader given a SourceFile. */
	SourceReader(SourceFile* file) { reset(file); }

	virtual ~SourceReader() {}

	/*	Swaps the current SourceFile for a new one.
		All positioning is reset to the default and cannot be recovered.
		Makes the reading index valid. */
	void reset(SourceFile* file);

	/*	Bump characters. 
		The reader will move forward by 'n' amount of characters. 
		The 'curr' character is set to the value of the 'next' character.
		If no argument is provided, characters are bumped by one.  */
	void bump(int n = 1);

	/*	The current character in the source file */
	inline char curr_c() const { return curr; }
	/*	The next character in the source file */
	inline char next_c() const { return next; }

	/*	Current absolute position in the sourcefile.
		Coincides with the position of the character 'curr'. */
	inline int bitpos() const { return index - 2; }
	/*	Current line number.
		Coincides with the position of the character 'curr'. */
	inline int lineno() const { return source->line_from_index(bitpos()); }
	/*	Current column number.
		Coincides with the position of the character 'curr'. */
	inline int colno() const { return source->col_from_index(bitpos()); }
};

/*	The object that is responsible for tokenizing source files. 
	Inherets the SourceReader class for character reading.
	Use method 'next_token()' for building the next token from the source. */
class Lexer : public SourceReader {

private:
	/*	A map of all the keywords recognized by the compiler.
		Maps a string to an integer that corresponds to a token type. */
	std::unordered_map<std::string, int> keywords;

	/*	Inserts all of the keywords recognized by the compiler into an unordered map.
		Internal use only. */
	void load_keywords();

	/*	The main identification pattern in the tokenization process.
		Accumulates characters and builds tokens according to the language's syntax. */
	Token next_token_inner();

public:
	/*	Construct a lexer.
		The reading process will fail if no SourceFile will be supplied. */
	Lexer() { load_keywords(); }
	/*	Construct a lexer to work on the provided SourceFile. */
	Lexer(SourceFile* file) : SourceReader(file) { load_keywords(); }

	/*	Gets the next token.
		Tokens get marked with a type, location and value if necessary.
		Once EOF has been reached, '\0' will be returned. */
	Token next_token();

	/*	Test the lexer.
		Prints all of the tokens in the source file. */
	static void test_print_tokens(std::string file);
};