#include "parser.hpp"
#include "util/ranges.hpp"
#include "util/token_info.hpp"
#include "ast/ast.hpp"

#define DEFAULT_PARSE_END(x) { { end_trace(); return x; }; }
#define	EXPECT_OR_PASS(x) { if (expect_symbol(x, recover::stmt_start + Recovery{x})) if (curr_tok.type() == x) bump(); }

namespace recover {

	static const Recovery stmt_start = {
		(int)TokenType::IMPORT,
		(int)TokenType::EXPORT,
		(int)TokenType::USE,
		(int)TokenType::MOD,

		(int)TokenType::VAR,
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

		(int)TokenType::CONST,
		(int)TokenType::STATIC,

		(int)TokenType::PUB,
		(int)TokenType::PRIV,

		(int)TokenType::MUT,
	};

	static const Recovery semi = { ';' };
}

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

/* True if the provided token could be a type.
 * ~could be~ because only the first token is checked.
 * i.e. '*'(ptr) would pass as a type token, even though it could
 * be followed by something else and be multiplication. */
inline bool is_type(const Token& tk) {
	return tk.type() == '&' || tk.type() == '*' || tk.type() == '[' || tk.type() == '(' ||
		tk == TokenType::ID || is_primitive(tk);
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
			tk == TokenType::CONST	||
			tk == TokenType::STATIC;
}

Token Parser::split_multi_binop() {
	switch (curr_tok.type()) {
		[[fallthrough]] case (int)TokenType::EQEQ:
		[[fallthrough]] case (int)TokenType::NE:
		[[fallthrough]] case (int)TokenType::SUME:
		[[fallthrough]] case (int)TokenType::SUBE:
		[[fallthrough]] case (int)TokenType::MULE:
		[[fallthrough]] case (int)TokenType::DIVE:
		[[fallthrough]] case (int)TokenType::MOD:
		[[fallthrough]] case (int)TokenType::CARE:
		[[fallthrough]] case (int)TokenType::GE:
		[[fallthrough]] case (int)TokenType::LE:
		[[fallthrough]] case (int)TokenType::ORE:
		[[fallthrough]] case (int)TokenType::ANDE:
		[[fallthrough]] case (int)TokenType::OR:
		[[fallthrough]] case (int)TokenType::AND:
		[[fallthrough]] case (int)TokenType::SHL:
		[[fallthrough]] case (int)TokenType::SHR:
			curr_tok = Token(
				curr_tok.raw()[1],
				curr_tok.raw().substr(1, 2),
				Span(*curr_tok.span().tu, curr_tok.span().lo_bit + 1, curr_tok.span().lo_bit + 2));
			return Token(
				curr_tok.raw()[0],
				curr_tok.raw().substr(0, 1),
				Span(*curr_tok.span().tu, curr_tok.span().lo_bit, curr_tok.span().lo_bit + 1));
		default:
			return curr_tok;
	}
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
	else {
		auto tk = split_multi_binop();
		if (tk.type() == (int)sym) {
			bump();
			return nullptr;
		}
	}
	std::string found = curr_tok.type() < 256 ?
		std::string{'\'', (char)curr_tok.type(), '\'' } :	// TRUE
		translate::tk_type(curr_tok);						// FALSE
	return err_expected(found, "a '" + std::string{sym} + "'");
}

inline Error* Parser::expect_symbol(char sym, const Recovery& to) {
	auto err = expect_symbol(sym);
	if (err) recover_to(to);
	return err;
}

inline Error* Parser::expect_primitive() {
	if (is_primitive(curr_tok)) {
		bump();
		return nullptr;
	}
	return err_expected(translate::tk_type(curr_tok), "a primitive type");
}

inline Error* Parser::expect_primitive(const Recovery& to) {
	auto err = expect_primitive();
	if (err) recover_to(to);
	return err;
}

inline Error* Parser::expect_ident() {
	if (curr_tok == TokenType::ID) {
		bump();
		return nullptr;
	}
	return err_expected(translate::tk_type(curr_tok), "an identifier");
}

