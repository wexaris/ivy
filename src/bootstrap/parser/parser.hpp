#pragma once
#include "driver/session.hpp"
#include "source/source_map.hpp"
#include "lexer/lexer.hpp"
#include <algorithm>

struct Node;

/* A single attribute.
 * Has an int for the type and a Span. */
struct Attr {
	TokenType ty;
	Span sp;
};
/* A vector of attributes. */
struct Attributes : std::vector<Attr> {

	bool contains(TokenType ty);

	inline bool none() {
		return this->size() == 0;
	}
};

/* A single part of a path.
 * Has a string and Span. */
struct SubPath {
	std::string id;
	Span span;
};
/* A vector of SubPaths. */
using Path = std::vector<SubPath>;

/* The workhorse of the compiler's frontend.
 * Responsible for scanning the input grammar.
 * Internally uses a Lexer for text reading and tokenization. */
class Parser {

private:
	/* A complete source code map of the package being parsed. */
	SourceMap source_map;

	/* The parser's internal lexer.
	 * Used to read text and build tokens from the input file.
	 * Call 'next_token()' whenever a new token is required. */
	Lexer lexer;

	/* The last token provided by the lexer.
	 * Stores a type, location and value, if necessary. */
	Token curr_tok;

	/* Bumps the current token.
	 * The current one becomes the previous one.
	 * A new token is read as the new one. */
	inline void bump(int n = 1) {
		for (int i = 0; i < n; i++)
			curr_tok = lexer.next_token();
	}

	/* Requests the Session to print a trace message.
	 * The Session won't print if verbosity is disabled. */
	inline void trace(const std::string& msg) const { Session::trace(msg); }
	/* Requests the Session to remove one level of trace indentation. */
	inline void end_trace() const { Session::end_trace(); }

	/* Throws a spanned error through the current Session. */
	[[noreturn]] inline void err(const std::string& msg) const {
		Session::span_err(msg, curr_tok.span());
		std::exit(1);
	}

	/* Emits an 'expected token' message:
	 * unexpected token: <curr_tok>; expected <expected> */
	void expected_msg(const std::string& expected);

	/* Expects the current token to be of the provided type.
	 * Bumps if true, throws an error if false. */
	inline void expect(TokenType exp) { expect((int)exp); }
	/* Expects the current token to be of the provided type.
	 * Bumps if true, throws an error if false. */
	void expect(int exp);
	/* Expects the current token to be any of the provided types.
	 * Bumps if true, throws an error if false. */
	void expect(const std::vector<TokenType>& exp);

	/* Can interfere with normal token tree parsing,
	 * so && is built on demand. */
	inline void AND() {
		expect('&');
		expect('&');
	}
	/* Can interfere with normal token tree parsing,
	 * so >> is built on demand. */ 
	inline void SHR() {
		expect('>');
		expect('>');
	}

// Parsing functions based on BNFs
private:

	void ident();
	void lifetime();

	Path path();
	Attributes attributes();

	void generic_params();

	// type
	void type();
	void type_or_lt();
	void type_with_lt();
	void type_sum();
	void primitive();

	// file
	void module_decl();
	void item();

	// stmt
	void stmt_item();
	void import_item();
	void item_static();
	void item_const();
	void item_type();
	void view_item();
	void block_item();
	void item_fun();
	void item_union();
	void item_trait();
	void item_impl();

	void item_struct();
	void struct_tuple_block();
	void struct_tuple_items();
	void struct_decl_block();
	void struct_decl_items();
	void struct_decl_item();

	void item_enum();
	void enum_defs();
	void enum_def();

	// expr
	void expr();

	/* There shouldn't be any reason to contstruct multiples of the same parser. */
	Parser(const Parser& other) = default;

public:
	/* Constructs a parser for the file at the provided location. */
	Parser(const std::string& filepath)
		: lexer(source_map.load_file(filepath)), curr_tok(lexer.next_token())
	{}

	/* Begins the process of parsing the package.
	 * Returns the root node of the abstract syntax tree. */
	Node* parse();
};