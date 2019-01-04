#pragma once
#include "driver/session.hpp"
#include "source/source_map.hpp"
#include "lexer/lexer.hpp"
#include "ast/ast.hpp"

using Recovery = std::vector<int>;

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

	/* Bump until one of a given set of characters has been reached. */
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
	inline Error* expect_block_decl();
	inline Error* expect_block_decl(const Recovery& to);
	inline Error* expect_mod_or_package();
	inline Error* expect_mod_or_package(const Recovery& to);
	inline Error* expect_keyword(TokenType ty);
	inline Error* expect_keyword(TokenType ty, const Recovery& to);

	/* Expect a symbol, recover on failure and check again.
	 * A lot of parse items end with a symbol check, recovery, recheck,
	 * so this small fucntion does that very same thing for us. */
	void expect_sym_recheck(char expr, const Recovery& recov);

// Parsing functions based on BNFs
private:
	// items
	ast::TypePrimitive* primitive();
	ast::Ident* ident();
	ast::Lifetime* lifetime();
	ast::Value* literal();
	Error* unaryop();
	inline ast::Ident* ident(const Recovery& recovery);
	inline ast::Lifetime* lifetime(const Recovery& recovery);
	inline ast::Value* literal(const Recovery& recovery);
	inline Error* unaryop(const Recovery& recovery);

	// collectors
	inline Attributes attributes();
	ast::Path* path(const Recovery& to);

	// helping item collections
	GenericParamVec generic_params(const Recovery& recovery);
	std::tuple<Error*, ast::GenericParam*> generic_param(const Recovery& recovery);

	ParamVec param_list(bool is_method, const Recovery& recovery);
	std::tuple<Error*, ast::Param*> param(const Recovery& recovery);
	Error* param_self(const Recovery& recovery);

	ExprVec arg_list(const Recovery& recovery);
	std::tuple<Error*, ast::Expr*> arg(const Recovery& recovery);

	std::tuple<Error*, ast::Type*> return_type(const Recovery& recovery);

	StructFieldVec struct_init(const Recovery& recovery);
	std::tuple<Error*, IDExprPair> struct_field(const Recovery& recovery);

	ExprVec arr_init(const Recovery& recovery);
	std::tuple<Error*, ast::Expr*> arr_field();

	// decl
	ast::Decl* decl(bool is_global);
	ast::DeclModule* decl_module(bool is_global);
	std::vector<std::unique_ptr<ast::Decl>> module_block();
	ast::Decl* decl_import_item();
	ast::DeclVar* decl_var(bool is_const, bool is_static);
	ast::DeclType* decl_type();
	ast::DeclUse* decl_use();

	ast::DeclFun* decl_fun(bool is_method);
	FunBlock fun_block();

	void decl_trait();
	void trait_block();

	void decl_impl();
	void impl_block();

	void decl_union();
	void decl_struct();
	void struct_tuple_block();
	void struct_tuple_item(const Recovery& recovery);
	void struct_named_block();
	void struct_named_item(const Recovery& recovery);

	void decl_enum();
	void enum_block();
	Error* enum_item(const Recovery& recovery);

	// expr
	std::tuple<Error*, ast::Expr*> expr(int min_prec);
	std::tuple<Error*, ast::Expr*> expr(int min_prec, const Recovery& recovery);
	std::tuple<Error*, ast::Value*> val(const Recovery& recovery);
	ast::UnaryOp* unary_op();

	// stmt
	ast::Stmt* stmt(const Recovery& recovery);
	void stmt_if(const Recovery& recovery);
	void stmt_else(const Recovery& recovery);
	void stmt_loop(const Recovery& recovery);
	void stmt_while(const Recovery& recovery);
	void stmt_do(const Recovery& recovery);
	void stmt_for(const Recovery& recovery);
	void stmt_match(const Recovery& recovery);
	void stmt_switch(const Recovery& recovery);
	void stmt_case(const Recovery& recovery);
	ast::StmtReturn* stmt_return(const Recovery& recovery);
	ast::StmtBreak* stmt_break(const Recovery& recovery);
	ast::StmtContinue* stmt_continue(const Recovery& recovery);

	// type
	std::tuple<Error*, ast::Type*> type(const Recovery& recovery);
	std::tuple<Error*, ast::TypeRef*> type_ref(const Recovery& recovery);
	std::tuple<Error*, ast::TypePtr*> type_ptr(const Recovery& recovery);
	std::tuple<Error*, ast::Type*> type_tuple(const Recovery& recovery);
	std::tuple<Error*, ast::Type*> type_arr_or_slice(const Recovery& recovery);
	ast::TypePath* type_path(const Recovery& recovery);
	ast::TypeInfer* type_infer();
	ast::TypePrimitive* type_primitive();
	std::tuple<Error*, ast::GenericParam*> type_or_lt(const Recovery& recovery);
	std::tuple<Error*, ast::Lifetime*, ast::Type*> type_with_lt(const Recovery& recovery);

public:
	/* Constructs a parser for the file at the provided location. */
	Parser(SourceMap& src_map, const std::string& filepath)
		: handler(Session::handler), source_map(src_map), lexer(source_map.load_file(filepath), handler), curr_tok(lexer.next_token())
	{}

	/* There shouldn't be any reason to contstruct multiples of the same parser. */
	Parser(const Parser& other) = delete;

	/* Begins the process of parsing the package.
	 * Returns the root node of the abstract syntax tree. */
	std::shared_ptr<ASTRoot> parse();
};