inline Error* Parser::expect_ident(const Recovery& to) {
	auto err = expect_ident();
	if (err) recover_to(to);
	return err;
}

inline Error* Parser::expect_lifetime() {
	if (is_lifetime(curr_tok)) {
		bump();
		return nullptr;
	}
	return err_expected(translate::tk_type(curr_tok), "a lifetime");
}

inline Error* Parser::expect_lifetime(const Recovery& to) {
	auto err = expect_lifetime();
	if (err) recover_to(to);
	return err;
}

inline Error* Parser::expect_keyword(TokenType ty) {
	if (curr_tok == ty) {
		bump();
		return nullptr;
	}
	else {
		auto tk = split_multi_binop();
		if (tk == ty) {
			bump();
			return nullptr;
		}
	}
	return err_expected(translate::tk_type(curr_tok), "the keyword '" + translate::tk_type(ty) + "'");
}

inline Error* Parser::expect_keyword(TokenType ty, const Recovery& to) {
	auto err = expect_keyword(ty);
	if (err) recover_to(to);
	return err;
}

inline Error* Parser::expect_mod_or_package() {
	if (curr_tok == TokenType::MOD || curr_tok == TokenType::PACKAGE) {
		bump();
		return nullptr;
	}
	return err_expected(translate::tk_type(curr_tok), "either 'mod' or 'package'");
}

inline Error* Parser::expect_mod_or_package(const Recovery& to) {
	auto err = expect_mod_or_package();
	if (err) recover_to(to);
	return err;
}

