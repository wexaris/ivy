#include "parser.hpp"
#include "util/ranges.hpp"
#include "util/token_info.hpp"
#include "ast/ast.hpp"
#include <array>

constexpr std::array<int, 27> stmt_start = {
	(int)TokenType::IMPORT,
	(int)TokenType::EXPORT,
	(int)TokenType::USE,
	(int)TokenType::MOD,

	(int)TokenType::VAR,
	(int)TokenType::CONST,
	(int)TokenType::STATIC,
	(int)TokenType::TYPE,

	(int)TokenType::FUN,
	(int)TokenType::STRUCT,
	(int)TokenType::ENUM,
	(int)TokenType::UNION,
	(int)TokenType::MACRO,
	(int)TokenType::TRAIT,
	(int)TokenType::IMPL,

	(int)TokenType::LOOP,
	(int)TokenType::WHILE,
	(int)TokenType::DO,
	(int)TokenType::FOR,
	(int)TokenType::MATCH,
	(int)TokenType::SWITCH,
	(int)TokenType::CASE,
	(int)TokenType::RETURN,
	(int)TokenType::BREAK,

	(int)TokenType::PUB,
	(int)TokenType::PRIV,
	(int)TokenType::MUT,
};

constexpr std::array<int, 7> decl_start = {
	(int)TokenType::VAR,
	(int)TokenType::CONST,
	(int)TokenType::STATIC,
	(int)TokenType::TYPE,

	(int)TokenType::PUB,
	(int)TokenType::PRIV,
	(int)TokenType::MUT,
};

constexpr std::array<int, 6> expr_end = {
	(int)',',
	(int)':',
	(int)';',
	(int)')',
	(int)']',
	(int)'}',
};

