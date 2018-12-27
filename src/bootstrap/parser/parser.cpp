#include "parser.hpp"
#include "util/ranges.hpp"
#include "util/token_info.hpp"
#include "ast/ast.hpp"
#include <algorithm>

#define DEFAULT_PARSE_END(x) { { end_trace(); return x; }; }
#define	EXPECT_OR_PASS(x) { if (expect_symbol(x, recover::decl_start + Recovery{x})) if (curr_tok.type() == x) bump(); }

/* Contains recovery point presets */
namespace recover {

	static const Recovery decl_start = {
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

	static const Recovery type_start = {
		'*',
		'&',
		'[',
		'(',
		(int)TokenType::ID,

		(int)TokenType::THING,
		(int)TokenType::STR,
		(int)TokenType::CHAR,
		(int)TokenType::INT,
		(int)TokenType::I64,
		(int)TokenType::I32,
		(int)TokenType::I16,
		(int)TokenType::I8,
		(int)TokenType::UINT,
		(int)TokenType::U64,
		(int)TokenType::U32,
		(int)TokenType::U16,
		(int)TokenType::U8,
		(int)TokenType::FLOAT,
		(int)TokenType::F32,
		(int)TokenType::F64
	};

	static const Recovery expr_end = {
		',',
		'.',
		';',
		':',
		')',
		']',
		'}'
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
/////////////////////////////////      Operators / OPInfo      ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/* A single operator-opinfo pair.
 * Key - operator (char)
 * Value - OPInfo. */
struct OPInfoPair {
	const char key;
	OPInfo value;
};
/* Maps operators to their OPInfo. */
constexpr static OPInfoPair opinfo_map[] {
	{'+',    {1, OPInfo::LEFT}},
	{'-',    {1, OPInfo::LEFT}},
	{'*',    {2, OPInfo::LEFT}},
	{'/',    {2, OPInfo::LEFT}},
	{'^',    {3, OPInfo::RIGHT}}
};
/* Attempts to find the given operator in the OPInfo map.
 * If no operations match, a nullptr is returned. */
constexpr const OPInfoPair* op_find(char key) {
	for (unsigned long i = 0; i < sizeof(opinfo_map)/sizeof(*opinfo_map); i++)
		if (opinfo_map[i].key == key)
			return &opinfo_map[i];
	return nullptr;
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

inline bool is_binop(const Token& tk) {
	return tk.type() == '+' || tk.type() == '-' || tk.type() == '*' || tk.type() == '/' || tk.type() == '^';
}

inline bool is_unaryop(const Token& tk) {
	return tk.type() == '-' || tk.type() == '!' || tk.type() == '&' || tk.type() == '*';
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

inline Recovery operator+(Recovery first, const Recovery& second) {
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

// primitive : THING | STR | CHAR
//           | INT | I64 | I32 | I16 | I8
//           | UINT | U64 | U32 | U16 | U8
//           | FLOAT | F64 | F32
inline Error* Parser::primitive() {
	trace("primitive: " + std::string(curr_tok.raw()));
	if (!is_primitive(curr_tok))
		DEFAULT_PARSE_END(err_expected(translate::tk_type(curr_tok), "a primitive type"));
	bump();
	DEFAULT_PARSE_END(nullptr);
}
// primitive : THING | STR | CHAR
//           | INT | I64 | I32 | I16 | I8
//           | UINT | U64 | U32 | U16 | U8
//           | FLOAT | F64 | F32
inline Error* Parser::primitive(const Recovery& to) {
	if (auto err = primitive()) {
		recover_to(to);
		return err;
	}
	return nullptr;
}

// ident : ID
inline Error* Parser::ident() {
	trace("ident: " + std::string(curr_tok.raw()));
	if (curr_tok != TokenType::ID)
		DEFAULT_PARSE_END(err_expected(translate::tk_type(curr_tok), "an identifier"));
	bump();
	DEFAULT_PARSE_END(nullptr);
}
// ident : ID
inline Error* Parser::ident(const Recovery& to) {
	if (auto err = ident()) {
		recover_to(to);
		return err;
	}
	return nullptr;
}

// lifetime : LF
inline Error* Parser::lifetime() {
	trace("lifetime: " + std::string(curr_tok.raw()));
	if (!is_lifetime(curr_tok))
		DEFAULT_PARSE_END(err_expected(translate::tk_type(curr_tok), "a lifetime"));
	bump();
	DEFAULT_PARSE_END(nullptr);
}
// lifetime : LF
inline Error* Parser::lifetime(const Recovery& to) {
	if (auto err = lifetime()) {
		recover_to(to);
		return err;
	}
	return nullptr;
}

// literal : LIT_STRING
//         | LIT_CHAR
//         | LIT_INTEGER
//         | LIT_FLOAT
inline Error* Parser::literal() {
	switch (curr_tok.type()) {
		case (int)TokenType::LIT_STRING:
			trace("literal: string: " + translate::tk_info(curr_tok));
			bump();
			break;
		case (int)TokenType::LIT_CHAR:
			trace("literal: char: " + translate::tk_info(curr_tok));
			bump();
			break;
		case (int)TokenType::LIT_INTEGER:
			trace("literal: int: " + translate::tk_info(curr_tok));
			bump();
			break;
		case (int)TokenType::LIT_FLOAT:
			trace("literal: float: " + translate::tk_info(curr_tok));
			bump();
			break;
		default:
			trace("literal: invalid: " + std::string(curr_tok.raw()));
			DEFAULT_PARSE_END(err_expected(translate::tk_type(curr_tok), "a literal"));
	}
	DEFAULT_PARSE_END(nullptr);
}
// literal : LIT_STRING
//         | LIT_CHAR
//         | LIT_INTEGER
//         | LIT_FLOAT
inline Error* Parser::literal(const Recovery& to) {
	if (auto err = literal()) {
		recover_to(to);
		return err;
	}
	return nullptr;
}

// binop : '+' | '-' | '*' | '/' | '^'
inline Error* Parser::binop() {
	trace("binop: " + std::string(curr_tok.raw()));
	if (!is_binop(curr_tok))
		DEFAULT_PARSE_END(err_expected(translate::tk_type(curr_tok), "a binary operator"));
	bump();
	DEFAULT_PARSE_END(nullptr);
}
// binop : '+' | '-' | '*' | '/' | '^'
inline Error* Parser::binop(const Recovery& to) {
	if (auto err = binop()) {
		recover_to(to);
		return err;
	}
	return nullptr;
}

// unaryop : '-' | '!' | '&' | '*'
inline Error* Parser::unaryop() {
	trace("unaryop: " + std::string(curr_tok.raw()));
	if (!is_unaryop(curr_tok))
		DEFAULT_PARSE_END(err_expected(translate::tk_type(curr_tok), "a unary operator"));
	bump();
	DEFAULT_PARSE_END(nullptr);
}
// unaryop : '-' | '!' | '&' | '*'
inline Error* Parser::unaryop(const Recovery& to) {
	if (auto err = unaryop()) {
		recover_to(to);
		return err;
	}
	return nullptr;
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

	if (expect_symbol('<'))
		bug("generic_params not checked before invoking");
	
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

// param_list : '(' ')'
//            | '(' param (',' param)* ')'
void Parser::param_list(bool is_method, const Recovery& recovery) {
	trace("param_list");

	if (expect_symbol('('))
		bug("param_list not checked before invoking");

	if (curr_tok.type() != ')') {

		if (is_method && is_type(curr_tok) && curr_tok != TokenType::ID) {
			param_self(recovery + Recovery{','});
		}
		else param(recovery + Recovery{',', ')'});

		while (curr_tok.type() == ',') {
			bump();

			param(recovery + Recovery{',', ')'});

			if (curr_tok.type() == ')')
				break;
		} 
	}
	
	if (expect_symbol(')', recovery + Recovery{')'})) {
		if (curr_tok.type() == ')') {
			bump();
		}
		DEFAULT_PARSE_END();
	}

	end_trace();
}

// param : ident ':' type
Error* Parser::param(const Recovery& recovery) {
	trace("param");

	if (auto err = ident(recovery))
		DEFAULT_PARSE_END(err);

	if (auto err = expect_symbol(':', recovery + recover::type_start)) {
		if (!is_type(curr_tok))
			DEFAULT_PARSE_END(err);
	}

	if (auto err = type(recovery))
		DEFAULT_PARSE_END(err);

	end_trace();
	return nullptr;
}

// param_self : (& MUT?)? SELF
Error* Parser::param_self(const Recovery& recovery) {
	trace("param_self");

	if (curr_tok.type() == '&' || curr_tok.type() == '*') {
		bump();
		if (curr_tok == TokenType::MUT)
			bump();
	}
	if (curr_tok == TokenType::SELF)
		bump();
	else {
		auto err = err_expected(translate::tk_type(curr_tok), "a named parameter");
		recover_to(recovery);
		return(err);
	}
	return nullptr;
}

// arg_list : '(' ')'
//          | '(' arg (',' arg)* ')'
void Parser::arg_list(const Recovery& recovery) {
	trace("arg_list");

	if (expect_symbol('('))
		bug("arg_list not checked before invoking");

	if (curr_tok.type() != ')') {

		arg(recovery + Recovery{',', ')'});

		while (curr_tok.type() == ',') {
			bump();

			arg(recovery + Recovery{',', ')'});

			if (curr_tok.type() == ')')
				break;
		} 
	}
	
	if (expect_symbol(')', recovery + Recovery{')'})) {
		if (curr_tok.type() == ')') {
			bump();
		}
		DEFAULT_PARSE_END();
	}

	end_trace();
}

// arg : expr
Error* Parser::arg(const Recovery& recovery) {
	trace("arg");

	if (auto err = expr(1)) {
		recover_to(recovery);
		DEFAULT_PARSE_END(err);
	}

	end_trace();
	return nullptr;
}

// return_type: type
Error* Parser::return_type(const Recovery& recovery) {
	trace("return_type");
	auto err = type(recovery);
	end_trace();
	return err;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////    Parse    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// file : module_decl decl*
Node* Parser::parse() {
	trace("parse");

	// TODO: Create appropriate root node type
	Node* ast = nullptr;

	// Every file begins with a declaration of the module the file is part of
	decl_file_module();

	// As long as the end of the file has not been reached,
	// expect to find decls
	while (curr_tok != TokenType::END) {
		decl();
	}

	end_trace();
	return ast;
}

// decl_file_module : MOD path ';'
void Parser::decl_file_module() {
	trace("decl_file_module");

	if (auto err = expect_keyword(TokenType::MOD, recover::decl_start + recover::semi)) {
		err->add_help("Files are expected to start with a module declaration");
		if (curr_tok.type() == ';')
			bump();
		DEFAULT_PARSE_END();
	}

	auto mod_path = path(recover::decl_start + recover::semi);
	if (mod_path.empty()) {
		if (curr_tok.type() == ';')
			bump();
		DEFAULT_PARSE_END();
	}

	EXPECT_OR_PASS(';');

	end_trace();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////    Decl    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// decl : attributes? decl_sub_module
//      | attributes? decl_import_item
//		| attributes? decl_var
//      | attributes? decl_type
//      | attributes? decl_use
//      | attributes? block_decl
void Parser::decl() {
	trace("decl");

	auto attr = this->attributes();
	bool is_const = attr.contains(TokenType::CONST);
	bool is_static = attr.contains(TokenType::STATIC);

	switch (curr_tok.type()) 
	{
		case (int)TokenType::MOD:
			decl_sub_module();
			break;

		case (int)TokenType::IMPORT:
			decl_import_item();
			break;

		case (int)TokenType::VAR:
			decl_var(is_const, is_static);
			break;

		case (int)TokenType::TYPE:
			decl_type();
			break;

		case (int)TokenType::USE:
			decl_use();
			break;
			
		default: block_decl();
	}

	end_trace();
}

// decl_sub_module : MOD path '{' decl* '}'
void Parser::decl_sub_module() {
	trace("decl_sub_module");

	if (expect_keyword(TokenType::MOD))
		bug("decl_sub_module not checked before invoking");

	auto mod_path = path(recover::decl_start + Recovery{'{', ';'});

	if (auto err = expect_symbol('{', recover::decl_start + Recovery{'{', ';'})) {
		if (curr_tok.type() == ';') {
			err->set_msg("Cannot declare file module in the middle of source");
			err->add_help("To declare a file module, place the declaration at the top of the file");
			err->add_help("If you wanted a sub-module, it should be contained in braces");
			bump();
		}
		DEFAULT_PARSE_END();
	}
	else bump();

	// Keep expecting decls until a bracket is found
	// Fail if the file abruptly ends 
	while (curr_tok.type() != '}') {
		if (curr_tok == TokenType::END) {
			handler.emit_fatal_higligted("unexpected end of file", curr_tok.span());
			DEFAULT_PARSE_END();
		}
		decl();
	}

	EXPECT_OR_PASS('}');

	end_trace();
}

// decl_import_item : IMPORT MOD path ';'
//                  | IMPORT PACKAGE path ';'
void Parser::decl_import_item() {
	trace("decl_import_item");

	if (expect_keyword(TokenType::IMPORT))
		bug("decl_import_item not checked before invoking");

	if (curr_tok == TokenType::MOD) {
		bump();

		auto import_path = path(recover::decl_start + recover::semi);
		if (import_path.empty()) {
			if (curr_tok.type() == ';')
				bump();
			DEFAULT_PARSE_END();
		}

		// TODO:  
	}
	else if (curr_tok == TokenType::PACKAGE) {
		bump();

		auto import_path = path(recover::decl_start + recover::semi);
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

// decl_var : VAR ident (':' type_with_lf)? ('=' type_sum)? ';'
void Parser::decl_var(bool is_const, bool is_static) {
	trace("decl_var");

	if (expect_keyword(TokenType::VAR))
		bug("decl_var not checked before invoking");

	//auto name = curr_tok.raw();
	if (ident(recover::decl_start + recover::semi)) {
		if (curr_tok.type() == ';')
			bump();
		DEFAULT_PARSE_END();
	}

	bool has_type = true;
	Span type_pos = curr_tok.span();
	if (curr_tok.type() == ':') {
		bump();
		type_pos = curr_tok.span();
		if (type_with_lt(recover::decl_start + Recovery{'='})) {
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
		if (expr(1)) {
			recover_to(recover::decl_start + recover::semi);
			if (curr_tok.type() == ';')
				bump();
			DEFAULT_PARSE_END();
		}
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

// decl_type : TYPE ident ';'
//           | TYPE ident '=' type_sum ';'
void Parser::decl_type() {
	trace("decl_type");

	if (expect_keyword(TokenType::TYPE))
		bug("decl_type not checked before invoking");

	//auto name = curr_tok.raw();
	if (ident(recover::decl_start + recover::semi)) {
		if (curr_tok.type() == ';')
			bump();
		DEFAULT_PARSE_END();
	}

	if (curr_tok.type() == ';') {
		bump();
		DEFAULT_PARSE_END();
	}

	if (expect_symbol('=', recover::decl_start + recover::semi))
		DEFAULT_PARSE_END();

	if (type(recover::decl_start + recover::semi)) {
		if (curr_tok.type() == ';')
			bump();
	}

	EXPECT_OR_PASS(';');

	end_trace();
}

// decl_use : USE path ';'
void Parser::decl_use() {
	trace("decl_use");

	if (expect_keyword(TokenType::USE))
		bug("decl_use not checked before invoking");

	auto path = this->path(recover::decl_start + recover::semi);
	if (path.empty()) {
		if (curr_tok.type() == ';')
			bump();
		DEFAULT_PARSE_END();
	}

	EXPECT_OR_PASS(';');
			
	end_trace();
}

// block_decl : decl_fun
//            | decl_struct
//            | decl_enum
//            | decl_union
//            | decl_trait
//            | decl_impl
void Parser::block_decl() {
	trace("block_decl");

	switch (curr_tok.type()) {
		case (int)TokenType::FUN:
			decl_fun(false);
			break;
		case (int)TokenType::STRUCT:
			decl_struct();
			break;
		case (int)TokenType::ENUM:
 			decl_enum();
			break;
		case (int)TokenType::UNION:
			decl_union();
			break;
		case (int)TokenType::TRAIT:
			decl_trait();
			break;
		case (int)TokenType::IMPL:
			decl_impl();
			break;
		default:
			if (expect_block_decl(recover::decl_start))
				DEFAULT_PARSE_END();
	}

	end_trace();
}

// decl_fun : FUN ident generic_params? param_list (RARROW return_type)? ';'
//          | FUN ident generic_params? param_list (RARROW return_type)? fun_block
void Parser::decl_fun(bool is_method) {
	trace("decl_fun");

	if (expect_keyword(TokenType::FUN))
		bug("decl_fun not checked before invoking");

	//auto name = curr_tok.raw();
	ident(recover::decl_start + Recovery{'<', '(', (int)TokenType::RARROW, ';', '{'});

	if (curr_tok.type() == '<')
		generic_params(recover::decl_start + Recovery{'(', (int)TokenType::RARROW, '{'});

	if (curr_tok.type() == '(')
		param_list(is_method, {(int)TokenType::RARROW, '{'});

	if (curr_tok == TokenType::RARROW) {
		bump();
		if (return_type(recover::decl_start + Recovery{'{', ';'})) {
			if (curr_tok.type() != '{' && curr_tok.type() != ';')
				DEFAULT_PARSE_END();
		}
	}

	if (curr_tok.type() == ';') {
		bump();
		DEFAULT_PARSE_END();
	}
	else if (curr_tok.type() != '{') {
		expect_symbol('{', recover::decl_start + Recovery{'{', ';'});
		if (curr_tok.type() != '{') {
			if (curr_tok.type() == ';')
				bump();
			DEFAULT_PARSE_END();
		}
	}

	fun_block();

	end_trace();
}

void Parser::fun_block() {
	trace("fun_block");

	if (expect_symbol('{'))
		bug("fun_block not checked before invoking");

	// FIXME: 
	unimpl("function block");

	while (curr_tok.type() != '}') {
		bump();
	}

	end_trace();
}

// decl_struct : STRUCT ident generic_params? ';'
//             | STRUCT ident generic_params? struct_tuple_block ';'
//             | STRUCT ident generic_params? struct_named_block
void Parser::decl_struct() {
	trace("decl_struct");

	if (expect_keyword(TokenType::STRUCT))
		bug("decl_struct not checked before invoking");

	//auto name = curr_tok.raw();
	ident(recover::decl_start + Recovery{'<', '(', '{', ';'});

	if (curr_tok.type() == '<')
		generic_params({';', '(', '{'});

	if (curr_tok.type() == ';') {
		bump();
		DEFAULT_PARSE_END();
	}
	else if (curr_tok.type() == '(') {
		struct_tuple_block();
		if (expect_symbol(';', recover::decl_start + recover::semi)) {
			if (curr_tok.type() == ';') 
				bump();
		}
		DEFAULT_PARSE_END();
	}
	else if (curr_tok.type() != '{') {
		expect_symbol('{', recover::decl_start + Recovery{'{', ';'});
		if (curr_tok.type() != '{') {
			if (curr_tok.type() == ';')
				bump();
			DEFAULT_PARSE_END();
		}
	}

	struct_named_block();

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

	if (type_with_lt(recover::decl_start + Recovery{',', ')'})) {
		if (curr_tok.type() != ',' && curr_tok.type() != ')') {
			end_trace();
			DEFAULT_PARSE_END();
		}
	}

	end_trace();
}

// struct_named_block : '{' struct_named_item* '}'
void Parser::struct_named_block() {
	trace("struct_named_block");

	if (expect_symbol('{'))
		bug("struct_named_block not checked before invoking");

	while (curr_tok.type() != '}') {
		struct_named_item();
	}

	EXPECT_OR_PASS('}');
	end_trace();
}

// struct_named_item  : attributes? ident ':' type_with_lt ';' 
void Parser::struct_named_item() {
	trace("struct_named_item");

	auto attr = attributes();

	if (ident(recover::decl_start + Recovery{':', ';'})) {
		if (curr_tok.type() == ';') {
			bump();
			DEFAULT_PARSE_END();
		}
		if (curr_tok.type() != ':')
			DEFAULT_PARSE_END();
	}

	if (expect_symbol(':', recover::decl_start + Recovery{':', ';'})) {
		if (curr_tok.type() == ';') {
			bump();
			DEFAULT_PARSE_END();
		}
		if (curr_tok.type() != ':')
			DEFAULT_PARSE_END();
	}

	if (type_with_lt(recover::decl_start + recover::semi)) {
		if (curr_tok.type() == ';')
			bump();
		DEFAULT_PARSE_END();
	}

	EXPECT_OR_PASS(';');
	end_trace();
}

// decl_enum : attributes? ENUM ident generic_params? ';'
//           | attributes? ENUM ident generic_params? enum_block
void Parser::decl_enum() {
	trace("decl_enum");
	
	if (expect_keyword(TokenType::ENUM))
		bug("decl_enum not checked before invoking");

	//auto name = curr_tok.raw();
	ident(recover::decl_start + Recovery{'<', '{', ';'});

	if (curr_tok.type() == '<')
		generic_params({'{', ';'});

	if (curr_tok.type() == ';') {
		bump();
		DEFAULT_PARSE_END();
	}
	else if (curr_tok.type() != '{') {
		expect_symbol('{', recover::decl_start + Recovery{'{', ';'});
		if (curr_tok.type() != '{') {
			if (curr_tok.type() == ';')
				bump();
			DEFAULT_PARSE_END();
		}
	}

	enum_block();

	end_trace();
}

// enum_block : '{' '}'
//            | '{' enum_item (',' enum_item)*     '}'
//            | '{' enum_item (',' enum_item)* ',' '}'
void Parser::enum_block() {
	trace("enum_block");

	if (expect_symbol('{'))
		bug("enum_block not checked before invoking");

	if (curr_tok.type() != '}') {

		if (enum_item({',', '}'})) {
			if (curr_tok.type() != ',' && curr_tok.type() != '}')
				DEFAULT_PARSE_END();
		}

		// Keep looping as long as there are commas
		while (curr_tok.type() != '}') {
			if (expect_symbol(',', {(int)TokenType::ID, '}'})) {

			}

			// Handle dangling comma at end of definitions
			if (curr_tok.type() == '}')
				break;

			if (enum_item({',', '}'})) {
				if (curr_tok.type() != ',' && curr_tok.type() != '}')
					DEFAULT_PARSE_END();
			}
		}
	}

	EXPECT_OR_PASS('}');
	end_trace();
}

// enum_item : ident
//           | ident '=' expr
//           | ident struct_tuple_block
Error* Parser::enum_item(const Recovery& recovery) {
	trace("enum_item");

	//auto name = curr_tok.raw();
	if (auto err = ident(recovery + Recovery{'=', '('})) {
		if (curr_tok.type() != '=' && curr_tok.type() != '(')
			DEFAULT_PARSE_END(err);
	}

	if (curr_tok.type() == '=') {
		bump();
		if (auto err = expr(1)) {
			recover_to(recovery);
			DEFAULT_PARSE_END(err);
		}
	}
	else if (curr_tok.type() == '(') {
		struct_tuple_block();
	}

	end_trace();
	return nullptr;
}

// decl_union : UNION ident generic_params? ';'
//            | UNION ident generic_params? struct_named_block
void Parser::decl_union() {
	trace("decl_union");

	if (expect_keyword(TokenType::UNION))
		bug("decl_union not checked before invoking");

	//auto name = curr_tok.raw();
	ident(recover::decl_start + Recovery{'{', '<', ';'});

	if (curr_tok.type() == '<')
		generic_params({';', '{'});

	if (curr_tok.type() == ';') {
		bump();
		DEFAULT_PARSE_END();
	}
	else if (curr_tok.type() != '{') {
		expect_symbol('{', recover::decl_start + Recovery{'{', ';'});
		if (curr_tok.type() != '{') {
			if (curr_tok.type() == ';')
				bump();
			DEFAULT_PARSE_END();
		}
	}

	struct_named_block();

	end_trace();
}

// decl_trait : TRAIT ident
void Parser::decl_trait() {
	trace("decl_trait");

	if (expect_keyword(TokenType::TRAIT))
		bug("decl_trait not checked before invoking");

	//auto name = curr_tok.raw();
	ident(recover::decl_start + Recovery{'<', '{', ';'});

	if (curr_tok.type() == '<')
		generic_params({'{', ';'});

	if (curr_tok.type() == ';') {
		auto err = expect_symbol('{');
		err->add_note("traits should be defined at declaration");
		bump();
		DEFAULT_PARSE_END();
	}
	else if (curr_tok.type() != '{') {
		expect_symbol('{', recover::decl_start + Recovery{'{'});
		if (curr_tok.type() != '{')
			DEFAULT_PARSE_END();
	}

	trait_block();

	end_trace();
}

// trait_block : '{' (decl_type | decl_fun)* '}'
void Parser::trait_block() {
	trace("trait_block");

	if (expect_symbol('{'))
		bug("trait_block not checked before invoking");

	while (curr_tok.type() != '}') {

		auto attr = attributes();

		switch (curr_tok.type()) {
			case (int)TokenType::TYPE:
				decl_type();
				break;
			case (int)TokenType::FUN:
				decl_fun(true);
				break;
			default:
				err_expected(translate::tk_type(curr_tok), "one of 'fun' or 'type'");
				recover_to(recover::decl_start + Recovery{'}'});
		}
	}

	end_trace();
}

// decl_impl : IMPL generic_params? ident generic_params?                           impl_block
//           | IMPL generic_params? ident generic_params? FOR ident generic_params? impl_block
void Parser::decl_impl() {
	trace("decl_impl");

	if (expect_keyword(TokenType::IMPL))
		bug("decl_impl not checked before invoking");

	if (curr_tok.type() == '<')
		generic_params({'{', ';'});

	if (ident(recover::decl_start + Recovery{'{', '<', ';'})) {
		if (curr_tok.type() != '{' && curr_tok.type() != '<' && curr_tok.type() != ';')
			DEFAULT_PARSE_END();
	}
	if (curr_tok.type() == '<')
		generic_params({'{', ';'});

	if (curr_tok == TokenType::FOR) {
		bump();

		if (ident(recover::decl_start + Recovery{'{', '<', ';'})) {
			if (curr_tok.type() != '{' && curr_tok.type() != '<' && curr_tok.type() != ';')
				DEFAULT_PARSE_END();
		}
		if (curr_tok.type() == '<')
			generic_params({'{', ';'});
	}

	if (curr_tok.type() != '{') {
		expect_symbol('{', recover::decl_start + Recovery{'{', ';'});
		if (curr_tok.type() != '{') {
			if (curr_tok.type() == ';')
				bump();
			DEFAULT_PARSE_END();
		}
	}

	impl_block();

	end_trace();
}

// impl_block : '{' decl* '}'
void Parser::impl_block() {
	trace("impl_block");

	if (expect_symbol('{'))
		bug("impl_block not checked before invoking");

	while (curr_tok.type() != '}') {

		auto attrib = attributes();

		// FIXME:  Add constructor support
		if (curr_tok == TokenType::TYPE)
			decl_type();
		else if (curr_tok == TokenType::FUN)
			decl_fun(true);
	}

	EXPECT_OR_PASS('}');
	end_trace();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////    Expr    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// TODO:  we want fun_blocks to also work as expressions
//        e.g 'var foo: Bar = { ... };'
// expr : val (binop expr)*
Error* Parser::expr(int min_prec) {
	trace("expr");

	if (auto err = val(recover::expr_end + Recovery{'+', '-', '*', '/', '^'})) {
		if (!is_binop(curr_tok))
			DEFAULT_PARSE_END(err);
	}

	while (is_binop(curr_tok)) {
		
		auto opinfo = op_find(curr_tok.type());
		if (!opinfo)
			bug("inconsistent binary operator definitions");

		if (opinfo->value.prec < min_prec)
			break;

		trace("binop: " + std::string(curr_tok.raw()));

		bump();
		int next_prec = opinfo->value.assoc == OPInfo::LEFT ? opinfo->value.prec + 1 : opinfo->value.prec;

		end_trace();
		if (auto err = expr(next_prec))
			DEFAULT_PARSE_END(err);
	}

	end_trace();
	return nullptr;
}

// val  : unaryop val
//      | literal
//      | path
//      | path arg_list
//      | path struct_init
//      | path arr_size_decl
//      | '(' expr ')'
Error* Parser::val(const Recovery& recovery) {
	trace("val");

	if (curr_tok == TokenType::ID) {				// path
		auto p = path(recovery);

		if (curr_tok.type() == '(')	{				// path arg_list
			arg_list(recovery);
		}
		else if (curr_tok.type() == '{') {			// path struct_init
			struct_init(recovery);
		}
		else if (curr_tok.type() == '[') {			// path arr_init
			arr_init(recovery);
		}
	}
	else if (curr_tok.type() == '(') {				// '(' expr ')'
		bump();
		expr(1);
		if (auto err = expect_symbol(')', recovery + Recovery{')'})) {
			if (curr_tok.type() == ')')
				bump();
			else DEFAULT_PARSE_END(err);
		}
	}
	else if (is_literal(curr_tok)) {				// literal
		if (literal())
			bug("inconsistent literal definitions");
	}
	else if (is_unaryop(curr_tok)) {						// unaryop val
		if (unaryop())
			bug("inconsistent unary operator definitions");

		if (auto err = val(recovery))
			DEFAULT_PARSE_END(err);
	}
	else {
		auto err = err_expected(translate::tk_type(curr_tok), "an expression");
		recover_to(recovery);
		DEFAULT_PARSE_END(err);
	}

	end_trace();
	return nullptr;
}

// struct_init : '{' '}'
//             | '{' struct_init_item (',' struct_init_item)*     '}'
//             | '{' struct_init_item (',' struct_init_item)* ',' '}'
void Parser::struct_init(const Recovery& recovery) {
	trace("struct_init");

	if (expect_symbol('{'))
		bug("struct_init not checked before invoking");

	if (curr_tok.type() != '}') {
		
		struct_field(recovery + Recovery{',' , '}'});

		while (curr_tok.type() == ',') {
			bump();

			if (curr_tok.type() == '}')
				break;

			struct_field(recovery + Recovery{',' , '}'});
		}

	}

	if (expect_symbol('}', recovery + Recovery{'}'})) {
		if (curr_tok.type() == '}')
			bump();
	}

	end_trace();
}

// struct_field : ident (':' expr)?
Error* Parser::struct_field(const Recovery& recovery) {
	trace("struct_field");

	auto err = ident(recovery + Recovery{':'});
	if (curr_tok.type() == ':') {
		bump();
		expr(1);
	}

	DEFAULT_PARSE_END(err);
}

// arr_init : '[' ']'
//          | '{' arr_field (',' arr_field)*     '}'
//          | '{' arr_field (',' arr_field)* ',' '}'
void Parser::arr_init(const Recovery& recovery) {
	trace("arr_init");

	if (expect_symbol('['))
		bug("arr_init not checked before invoking");

	if (curr_tok.type() != ']') {
		
		arr_field();

		while (curr_tok.type() == ',') {
			bump();

			if (curr_tok.type() == ']')
				break;

			arr_field();
		}
	}

	if (expect_symbol(']', recovery + Recovery{']'})) {
		if (curr_tok.type() == ']')
			bump();
	}

	end_trace();
}

// arr_field : expr
Error* Parser::arr_field() {
	trace("arr_field");

	auto err = expr(1);

	DEFAULT_PARSE_END(err);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////    Type    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// type : '&' MUT? type
//      | '*' MUT? type
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
			if (curr_tok == TokenType::MUT) {		// '&' MUT
				bump();
				trace("type: mut ref");
			}
			else trace("type: ref");
			if (auto err = type(recovery)) {				// '&' type
				DEFAULT_PARSE_END(err);
			}
			break;
	
		// pointer type
		case '*':
			bump();
			if (curr_tok == TokenType::MUT) {		// '*' MUT
				bump();
				trace("type: mut ptr");
			}
			else trace("type: ptr");
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
				if (auto err = type(recover::decl_start + Recovery{']', ';'})) {		// '[' type ']' | '[' type ';' expr ']'
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
					if (auto err = expr(1)) {				// '[' type ';' expr ']	
						recover_to(recovery + Recovery{']'});
						if (curr_tok.type() != ']')
							DEFAULT_PARSE_END(err);
					}
				}
				if (auto err = expect_symbol(']', recover::decl_start + Recovery{']', ';'})) {
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