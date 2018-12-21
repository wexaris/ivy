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

using Recovery = std::vector<int>;

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

	/* The Session's SourceMap.
	 * A complete map of the package being parsed. */
	SourceMap& source_map;

	/* The parser's internal lexer.
	 * Used to read text and build tokens from the input file. */
	Lexer lexer;

	/* The last token provided by the Lexer. */
	Token curr_tok;

	/* Splits up the current token into smaller tokens
	 * if the current token is a multi-character binary op. */
	Token split_multi_binop();

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

	/* Bump until one of a given set of characters has been reached */
	void recover_to(const Recovery& to);

	/* Creates and emits an internal compiler failure error message.
	 * Will be fatal. */
	inline void bug(const std::string& msg);

	/* Creates and emits an internal compiler failure error message about a missing feature.
	 * Will be fatal. */
	inline void unimpl(const std::string& msg);

// Expect errors
private:
	/* Ceates an "unexpected token: .., expected .." error message. */
	inline Error* err_expected(const std::string& found, const std::string& expected, int code = 0);

	inline Error* expect_symbol(char exp);
	inline Error* expect_symbol(char exp, const Recovery& to);
	inline Error* expect_primitive();
	inline Error* expect_primitive(const Recovery& to);
	inline Error* expect_ident();
	inline Error* expect_ident(const Recovery& to);
	inline Error* expect_lifetime();
	inline Error* expect_lifetime(const Recovery& to);
	inline Error* expect_keyword(TokenType ty);
	inline Error* expect_keyword(TokenType ty, const Recovery& to);
	inline Error* expect_mod_or_package();
	inline Error* expect_mod_or_package(const Recovery& to);
	inline Error* expect_block_decl();
	inline Error* expect_block_decl(const Recovery& to);

// Parsing functions based on BNFs
private:

	inline Error* ident();
	inline Error* ident(const Recovery& recovery);
	inline Error* lifetime();
	inline Error* lifetime(const Recovery& recovery);

	Path path(const Recovery& to);
	inline Attributes attributes();

	void generic_params(const Recovery& recovery);

	// type
	Error* type(const Recovery& recovery);
	Error* type_or_lt(const Recovery& recovery);
	Error* type_with_lt(const Recovery& recovery);
	Error* type_sum(const Recovery& recovery);
	Error* primitive();

	// file
	void module_decl(); // for entire file
	void item();

	void item_module(); // sub-modules

	// stmt
	void stmt_item(Attributes& attr);
	void import_item();
	void item_var(bool is_const, bool is_static);
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