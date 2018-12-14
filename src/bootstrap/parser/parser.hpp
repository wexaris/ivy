#pragma once
#include "driver/session.hpp"
#include "source/source_map.hpp"
#include "lexer/lexer.hpp"

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
	std::string_view id;
	Span span;
};
/* A vector of SubPaths. */
using Path = std::vector<SubPath>;

/* The workhorse of the compiler's frontend.
 * Responsible for scanning the input grammar.
 * Internally uses a Lexer for text reading and tokenization. */
class Parser {

private:
	/* The Session's ErrorHandler */
	ErrorHandler& handler;

	/* A complete source code map of the package being parsed. */
	SourceMap& source_map;

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

	/* Requests trace message to be printed.
	 * Won't be printed if tracing is disabled. */
	inline void trace(const std::string& msg) const { handler.trace(msg); }
	/* Requests a level of indentation to be removed from trace messages. */
	inline void end_trace() const { handler.end_trace(); }

	/* Ceates an "unexpected token: .., expected .." error message. */
	inline Error* err_expected(const std::string& found, const std::string& expected, int code = 0);

	inline Error* expect_symbol(char exp);
	inline Error* expect_primitive();
	inline Error* expect_lifetime();
	inline Error* expect_keyword(TokenType ty);
	inline Error* expect_mod_or_package();
	inline Error* expect_block_item_decl();

	/* Creates and emits an internal compiler failure error message.
	 * Will be fatal. */
	inline void bug(const std::string& msg);

	/* Creates and emits an internal compiler failure error message about a missing feature.
	 * Will be fatal. */
	inline void unimpl(const std::string& msg);

	/* Can interfere with normal token tree parsing,
	 * so && is built on demand. */
	inline void AND();
	/* Can interfere with normal token tree parsing,
	 * so >> is built on demand. */ 
	inline void SHR();

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
	Parser(const Parser& other) = delete;

public:
	/* Constructs a parser for the file at the provided location. */
	Parser(SourceMap& src_map, const std::string& filepath)
		: handler(Session::handler), source_map(src_map), lexer(source_map.load_file(filepath), handler), curr_tok(lexer.next_token())
	{}

	/* Begins the process of parsing the package.
	 * Returns the root node of the abstract syntax tree. */
	Node* parse();
};