bool Attributes::contains(TokenType ty) {
	for (auto attr : *this)
		if (attr.ty == ty)
			return true;
	return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////      Helper Functions      ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/* Scans and adds characters to a string as long as their base is under the one specified. */
/*std::string scan_digits(Lexer* lex, int base) {
	// String for storing digits
	std::string ret;
	// Get digits as long as they are hex
	while (range::is_hex(lex->curr_c())) {
		// If the hex value is less than the base, add it to the return
		// If not, stop scanning
		if (range::val_hex(lex->curr_c(), lex) < base) { ret += lex->curr_c(); lex->bump(); }
		else break;
	}
	return ret;
}*/

/* Scans and adds characters that correspond to an exponent. */
/*std::string scan_exponent(Lexer* lex) {
	// String for storing exponent sign
	std::string ret;
	if (lex->curr_c() == 'e' || lex->curr_c() == 'E') {
		ret += lex->curr_c();
		lex->bump();
		if (lex->curr_c() == '-' || lex->curr_c() == '+') {
			ret += lex->curr_c();
			lex->bump();
		}
		// String for storing exponent value
		std::string expo = scan_digits(lex, 10);

		// Handle no exponent value
		if (expo.length() == 0) throw std::runtime_error("Bad floating point number literal");

		return ret + expo;
	}
	// No exponent
	return "";
}*/

/* Responsible for constructing numbers from read characters. */
/*Token scan_number(Lexer* lex) {
	// The base for counting
	int base = 10;

	// Handle possible number bases
	if (lex->curr_c() == '0' && (lex->next_c() == 'x' || lex->next_c() == 'X')) 	{ base = 16; lex->bump(2); }
	else if (lex->curr_c() == '0' && lex->next_c() == 'b') 							{ base = 2; lex->bump(2); }
	else if (lex->curr_c() == '0' && lex->next_c() == 'o') 							{ base = 8; lex->bump(2); }

	// The number string
	std::string num_str = scan_digits(lex, base);

	// Handle int declaration
	if (lex->curr_c() == 'i' || lex->curr_c() == 'u') {
		// Set whether integer is signed
		bool sign = (lex->curr_c() == 'i');

		// TODO: Create AST node - signed or unsigned

		lex->bump();

		// Set integer size
		if (lex->curr_c() == '8') { lex->bump();
			// TODO: Create AST node - i8 or u8
		}
		else if (lex->curr_c() == '1' && lex->next_c() == '6') { lex->bump(2);
			// TODO: Create AST node - i16 or u16
		}
		else if (lex->curr_c() == '3' && lex->next_c() == '2') { lex->bump(2);
			// TODO: Create AST node - i32 or u32
		}
		else if (lex->curr_c() == '6' && lex->next_c() == '4') { lex->bump(2);
			// TODO: Create AST node - i64 or u64
		}
		// The parsed number
		int parsed;
		{
			auto i = num_str.length() - 1;
			int power = 1u, n = 0;
			while (true) {
				int digit = range::val_hex(num_str[i], lex);
				if (digit >= base) throw std::runtime_error("Digit larger than base");
				n += digit * power;
				power *= base;
				if (i == 0) parsed = n;
				i--;
			}
		}
		//return sign ? Token(LIT_INT, (intmax_t)parsed) : Token(LIT_UINT, (uintmax_t)parsed);
	}
	// Whether the number is a float
	bool is_float = false;

	// Get decimal place
	if (lex->curr_c() == '.' && !range::is_alpha(lex->next_c())) {
		is_float = true;
		lex->bump();
		std::string dec_str = scan_digits(lex, 10);
		num_str += "." + dec_str;
	}

	// Get exponent
	std::string expo_str = scan_exponent(lex);
	if (!expo_str.empty()) {
		is_float = true;
		num_str += expo_str;
	}

	// Handle float declaration
	if (lex->curr_c() == 'f') {
		lex->bump();
		if (lex->curr_c() == '3' && lex->next_c() == '2') {
			lex->bump(2);
			// TODO: Create AST node - f32
			return Token(TokenType::LIT_FLOAT, num_str);
		}
		else if (lex->curr_c() == '6' && lex->next_c() == '4') {
			lex->bump(2);
			// TODO: Create AST node - f64
			return Token(TokenType::LIT_FLOAT, num_str);
		}
		else
			is_float = true;
	}

	if (is_float)
		// TODO: Create AST node - float
		return Token(TokenType::LIT_FLOAT, num_str);

	// The parsed number
	int parsed;
	{
		auto i = num_str.length();
		int power = 1u, n = 0;
		while (i > 0) {
			int digit = range::val_hex(num_str[i-1], lex);
			if (digit >= base) throw std::runtime_error("Digit larger than base");
			n += digit * power;
			power *= base;
			i--;
		}
		parsed = n;
	}
	// TODO: Create AST node - int
	//return Token(LIT_INT, (intmax_t)parsed);
}*/

/* True if the provided token is a primitive. */
inline bool is_primitive(const Token& tk) {
	return tk == TokenType::THING || tk == TokenType::STR || tk == TokenType::CHAR ||
		tk == TokenType::INT ||
		tk == TokenType::I64 || tk == TokenType::I32 || tk == TokenType::I16 || tk == TokenType::I8 ||
		tk == TokenType::UINT ||
		tk == TokenType::U64 || tk == TokenType::U32 || tk == TokenType::U16 || tk == TokenType::U8 ||
		tk == TokenType::FLOAT ||
		tk == TokenType::F32 || tk == TokenType::F64;
}

/* True if the provided token's type is a literal. */
inline bool is_literal(const Token& tk) {
	return tk == TokenType::LIT_STRING		||
			tk == TokenType::LIT_CHAR		||
			tk == TokenType::LIT_INTEGER	||
			tk == TokenType::LIT_FLOAT;
}

/* True if the provided token's type is a lifetime. */
inline bool is_lifetime(const Token& tk) {
	return tk == TokenType::LF;
} 

/* True is the provided token is an attribute. */
inline bool is_attr(const Token& tk) {
	return tk == TokenType::PUB		||
			tk == TokenType::PRIV	||
			tk == TokenType::MUT	||
			tk == TokenType::CONST;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////    Expects    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

inline Error* Parser::err_expected(const std::string& found, const std::string& expected, int code) { 
	return handler.make_error_higligted("unexpected " + found + "; expected " + expected, curr_tok.span(), code);
}

inline Error* Parser::expect_symbol(char sym) {
	if (curr_tok.type() == (int)sym) {
		bump();
		return nullptr;
	}
	std::string found = curr_tok.type() < 256 ?
		std::string{'\'', (char)curr_tok.type(), '\'' } :	// TRUE
		translate::tk_type(curr_tok);						// FALSE
	return err_expected(found, "a '" + std::string{sym} + "'");
}

inline Error* Parser::expect_primitive() {
	if (is_primitive(curr_tok))
	{
		bump();
		return nullptr;
	}
	return err_expected(translate::tk_type(curr_tok), "a primitive type");
}

inline Error* Parser::expect_ident() {
	if (curr_tok == TokenType::ID) {
		bump();
		return nullptr;
	}
	return err_expected(translate::tk_type(curr_tok), "an identifier");
}

inline Error* Parser::expect_lifetime() {
	if (is_lifetime(curr_tok)) {
		bump();
		return nullptr;
	}
	return err_expected(translate::tk_type(curr_tok), "a lifetime");
}

inline Error* Parser::expect_keyword(TokenType ty) {
	if (curr_tok == ty) {
		bump();
		return nullptr;
	}
	return err_expected(translate::tk_type(curr_tok), "the keyword '" + translate::tk_type(ty) + "'");
}

inline Error* Parser::expect_mod_or_package() {
	if (curr_tok == TokenType::MOD || curr_tok == TokenType::PACKAGE) {
		bump();
		return nullptr;
	}
	return err_expected(translate::tk_type(curr_tok), "either 'mod' or 'package'");
}

inline Error* Parser::expect_block_item_decl() {
	if (curr_tok == TokenType::FUN ||
		curr_tok == TokenType::STRUCT ||
		curr_tok == TokenType::ENUM ||
		curr_tok == TokenType::UNION ||
		curr_tok == TokenType::TRAIT ||
		curr_tok == TokenType::IMPL)
	{
		bump();
		return nullptr;
	}
	return err_expected(translate::tk_type(curr_tok), "one of 'fun', 'struct', 'enum', 'union', 'trait' or 'impl'");
}

inline void Parser::bug(const std::string& msg) {
	handler.make_bug(msg).emit();
}

inline void Parser::unimpl(const std::string& msg) {
	handler.make_bug(msg + " not implemented yet").emit();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////      Helper Methods      /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

template<size_t N>
void Parser::recover_to(const std::array<int, N>& to) {
	while (curr_tok != TokenType::END)
	{
		for (auto c : to)
			if (curr_tok.type() == c)
				return;
		bump();
	}
}

/* Can interfere with normal token tree parsing,
 * so && is built on demand. */
inline void Parser::AND() {
	expect_symbol('&');
	expect_symbol('&');
}
/* Can interfere with normal token tree parsing,
 * so >> is built on demand. */ 
inline void Parser::SHR() {
	expect_symbol('>');
	expect_symbol('>');
}

// attributes: PUB | PRIV | MUT | CONST
Attributes Parser::attributes() {
	Attributes attributes;
	while (is_attr(curr_tok)) {
		attributes.push_back({ (TokenType)curr_tok.type(), curr_tok.span() });
		bump();
	}
	return attributes;
}

// SubPath(TokenType, Span)
// path : SubPath (',' SubPath)*
Path Parser::path() {
	trace("path");

	Path path = { SubPath{ curr_tok.raw(), curr_tok.span() } };
	ident();

	while (curr_tok == TokenType::SCOPE) {
		bump();
		path.push_back(SubPath{ curr_tok.raw(), curr_tok.span() });
		ident();
	}

	end_trace();
	return path;
}

// ident : ID
Error* Parser::ident() {
	trace("ident: " + std::string(curr_tok.raw()));
	end_trace();
	return expect_ident();;
}
// lifetime : LF
Error* Parser::lifetime() {
	trace("lifetime: " + std::string(curr_tok.raw()));
	end_trace();
	return expect_lifetime();
}

// generic_params : '<' type_or_lt (',' type_or_lt)* '>'
void Parser::generic_params() {
	trace("generic_params");
	expect_symbol('<');

	if(curr_tok.type() != '>') {
		type_or_lt();
		while (curr_tok.type() == ',') {
			bump();
			type_or_lt();
		}
	}
	
	expect_symbol('>');
	end_trace();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////    Parse    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// file : module_decl item*
Node* Parser::parse() {
	trace("parse");

	// TODO: Create appropriate root node type
	Node* ast = nullptr;

	// Every file begins with a declaration of the module the file is part of
	module_decl();

	// As long as the end of the file has not been reached,
	// expect to find items
	while (curr_tok != TokenType::END) {
		item();
	}

	end_trace();
	return ast;
}

// module_decl : MOD path ';'
void Parser::module_decl() {
	trace("module_decl");

	auto err = expect_keyword(TokenType::MOD);
	if (err) {
		recover_to(stmt_start);
		end_trace();
		return;
	}
	Path mod_path = path();
	if (expect_symbol(';')) {
		recover_to(stmt_start);
		end_trace();
		return;
	}

	end_trace();
}

// item : attributes? import_item
//      | attributes? stmt_item
void Parser::item() {
	trace("item");

	auto attributes = this->attributes();

	switch (curr_tok.type()) {
		case (int)TokenType::IMPORT:
			import_item();
			break;
		default:
			stmt_item();
	}

	end_trace();
}

// import_item : IMPORT MOD path
//             | IMPORT PACKAGE path
void Parser::import_item() {
	trace("import_item");
	expect_keyword(TokenType::IMPORT);

	switch (curr_tok.type()) {
		case (int)TokenType::MOD:
			this->path();
			expect_symbol(';');
			break;
		case (int)TokenType::PACKAGE:
			this->path();
			expect_symbol(';');
			break;
		default:
			expect_mod_or_package();
	}

	end_trace();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////    Stmt    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// stmt_item : item_static
//           | item_const
//           | item_type
//           | view_item
//           | block_item
void Parser::stmt_item() {
	trace("stmt_item");

	switch (curr_tok.type()) {
		case (int)TokenType::STATIC:
			item_static();
			break;
		case (int)TokenType::CONST:
			item_const();
			break;
		case (int)TokenType::TYPE:
			item_type();
			break;
		case (int)TokenType::USE:
			view_item();
			break;
		default:
			block_item();
	}

	end_trace();
}

// item_static : STATIC ident ':' type '=' expr ';'
void Parser::item_static() {
	trace("item_static");
	expect_keyword(TokenType::STATIC);

	//auto name = curr_tok.raw();
	ident();

	expect_symbol(':');
	type();

	expect_symbol('=');
	expr();

	expect_symbol(';');
	end_trace();
}

// item_const : CONST ident ':' type '=' expr ';'
void Parser::item_const() {
	trace("item_const");
	expect_keyword(TokenType::CONST);

	//auto name = curr_tok.raw();
	ident();

	expect_symbol(':');
	type();

	expect_symbol('=');
	expr();

	expect_symbol(';');
	end_trace();
}

// item_type : TYPE ident ''=' type_sum ';'
void Parser::item_type() {
	trace("item_type");
	expect_keyword(TokenType::TYPE);

	//auto name = curr_tok.raw();
	ident();

	expect_symbol('=');
	type();

	expect_symbol(';');
	end_trace();
}

// view_item : USE path ';'
void Parser::view_item() {
	trace("view_item");

	expect_keyword(TokenType::USE);
	auto path = this->path();

	expect_symbol(';');
	end_trace();
}

// block_item : item_fun
//            | item_struct
//            | item_enum
//            | item_union
//            | item_trait
//            | item_impl
void Parser::block_item() {
	trace("block_item");

	switch (curr_tok.type()) {
		case (int)TokenType::FUN:
			item_fun();
			break;
		case (int)TokenType::STRUCT:
			item_struct();
			break;
		case (int)TokenType::ENUM:
			item_enum();
			break;
		case (int)TokenType::UNION:
			item_union();
			break;
		case (int)TokenType::TRAIT:
			item_trait();
			break;
		case (int)TokenType::IMPL:
			item_impl();
			break;
		default:
			if (expect_block_item_decl()) {
				recover_to(stmt_start);
				end_trace();
				return;
			}
	}

	end_trace();
}

void Parser::item_fun() {
	trace("item_fun");
	expect_keyword(TokenType::FUN);

	//auto name = curr_tok.raw();
	ident();

	unimpl("item_fun");

	end_trace();
}

// item_struct : STRUCT ident generic_params? ';'
//             | STRUCT ident generic_params? struct_tuple_block ';'
//             | STRUCT ident generic_params? struct_decl_block
void Parser::item_struct() {
	trace("item_struct");
	expect_keyword(TokenType::STRUCT);

	//auto name = curr_tok.raw();
	ident();

	if (curr_tok.type() == '<')
		generic_params();

	if (curr_tok.type() == ';')
		bump();
	else if (curr_tok.type() == '(') {
		struct_tuple_block();
		expect_symbol(';');
	}
	else struct_decl_block();

	end_trace();
}

// struct_tuple_block : '(' struct_tuple_items? ')'
void Parser::struct_tuple_block() {
	trace("struct_tuple_block");
	expect_symbol('(');

	if (curr_tok.type() != ')')
		struct_tuple_items();

	expect_symbol(')');
	end_trace();
}

// struct_decl_items : attributes? type (',' attributes? type)*
//                   | attributes? type (',' attributes? type)* ','
void Parser::struct_tuple_items() {
	trace("struct_tuple_items");

	Attributes attr = attributes();
	type();

	while (curr_tok.type() == ',') {
		bump();
		if (curr_tok.type() == ')')
			break;
		attr = attributes();
		type();
	}

	end_trace();
}

// struct_decl_block : '{' struct_decl_items? '}'
void Parser::struct_decl_block() {
	trace("struct_decl_block");
	expect_symbol('{');

	if (curr_tok.type() != '}')
		struct_decl_items();

	expect_symbol('}');
	end_trace();
}

// struct_decl_items : struct_decl_item*
void Parser::struct_decl_items() {
	trace("struct_decl_items");

	while (curr_tok.type() != '}') {
		struct_decl_item();
	}

	end_trace();
}

// struct_decl_item : attributes? ident ':' type_with_lt ';' 
void Parser::struct_decl_item() {
	trace("struct_decl_item");

	Attributes attr = attributes();

	//auto name = curr_tok.raw();
	ident();

	expect_symbol(':');
	type_with_lt();

	expect_symbol(';');
	end_trace();
}

// item_enum : attributes? ENUM ident generic_params? ';'
//           | attributes? ENUM ident generic_params? enum_defs
void Parser::item_enum() {
	trace("item_enum");
	expect_keyword(TokenType::ENUM);

	//auto name = curr_tok.raw();
	ident();

	if (curr_tok.type() == '<')
		generic_params();

	if (curr_tok.type() == ';')
		bump();
	else if (curr_tok.type() == '{')
		enum_defs();

	end_trace();
}

// enum_defs : '{' '}'
//           | '{' enum_def (',' enum_def)*		'}'
//           | '{' enum_def (',' enum_def)* ',' '}'
void Parser::enum_defs() {
	trace("enum_defs");
	expect_symbol('{');

	if (curr_tok.type() != '}') {

		enum_def();

		// Keep looping as long as there are commas
		while (curr_tok.type() == ',') {
			bump();

			// Handle dangling comma at end of definitions
			if (curr_tok.type() == '}')
				break;

			enum_def();
		}
	}

	expect_symbol('}');
	end_trace();
}

void Parser::enum_def() {
	trace("enum_def");

	//auto name = curr_tok.raw();
	ident();

	if (curr_tok.type() == '=') {
		bump();
		expr();
	}
	else if (curr_tok.type() == '(') {
		bump();
		unimpl("enum unions");
	}

	end_trace();
}

void Parser::item_union() {
	trace("item_union");
	expect_keyword(TokenType::UNION);

	//auto name = curr_tok.raw();
	ident();

	if (curr_tok.type() == '<')
		generic_params();

	if (curr_tok.type() == ';')
		bump();
	else
		struct_decl_block();

	end_trace();
}

void Parser::item_trait() {
	trace("item_trait");
	expect_keyword(TokenType::TRAIT);

	//auto name = curr_tok.raw();
	ident();

	unimpl("item_trait");

	end_trace();
}

void Parser::item_impl() {
	trace("item_impl");
	expect_keyword(TokenType::IMPL);

	unimpl("item_impl");

	end_trace();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////    Expr    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void Parser::expr() {
	trace("expr");

	unimpl("expressions");

	end_trace();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////    Type    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/* True if the provided token could be a type.
 * ~could be~ because only the first token is checked.
 * i.e. '*'(ptr) would pass as a type token, even though it could
 * be followed by something else and be multiplication. */
inline bool is_type(const Token& tk) {
	return tk.type() == '&' || tk.type() == '*' || tk.type() == '[' || tk.type() == '(' ||
		tk == TokenType::ID || is_primitive(tk);
}

// type : '&' type
//      | '*' type
//      | '[' ']'
//      | '[' type ']'
//      | '[' type ';' expr ']'
//      | '(' ')'
//      | '(' type (',' type)* ')'
//      | ID
//      | primitive
void Parser::type() {
	trace("type");

	switch (curr_tok.type()) {
		// reference type
		case '&':
			bump();
			type();								// '&' type
			break;
	
		// pointer type
		case '*':
			bump();
			type();								// '*' type
			break;

		// array type
		case '[':
			bump();
			if (curr_tok.type() == ']') {
				bump();							// '[' ']'
			}
			else {
				type();							// '[' type ']'
				if (curr_tok.type() == ';') {
					bump();
					expr();						// '[' type ';' expr ']'
				}
				expect_symbol(']');
			}
			break;

		// tuple type
		case '(':
			bump();
			if (curr_tok.type() == ')') {
				bump();
				break;							// '(' ')'
			}
			else {
				type();
				while(curr_tok.type() == ',') {
					bump();
					type();						// '(' type (',' type)* ')'
				}
				expect_symbol(')');
			}
			break;

		// custom type
		case static_cast<int>(TokenType::ID):
			//std::string type_name = curr_tok.raw();
			ident();							// ident
			break;

		// primitive type
		default:
			if (is_primitive(curr_tok))
				primitive();
			else
				err_expected(translate::tk_type(curr_tok), "a type", 0);
	}
	
	end_trace();
}

// type_or_lt : type
//            | lifetime
void Parser::type_or_lt() {
	trace("type_or_lt");

	if (is_lifetime(curr_tok)) lifetime();
	else if (is_type(curr_tok)) type();
	else err_expected(translate::tk_type(curr_tok), "a type or lifetime", 0);

	end_trace();
}

// type_with_lt : lifetime? type
void Parser::type_with_lt() {
	trace("type_with_lt");

	if (is_lifetime(curr_tok))
		lifetime();
	type();

	end_trace();
}

// type_sum : type ('+' type)*
void Parser::type_sum() {
	trace("type_sum");

	while (curr_tok.type() == '+') {
		bump();
		type();
	}

	end_trace();
}

// primitive : THING | STR | CHAR
//           | INT | I64 | I32 | I16 | I8
//           | UINT | U64 | U32 | U16 | U8 
void Parser::primitive() {
	trace("primitive: " + std::string(curr_tok.raw()));

	expect_primitive();

	end_trace();
}