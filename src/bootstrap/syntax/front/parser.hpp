#pragma once
#include "lexer.hpp"
#include "../source_map.hpp"
#include <memory>

/*	The workhorse of the compiler's frontend.
	Responsible for scanning the input grammar.
	Internally uses a lexer for text reading and tokenization. */
class Parser {

private:
	/* 	A complete source code map of the package being parsed. */
	SourceMap source_map;

	/*	The parser's internal lexer.
		Used to read text and build tokens from the input file.
		Call 'next_token()' whenever a new token is requred. */
	std::unique_ptr<Lexer> lex;
	/*	The last token provided by the lexer.
		Stores a type, location and value, if necessary. */
	Token token;

	/*	The symbol table used by the lexer and parser.
		Used to track important token values, such as identifiers, for later use. */
	std::unordered_map<int, std::string> symbol_table;

	/*	Matches the current token type to the one expected.
		The input type is the one the parser expects to receive.
		If the types do not match, the program terminates. */
	void expect(int type);

public:
	/*	Constructs a parser for the file at the provided location. */
	Parser() { lex = std::unique_ptr<Lexer>(new Lexer()); }
	/*	Constructs a parser for the file at the provided location. */
	Parser(std::string filepath) { lex = std::unique_ptr<Lexer>(new Lexer(source_map.load_file(filepath))); }
};