inline Error* Parser::expect_block_decl() {
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

inline Error* Parser::expect_block_decl(const Recovery& to) {
	auto err = expect_block_decl();
	if (err) recover_to(to);
	return err;
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

Recovery operator+(Recovery first, const Recovery& second) {
	first.reserve(first.size() + second.size());
	first.insert(
		std::end(first),
		std::begin(second),
		std::end(second)
	);
	return first;
}

void Parser::recover_to(const Recovery& to) {
	while (curr_tok != TokenType::END)
	{
		for (auto c : to)
			if (curr_tok.type() == c)
				return;
		bump();
	}
}

// attributes: PUB | PRIV | MUT | CONST
inline Attributes Parser::attributes() {
	Attributes attributes;
	while (is_attr(curr_tok)) {
		attributes.push_back({ (TokenType)curr_tok.type(), curr_tok.span() });
		bump();
	}
	return attributes;
}

// ident : ID
inline Error* Parser::ident() {
	trace("ident: " + std::string(curr_tok.raw()));
	end_trace();
	return expect_ident();
}
// ident : ID
inline Error* Parser::ident(const Recovery& to) {
	trace("ident: " + std::string(curr_tok.raw()));
	end_trace();
	return expect_ident(to);
}
// lifetime : LF
inline Error* Parser::lifetime() {
	trace("lifetime: " + std::string(curr_tok.raw()));
	end_trace();
	return expect_lifetime();
}
// lifetime : LF
inline Error* Parser::lifetime(const Recovery& to) {
	trace("lifetime: " + std::string(curr_tok.raw()));
	end_trace();
	return expect_lifetime(to);
}

// primitive : THING | STR | CHAR
//           | INT | I64 | I32 | I16 | I8
//           | UINT | U64 | U32 | U16 | U8
//           | FLOAT | F64 | F32
inline Error* Parser::primitive() {
	trace("primitive: " + std::string(curr_tok.raw()));
	end_trace();
	return expect_primitive();
}
// primitive : THING | STR | CHAR
//           | INT | I64 | I32 | I16 | I8
//           | UINT | U64 | U32 | U16 | U8
//           | FLOAT | F64 | F32
inline Error* Parser::primitive(const Recovery& to) {
	trace("primitive: " + std::string(curr_tok.raw()));
	end_trace();
	return expect_primitive(to);
}

// SubPath(TokenType, Span)
// path : SubPath (',' SubPath)*
Path Parser::path(const Recovery& to) {
	trace("path");

	bool bad_path = false;

	Path path = { SubPath{ curr_tok.raw(), curr_tok.span() } };

	if (ident(to + Recovery{(int)TokenType::SCOPE}))
		bad_path = true;

	while (curr_tok == TokenType::SCOPE) {
		bump();
		path.push_back(SubPath{ curr_tok.raw(), curr_tok.span() });

		if (ident(to + Recovery{(int)TokenType::SCOPE}))
			bad_path = true;
	}

	end_trace();
	return !bad_path ? path : Path();
}

// generic_params : '<' type_or_lt (',' type_or_lt)* '>'
void Parser::generic_params(const Recovery& recovery) {
	trace("generic_params");

	if (expect_symbol('<', recovery))  // should have been checked already
		DEFAULT_PARSE_END();\
	
	if (curr_tok.type() != '>') {

		if (auto err = type_or_lt(recovery + Recovery{',', '>'})) {
			if (curr_tok.type() != ',' && curr_tok.type() != '>') {
				err->cancel();
				handler.make_error_higligted("unterminated generic clause", curr_tok.span());
				DEFAULT_PARSE_END();
			}
		}

		while (curr_tok.type() == ',') {
			bump();

			if (curr_tok.type() == '>') {
				break;
			}

			if (auto err = type_or_lt(recovery + Recovery{',', '>'})) {
				if (curr_tok.type() != ',' && curr_tok.type() != '>') {
					err->cancel();
					handler.make_error_higligted("unterminated generic clause", curr_tok.span());
					DEFAULT_PARSE_END();
				}
			}
		}
	}
	
	expect_symbol('>', recovery);
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

	if (auto err = expect_keyword(TokenType::MOD, recover::stmt_start + recover::semi)) {
		err->add_help("Files are expected to start with a module declaration");
		if (curr_tok.type() == ';')
			bump();
		DEFAULT_PARSE_END();
	}

	auto mod_path = path(recover::stmt_start + recover::semi);
	if (mod_path.empty()) {
		if (curr_tok.type() == ';')
			bump();
		DEFAULT_PARSE_END();
	}

	EXPECT_OR_PASS(';');

	end_trace();
}

// item : attributes? import_item
//      | attributes? stmt_item
void Parser::item() {
	trace("item");

	auto attributes = this->attributes();

	if (curr_tok == TokenType::MOD) {
		item_module();
	}
	if (curr_tok == TokenType::IMPORT) {
		import_item();
	}
	else stmt_item(attributes);

	end_trace();
}

// item_module : MOD path '{' item* '}'
void Parser::item_module() {
	trace("item_module");

	if (expect_keyword(TokenType::MOD))
		bug("item_module not checked before invoking");

	auto mod_path = path(recover::stmt_start + Recovery{'{', ';'});

	if (auto err = expect_symbol('{', recover::stmt_start + Recovery{'{', ';'})) {
		if (curr_tok.type() == ';') {
			err->set_msg("Cannot declare file module in the middle of source");
			err->add_help("To declare a file module, place the declaration at the top of the file");
			err->add_help("If you wanted a sub-module, it should be contained in braces");
			bump();
		}
		DEFAULT_PARSE_END();
	}
	else bump();

	// Keep expecting items until a bracket is found
	// Fail if the file abruptly ends 
	while (curr_tok.type() != '}') {
		if (curr_tok == TokenType::END) {
			handler.emit_fatal_higligted("unexpected end of file", curr_tok.span());
			DEFAULT_PARSE_END();
		}
		item();
	}

	EXPECT_OR_PASS('}');

	end_trace();
}

// import_item : IMPORT MOD path ';'
//             | IMPORT PACKAGE path ';'
void Parser::import_item() {
	trace("import_item");

	if (expect_keyword(TokenType::IMPORT))
		bug("import_item not checked before invoking");

	if (curr_tok == TokenType::MOD) {
		bump();

		auto import_path = path(recover::stmt_start + recover::semi);
		if (import_path.empty()) {
			if (curr_tok.type() == ';')
				bump();
			DEFAULT_PARSE_END();
		}

		// TODO:  
	}
	else if (curr_tok == TokenType::PACKAGE) {
		bump();

		auto import_path = path(recover::stmt_start + recover::semi);
		if (import_path.empty()) {
			if (curr_tok.type() == ';')
				bump();
			DEFAULT_PARSE_END();
		}

		// TODO:  
	}
	else expect_mod_or_package();

	EXPECT_OR_PASS(';');

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
void Parser::stmt_item(Attributes& attr) {
	trace("stmt_item");

	bool is_const = attr.contains(TokenType::CONST);
	bool is_static = attr.contains(TokenType::STATIC);

	switch (curr_tok.type()) {
		case (int)TokenType::VAR:
			item_var(is_const, is_static);
			break;

		case (int)TokenType::TYPE:
			item_type();
			break;

		case (int)TokenType::USE:
			view_item();
			break;
			
		default: block_item();
	}

	end_trace();
}

// item_type : VAR ident (':' type_with_lf)? ('=' type_sum)? ';'
void Parser::item_var(bool is_const, bool is_static) {
	trace("item_var");

	if (expect_keyword(TokenType::VAR))
		bug("item_var not checked before invoking");

	//auto name = curr_tok.raw();
	if (ident(recover::stmt_start + recover::semi)) {
		if (curr_tok.type() == ';')
			bump();
		DEFAULT_PARSE_END();
	}

	bool has_type = true;
	Span type_pos = curr_tok.span();
	if (curr_tok.type() == ':') {
		bump();
		type_pos = curr_tok.span();
		if (type_with_lt(recover::stmt_start + Recovery{'='})) {
			if (curr_tok.type() != ';')
				DEFAULT_PARSE_END();
			has_type = false;
			bump();
		}
	}
	else has_type = false;

	bool has_init = true;
	if (curr_tok.type() == '=') {
		bump();
		expr();
	}
	else has_init = false;

	if (!has_type && !has_init) {
		if (is_const) {
			auto err = handler.make_error_higligted("const variable missing type", type_pos);
			err->add_note("a constant variable needs to have a type deducable at compile time");
		}
		if (is_static) {
			auto err = handler.make_error_higligted("static variable missing type", type_pos);
			err->add_note("a static variable needs to have a type deducable at compile time");
		}
	}

	EXPECT_OR_PASS(';');

	end_trace();
}

// item_type : TYPE ident ';'
//           | TYPE ident '=' type_sum ';'
void Parser::item_type() {
	trace("item_type");

	if (expect_keyword(TokenType::TYPE))
		bug("item_type not checked before invoking");

	//auto name = curr_tok.raw();
	if (ident(recover::stmt_start + recover::semi)) {
		if (curr_tok.type() == ';')
			bump();
		DEFAULT_PARSE_END();
	}

	if (curr_tok.type() == ';') {
		bump();
		DEFAULT_PARSE_END();
	}

	if (expect_symbol('=', recover::stmt_start + recover::semi))
		DEFAULT_PARSE_END();

	if (type(recover::stmt_start + recover::semi)) {
		if (curr_tok.type() == ';')
			bump();
	}

	EXPECT_OR_PASS(';');

	end_trace();
}

// view_item : USE path ';'
void Parser::view_item() {
	trace("view_item");

	if (expect_keyword(TokenType::USE))
		bug("view_item not checked before invoking");

	auto path = this->path(recover::stmt_start + recover::semi);
	if (path.empty()) {
		if (curr_tok.type() == ';')
			bump();
		DEFAULT_PARSE_END();
	}

	EXPECT_OR_PASS(';');
			
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
			if (expect_block_decl(recover::stmt_start))
				DEFAULT_PARSE_END();
	}

	end_trace();
}

void Parser::item_fun() {
	trace("item_fun");

	if (expect_keyword(TokenType::FUN))
		bug("item_fun not checked before invoking");

	//auto name = curr_tok.raw();
	ident();

	if (curr_tok.type() == '<')
		generic_params({'(', (int)TokenType::RARROW, '{'});

	// FIXME: 
	unimpl("item_fun");

	end_trace();
}

// item_struct : STRUCT ident generic_params? ';'
//             | STRUCT ident generic_params? struct_tuple_block ';'
//             | STRUCT ident generic_params? struct_decl_block
void Parser::item_struct() {
	trace("item_struct");

	if (expect_keyword(TokenType::STRUCT))
		bug("item_struct not checked before invoking");

	//auto name = curr_tok.raw();
	ident();

	if (curr_tok.type() == '<')
		generic_params({';', '(', '{'});

	if (curr_tok.type() == ';')
		bump();
	else if (curr_tok.type() == '(') {
		struct_tuple_block();
		expect_symbol(';', recover::stmt_start + recover::semi);
	}
	else struct_decl_block();

	end_trace();
}

// struct_tuple_block : '(' ')'
//                    | '(' struct_tuple_item (',' struct_tuple_item)*     ')'
//                    | '(' struct_tuple_item (',' struct_tuple_item)* ',' ')'
void Parser::struct_tuple_block() {
	trace("struct_tuple_block");

	if (expect_symbol('('))
		bug("struct_tuple_block not checked before invoking");

	while (curr_tok.type() != ')') {
		
		struct_tuple_item();

		while (curr_tok.type() == ',') {
			bump();

			if (curr_tok.type() == ')')
				break;

			struct_tuple_item();
		}
	}

	EXPECT_OR_PASS(')');
	end_trace();
}

// struct_tuple_item  : attributes? type
void Parser::struct_tuple_item() {
	trace("struct_tuple_item");

	Attributes attr = attributes();

	if (type_with_lt(recover::stmt_start + Recovery{',', ')'})) {
		if (curr_tok.type() != ',' && curr_tok.type() != ')') {
			end_trace();
			DEFAULT_PARSE_END();
		}
	}

	end_trace();
}

// struct_decl_block : '{' struct_decl_item* '}'
void Parser::struct_decl_block() {
	trace("struct_decl_block");

	if (expect_symbol('{'))
		bug("struct_decl_block not checked before invoking");

	while (curr_tok.type() != '}') {
		struct_decl_item();
	}

	EXPECT_OR_PASS('}');
	end_trace();
}

// struct_decl_item  : attributes? ident ':' type_with_lt ';' 
void Parser::struct_decl_item() {
	trace("struct_decl_item");

	auto attr = attributes();

	if (expect_ident(recover::stmt_start + Recovery{':', ';'})) {
		if (curr_tok.type() == ';') {
			bump();
			DEFAULT_PARSE_END();
		}
		if (curr_tok.type() != ':')
			DEFAULT_PARSE_END();
	}

	if (expect_symbol(':', recover::stmt_start + Recovery{':', ';'})) {
		if (curr_tok.type() == ';') {
			bump();
			DEFAULT_PARSE_END();
		}
		if (curr_tok.type() != ':')
			DEFAULT_PARSE_END();
	}

	if (type_with_lt(recover::stmt_start + recover::semi)) {
		if (curr_tok.type() == ';')
			bump();
		DEFAULT_PARSE_END();
	}

	EXPECT_OR_PASS(';');
	end_trace();
}

// item_enum : attributes? ENUM ident generic_params? ';'
//           | attributes? ENUM ident generic_params? enum_defs
void Parser::item_enum() {
	trace("item_enum");
	
	if (expect_keyword(TokenType::ENUM))
		bug("item_enum not checked before invoking");

	//auto name = curr_tok.raw();
	ident(recover::stmt_start + Recovery{'{', '<', ';'});

	if (curr_tok.type() == '<')
		generic_params({'{', ';'});

	if (curr_tok.type() == ';') {
		bump();
	}
	else enum_decl_block();

	end_trace();
}

// enum_defs : '{' '}'
//           | '{' enum_decl (',' enum_decl)*     '}'
//           | '{' enum_decl (',' enum_decl)* ',' '}'
void Parser::enum_decl_block() {
	trace("enum_defs");

	if (expect_symbol('{'))
		bug("enum_defs not checked before invoking");

	if (curr_tok.type() != '}') {

		enum_decl({',', '}'});

		// Keep looping as long as there are commas
		while (curr_tok.type() == ',') {
			bump();

			// Handle dangling comma at end of definitions
			if (curr_tok.type() == '}')
				break;

			enum_decl({',', '}'});
		}
	}

	EXPECT_OR_PASS('}');
	end_trace();
}

// enum_decl : ident
//           | ident '=' expr
//           | ident struct_tuple_block
void Parser::enum_decl(const Recovery& recovery) {
	trace("enum_def");

	//auto name = curr_tok.raw();
	if (ident(recovery))
		DEFAULT_PARSE_END();

	if (curr_tok.type() == '=') {
		bump();
		expr();
	}
	else if (curr_tok.type() == '(') {
		struct_tuple_block();
	}

	for (auto& delim : recovery) {
		if (curr_tok.type() == delim)
			DEFAULT_PARSE_END();
	}
	recover_to(recovery);

	end_trace();
}

// item_union : UNION ident generic_params? ';'
//            | UNION ident generic_params? struct_decl_block
void Parser::item_union() {
	trace("item_union");

	if (expect_keyword(TokenType::UNION))
		bug("item_union not checked before invoking");

	//auto name = curr_tok.raw();
	ident();

	if (curr_tok.type() == '<')
		generic_params({';', '{'});

	if (curr_tok.type() == ';')
		bump();
	else
		struct_decl_block();

	end_trace();
}

void Parser::item_trait() {
	trace("item_trait");

	if (expect_keyword(TokenType::TRAIT))
		bug("item_trait not checked before invoking");

	//auto name = curr_tok.raw();
	ident();

	// FIXME: 
	unimpl("item_trait");

	end_trace();
}

// item_impl : IMPL generic_params? ident generic_params?                           impl_block
//           | IMPL generic_params? ident generic_params? FOR ident generic_params? impl_block
void Parser::item_impl() {
	trace("item_impl");

	if (expect_keyword(TokenType::IMPL))
		bug("item_impl not checked before invoking");

	if (curr_tok.type() == '<')
		generic_params({'{', ';'});

	if (ident(recover::stmt_start + Recovery{'{', '<', ';'})) {
		if (curr_tok.type() != '{' && curr_tok.type() != '<' && curr_tok.type() != ';')
			DEFAULT_PARSE_END();
	}
	if (curr_tok.type() == '<')
		generic_params({'{', ';'});

	if (curr_tok == TokenType::FOR) {
		bump();

		if (ident(recover::stmt_start + Recovery{'{', '<', ';'})) {
			if (curr_tok.type() != '{' && curr_tok.type() != '<' && curr_tok.type() != ';')
				DEFAULT_PARSE_END();
		}
		if (curr_tok.type() == '<')
			generic_params({'{', ';'});
	}

	if (curr_tok.type() != '{') {
		expect_symbol('{', recover::stmt_start + Recovery{'{', ';'});
		if (curr_tok.type() != '{') {
			if (curr_tok.type() == ';')
				bump();
			DEFAULT_PARSE_END();
		}
	}

	impl_block();

	end_trace();
}

void Parser::impl_block() {
	trace("impl_block");

	if (expect_symbol('{'))
		bug("impl_block not checked before invoking");

	while (curr_tok.type() != '}') {
		auto attrib = attributes();

		// FIXME:  Add constructor support
		// FIXME:  'impl' should probably have it's own item list
		item();
	}

	EXPECT_OR_PASS('}');
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

// type : '&' type
//      | '*' type
//      | '[' ']'
//      | '[' type ']'
//      | '[' type ';' expr ']'
//      | '(' ')'
//      | '(' type (',' type)* ')'
//      | ID
//      | primitive
Error* Parser::type(const Recovery& recovery) {

	switch (curr_tok.type()) {
		// reference type
		case '&':
			bump();
			trace("type: ref");
			if (auto err = type(recovery)) {				// '&' type
				DEFAULT_PARSE_END(err);
			}
			break;
	
		// pointer type
		case '*':
			bump();
			trace("type: ptr");
			if (auto err = type(recovery)) {				// '*' type
				DEFAULT_PARSE_END(err);
			}
			break;

		// array type
		case '[':
			bump();
			trace("type: array");
			if (curr_tok.type() == ']') {
				bump();										// '[' ']'
			}
			else {
				Error* passed_on = nullptr;
				if (auto err = type(recover::stmt_start + Recovery{']', ';'})) {		// '[' type ']' | '[' type ';' expr ']'
					if (curr_tok.type() == ']') {
						bump();
						DEFAULT_PARSE_END(err);
					}
					else if (curr_tok.type() == ';') {
						bump();
						passed_on = err;
						// Doesn't fail
						// Attempts to contilue parsing type
					}
				}
				if (curr_tok.type() == ';') {
					bump();
					expr();									// '[' type ';' expr ']
				}
				if (auto err = expect_symbol(']', recover::stmt_start + Recovery{']', ';'})) {
					if (curr_tok.type() == ']') {
						bump();
						DEFAULT_PARSE_END(err);
					}
					else if (curr_tok.type() == ';') {
						bump();
						DEFAULT_PARSE_END(err);
					}
					else {
						DEFAULT_PARSE_END(err);
					}
				}
				if (passed_on) {
					DEFAULT_PARSE_END(passed_on);
				}
			}
			end_trace();
			break;

		// tuple type
		case '(':
			trace("type: tuple");
			bump();
			if (curr_tok.type() == ')') {						// '(' ')'
				bump();				
			}
			else {
				auto err = type({';', ']', ')', '}'});
				DEFAULT_PARSE_END(err);
				while(curr_tok.type() == ',') {
					bump();
					err = type(recovery);						// '(' type (',' type)* ')'
					DEFAULT_PARSE_END(err);
				}
				if (auto err = expect_symbol(')')) {
					recover_to({')'});
					DEFAULT_PARSE_END(err);
				}
			}
			break;

		// custom type
		case (int)TokenType::ID:
			trace("type: ident");
			//auto type_name = curr_tok.raw();
			if (auto err = ident()) {							// ident
				recover_to(recovery);
				DEFAULT_PARSE_END(err);
			}
			break;

		// primitive type
		default:
			if (is_primitive(curr_tok)) {
				trace("type: primitive");
				primitive();
			}
			else {
				auto err = err_expected(translate::tk_type(curr_tok), "a type");
				recover_to(recovery);
				return err;
 			}
	}

	end_trace();
	return nullptr;
}

// type_or_lt : type
//            | lifetime
Error* Parser::type_or_lt(const Recovery& recovery) {
	trace("type_or_lt");

	if (is_lifetime(curr_tok)) lifetime();
	else if (is_type(curr_tok)) {
		auto err = type(recovery); 
		DEFAULT_PARSE_END(err);
	}
	else {
		auto err = err_expected(translate::tk_type(curr_tok), "a type or lifetime", 0);
		recover_to(recovery);
		DEFAULT_PARSE_END(err);
	}

	end_trace();
	return nullptr;
}

// type_with_lt : lifetime? type
Error* Parser::type_with_lt(const Recovery& recovery) {
	trace("type_with_lt");

	if (is_lifetime(curr_tok))
		lifetime();
	if (auto err = type(recovery))
		DEFAULT_PARSE_END(err);

	end_trace();
	return nullptr;
}

// type_sum : type ('+' type)*
Error* Parser::type_sum(const Recovery& recovery) {
	trace("type_sum");

	if (auto err = type(recovery))
		DEFAULT_PARSE_END(err);

	while (curr_tok.type() == '+') {
		bump();
		if (auto err = type(recovery + Recovery{'+'}))
			DEFAULT_PARSE_END(err);
	}

	end_trace();
	return nullptr;
}