#include "parser.hpp"
#include "util/ranges.hpp"
#include "util/token_info.hpp"
#include <algorithm>

#define DEFAULT_PARSE_END(x) { end_trace(); return x; }

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

		(int)TokenType::PUB,
		(int)TokenType::PRIV,
		(int)TokenType::STATIC,
		(int)TokenType::CONST,
		(int)TokenType::MUT,
	};

	static const Recovery stmt_start = {
		(int)TokenType::VAR,
		(int)TokenType::FUN,

		(int)TokenType::LOOP,
		(int)TokenType::WHILE,
		(int)TokenType::DO,
		(int)TokenType::FOR,
		(int)TokenType::MATCH,
		(int)TokenType::SWITCH,
		(int)TokenType::CASE,
		(int)TokenType::CONTINUE,
		(int)TokenType::RETURN,
		(int)TokenType::BREAK,

		(int)TokenType::STATIC,
		(int)TokenType::CONST,
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

/* Info about a binary operator. */
struct OPInfo {
	int prec;
	enum {
		LEFT,
		RIGHT
	} assoc;
};


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

/* Concatenate a low span and hi span into a new span. */
inline Span concat_span(const Span& start, const Span& end) {
	return Span(*start.tu, start.lo_bit, end.hi_bit);
}
/* Concatenate a low pos and hi span into a new span. */
inline Span concat_span(size_t lo, const Span& hi_sp) {
	return Span(*hi_sp.tu, lo, hi_sp.hi_bit);
}

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
	return tk == TokenType::LIT_TRUE		||
			tk == TokenType::LIT_FALSE		||
			tk == TokenType::LIT_STRING		||
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

/* True is the provided token is a binary operator. */
inline bool is_binop(const Token& tk) {
	return tk.type() == '+' || tk.type() == '-' || tk.type() == '*' || tk.type() == '/' || tk.type() == '^';
}

/* True is the provided token is a unary operator. */
inline bool is_unaryop(const Token& tk) {
	return tk.type() == '-' || tk.type() == '!' || tk.type() == '&' || tk.type() == '*';
}

/* Splits the current multi-character binop into smaller tokens. */
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

void Parser::recover_to(const Recovery& to) {
	while (curr_tok != TokenType::END) {
		for (auto c : to)
			if (curr_tok.type() == c)
				return;
		bump();
	}
}

inline Recovery operator+(Recovery first, const Recovery& second) {
	first.reserve(first.size() + second.size());
	first.insert(
		std::end(first),
		std::begin(second),
		std::end(second)
	);
	return first;
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
		Token backup = curr_tok;
		auto tk = split_multi_binop();
		if (tk.type() == (int)sym) {
			bump();
			return nullptr;
		}
		curr_tok = backup;
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
	return err_expected(translate::tk_type(curr_tok), "one of 'mod' or 'package'");
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

inline void Parser::expect_sym_recheck(char sym, const Recovery& recov) {
	if (expect_symbol(sym)) {
		recover_to(recov + Recovery{sym});
		if (curr_tok.type() == sym)
			bump();
	}
}

inline void Parser::bug(const std::string& msg) {
	handler.make_bug(msg).emit();
}

inline void Parser::unimpl(const std::string& msg) {
	handler.make_bug(msg + " not implemented yet").emit();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////      Parse Helpers      /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// primitive : THING | STR | CHAR
//           | INT | I64 | I32 | I16 | I8
//           | UINT | U64 | U32 | U16 | U8
//           | FLOAT | F64 | F32
ast::TypePrimitive* Parser::primitive() {
	trace("primitive: " + std::string(curr_tok.raw()));

	auto sp = Span(curr_tok.span());
	ast::TypePrimitive* ret = nullptr;

	switch (curr_tok.type()) {
		case (int)TokenType::THING:
			ret = new ast::TypeThing(sp);
			bump();
			break;
		case (int)TokenType::STR:
			ret = new ast::TypeStr(sp);
			bump();
			break;
		case (int)TokenType::CHAR:
			ret = new ast::TypeChar(sp);
			bump();
			break;
		case (int)TokenType::INT:
			ret = new ast::TypeISize(sp);
			bump();
			break;
		case (int)TokenType::I8:
			ret = new ast::TypeI8(sp);
			bump();
			break;
		case (int)TokenType::I16:
			ret = new ast::TypeI16(sp);
			bump();
			break;
		case (int)TokenType::I32:
			ret = new ast::TypeI32(sp);
			bump();
			break;
		case (int)TokenType::I64:
			ret = new ast::TypeI64(sp);
			bump();
			break;
		case (int)TokenType::UINT:
			ret = new ast::TypeUSize(sp);
			bump();
			break;
		case (int)TokenType::U8:
			ret = new ast::TypeU8(sp);
			bump();
			break;
		case (int)TokenType::U16:
			ret = new ast::TypeU16(sp);
			bump();
			break;
		case (int)TokenType::U32:
			ret = new ast::TypeU32(sp);
			bump();
			break;
		case (int)TokenType::U64:
			ret = new ast::TypeU64(sp);
			bump();
			break;
		case (int)TokenType::FLOAT:
			ret = new ast::TypeFSize(sp);
			bump();
			break;
		case (int)TokenType::F32:
			ret = new ast::TypeF32(sp);
			bump();
			break;
		case (int)TokenType::F64:
			ret = new ast::TypeF64(sp);
			bump();
			break;
		default: {
			if (is_type(curr_tok))
				bug("inconsistent primitive definitions");
			err_expected(translate::tk_type(curr_tok), "a type");
		}
	}	

	DEFAULT_PARSE_END(ret);
}

// ident : ID
ast::Ident* Parser::ident() {
	trace("ident: " + std::string(curr_tok.raw()));

	ast::Ident* id = nullptr;
	if (curr_tok == TokenType::ID) {
		auto sp = Span(curr_tok.span());
		id = new ast::Ident(curr_tok.raw(), sp);
		bump();
	}
	else err_expected(translate::tk_type(curr_tok), "an identifier");

	DEFAULT_PARSE_END(id);
}
// ident : ID
inline ast::Ident* Parser::ident(const Recovery& to) {
	auto ret = ident();
	if (!ret) { recover_to(to); }
	return ret;
}

// lifetime : LF
ast::Lifetime* Parser::lifetime() {
	trace("lifetime: " + std::string(curr_tok.raw()));

	ast::Lifetime* lf = nullptr;
	if (is_lifetime(curr_tok)) {
		auto sp = Span(curr_tok.span());
		lf = new ast::Lifetime(curr_tok.raw(), sp);
		bump();
	}
	else err_expected(translate::tk_type(curr_tok), "a lifetime");

	DEFAULT_PARSE_END(lf);
}
// lifetime : LF
inline ast::Lifetime* Parser::lifetime(const Recovery& to) {
	auto ret = lifetime();
	if (!ret) { recover_to(to); }
	return ret;
}

// literal : LIT_TRUE
//         | LIT_FALSE
//         | LIT_STRING
//         | LIT_CHAR
//         | LIT_INTEGER
//         | LIT_FLOAT
inline ast::Value* Parser::literal() {
	trace("literal: " + translate::tk_info(curr_tok));

	ast::Value* val = nullptr;
	switch (curr_tok.type()) {
		case (int)TokenType::LIT_TRUE: {
			auto sp = Span(curr_tok.span());
			val = new ast::ValueBool(true, sp);
			bump();
			break;
		}
		case (int)TokenType::LIT_FALSE: {
			auto sp = Span(curr_tok.span());
			val = new ast::ValueBool(false, sp);
			bump();
			break;
		}
		case (int)TokenType::LIT_STRING: {
			auto sp = Span(curr_tok.span());
			val = new ast::ValueString(curr_tok.raw(), sp);
			bump();
			break;
		}
		case (int)TokenType::LIT_CHAR: { // TODO:  This will have problems with character values larger than 256
			auto sp = Span(curr_tok.span());
			auto str = std::string(curr_tok.raw());
			val = new ast::ValueChar(curr_tok.raw()[0], sp); /// <--
			bump();
			break;
		}
		case (int)TokenType::LIT_INTEGER: {
			auto sp = Span(curr_tok.span());
			auto str = std::string(curr_tok.raw());
			val = new ast::ValueInt(atoi(str.c_str()), sp);
			bump();
			break;
		}
		case (int)TokenType::LIT_FLOAT: {
			auto sp = Span(curr_tok.span());
			auto str = std::string(curr_tok.raw());
			val = new ast::ValueFloat(atof(str.c_str()), sp);
			bump();
			break;
		}
		default:
			if (is_literal(curr_tok))
				bug("inconsistent literal definitions");
			err_expected(translate::tk_type(curr_tok), "a literal");
			val = nullptr;
			bump();
	}

	DEFAULT_PARSE_END(val);
}
// literal : LIT_TRUE
//         | LIT_FALSE
//         | LIT_STRING
//         | LIT_CHAR
//         | LIT_INTEGER
//         | LIT_FLOAT
inline ast::Value* Parser::literal(const Recovery& to) {
	auto ret = literal();
	if (!ret) { recover_to(to); }
	return ret;
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

// Path : ast::Ident (SCOPE ast::Ident)*
ast::Path* Parser::path(const Recovery& to) {
	trace("path");
	size_t start = curr_tok.span().lo_bit;
	Path path;

	// Add current token to path
	auto id_ret = ident(to + Recovery{(int)TokenType::SCOPE});
	path.push_back(std::unique_ptr<ast::Ident>(id_ret));

	while (curr_tok == TokenType::SCOPE) {
		bump();

		// Add current token to path
		auto id_ret = ident(to + Recovery{(int)TokenType::SCOPE});
		path.push_back(std::unique_ptr<ast::Ident>(id_ret));
	}

	auto sp = concat_span(start, curr_tok.span());
	auto decl = new ast::Path(path, sp);
	DEFAULT_PARSE_END(decl)
}

// generic_params : '<' type_or_lt (',' type_or_lt)* '>'
GenericParamVec Parser::generic_params(const Recovery& recovery) {
	GenericParamVec generics;

	if (curr_tok.type() != '<')
		return generics;
	else bump();

	trace("generic_params");
	
	if (curr_tok.type() != '>') {

		auto param_ret = generic_param(recovery + Recovery{',', '>'});
		generics.push_back(std::unique_ptr<ast::GenericParam>(std::get<1>(param_ret)));

		while (curr_tok.type() == ',') {
			bump();

			if (curr_tok.type() == '>')
				break;

			param_ret = generic_param(recovery + Recovery{',', '>'});
			generics.push_back(std::unique_ptr<ast::GenericParam>(std::get<1>(param_ret)));
			if (std::get<0>(param_ret)) {
				if (curr_tok.type() != ',' && curr_tok.type() != '>') {
					break;
				}
			}
		}
	}
	if (auto err = expect_symbol('>', recovery)) {
		err->set_msg("unterminated generic clause");
	}

	DEFAULT_PARSE_END(generics);
}

// generic_params : type_or_lt
std::tuple<Error*, ast::GenericParam*> Parser::generic_param(const Recovery& recovery) {
	trace("generic_param");
	auto ret = type_or_lt(recovery);
	DEFAULT_PARSE_END(ret);
}

// param_list : '(' ')'
//            | '(' param (',' param)* ')'
ParamVec Parser::param_list(bool is_method, const Recovery& recovery) {
	trace("param_list");

	ParamVec params;

	if (expect_symbol('(', recovery + Recovery{'('})) {
		if (curr_tok.type() == '(')
			bump();
		else DEFAULT_PARSE_END(params);
	}

	if (curr_tok.type() != ')') {
		
		if (is_method && is_type(curr_tok) && curr_tok != TokenType::ID) {
			param_self(recovery + Recovery{',', ')'});
		}
		else {
			auto param_ret = param(recovery + Recovery{',', ')'});
			params.push_back(std::unique_ptr<ast::Param>(std::get<1>(param_ret)));
		}

		while (curr_tok.type() == ',') {
			bump();

			auto param_ret = param(recovery + Recovery{',', ')'});
			params.push_back(std::unique_ptr<ast::Param>(std::get<1>(param_ret)));

			if (curr_tok.type() == ')' || curr_tok.type() != ',')
				break;
		} 
	}
	expect_sym_recheck(')', recovery);


	DEFAULT_PARSE_END(params);
}

// param : ident ':' type
std::tuple<Error*, ast::Param*> Parser::param(const Recovery& recovery) {
	trace("param");
	size_t start = curr_tok.span().lo_bit;
	Error* err = nullptr;

	auto id_ret = ident(recovery + Recovery{':'});
	if (!id_ret)
		err = &handler.last();

	if (auto exp_err = expect_symbol(':', recovery + recover::type_start)) {
		if (!err) { err = exp_err; }
	}

	auto ty_ret = type(recovery);
	if (!err && std::get<0>(ty_ret)) { err = std::get<0>(ty_ret); }

	auto sp = concat_span(start, curr_tok.span());
	auto param = new ast::Param(id_ret, std::get<1>(ty_ret), sp);
	DEFAULT_PARSE_END(std::tuple(err, param));
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
		DEFAULT_PARSE_END(err);
	}

	end_trace();
	return nullptr;
}

// arg_list : '(' ')'
//          | '(' arg (',' arg)* ')'
ExprVec Parser::arg_list(const Recovery& recovery) {
	trace("arg_list");

	ExprVec exprs;

	if (expect_symbol('('))
		bug("arg_list not checked before invoking");

	if (curr_tok.type() != ')') {

		auto arg_ret = arg(recovery + Recovery{',', ')'});
		if (!std::get<0>(arg_ret))
			exprs.push_back(std::unique_ptr<ast::Expr>(std::get<1>(arg_ret)));

		while (curr_tok.type() == ',') {
			bump();

			auto arg_ret = arg(recovery + Recovery{',', ')'});
			if (!std::get<0>(arg_ret))
				exprs.push_back(std::unique_ptr<ast::Expr>(std::get<1>(arg_ret)));

			if (curr_tok.type() == ')')
				break;
		} 
	}
	expect_sym_recheck(')', recovery);

	DEFAULT_PARSE_END(exprs);
}

// arg : expr
std::tuple<Error*, ast::Expr*> Parser::arg(const Recovery& recovery) {
	trace("arg");
	auto ret = expr(1, recovery);
	DEFAULT_PARSE_END(ret);
}

// return_type: type
std::tuple<Error*, ast::Type*> Parser::return_type(const Recovery& recovery) {
	trace("return_type");
	std::tuple<Error*, ast::Type*> ret;
	if (curr_tok == TokenType::RARROW) {
		bump();
		ret = type(recovery);
	}
	else {
		auto sp = Span(curr_tok.span());
		ret = std::tuple(nullptr, new ast::TypeVoid(sp));
	}
	DEFAULT_PARSE_END(ret);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////  ////  ////         Parse         ////  ////  //////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// parse : decl*
std::shared_ptr<ASTRoot> Parser::parse() {
	trace("parse");

	auto ast = std::make_shared<ASTRoot>(&lexer.trans_unit());

	// As long as the end of the file has not been reached,
	// expect to find decls
	while (curr_tok != TokenType::END) {
		ast->add_decl(decl(true));
	}

	DEFAULT_PARSE_END(ast);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////    Decl    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// decl : attributes? decl_module
//      | attributes? decl_import_item
//		| attributes? decl_var
//      | attributes? decl_type
//      | attributes? decl_use
//      | attributes? decl_fun
//      | attributes? decl_struct
//      | attributes? decl_enum
//      | attributes? decl_union
//      | attributes? decl_trait
//      | attributes? decl_impl
ast::Decl* Parser::decl(bool is_global) {
	trace("decl");

	ast::Decl* decl = nullptr;

	auto attr = this->attributes();
	bool is_const = attr.contains(TokenType::CONST);
	bool is_static = attr.contains(TokenType::STATIC);

	switch (curr_tok.type()) 
	{
		case (int)TokenType::MOD:
			decl = decl_module(is_global);
			break;
		case (int)TokenType::IMPORT:
			decl = decl_import_item();
			break;
		case (int)TokenType::VAR:
			decl = decl_var(is_const, is_static);
			break;
		case (int)TokenType::TYPE:
			decl = decl_type();
			break;
		case (int)TokenType::USE:
			decl = decl_use();
			break;
		case (int)TokenType::FUN:
			decl = decl_fun(false);
			break;
		case (int)TokenType::STRUCT:
			decl = nullptr; decl_struct();
			break;
		case (int)TokenType::ENUM:
 			decl = nullptr; decl_enum();
			break;
		case (int)TokenType::UNION:
			decl = nullptr; decl_union();
			break;
		case (int)TokenType::TRAIT:
			decl = nullptr; decl_trait();
			break;
		case (int)TokenType::IMPL:
			decl = nullptr; decl_impl();
			break;

		default: {
			err_expected(translate::tk_type(curr_tok), "a declaration");
			recover_to(recover::decl_start);
		}
	}

	DEFAULT_PARSE_END(decl);
}

// decl_module : MOD path module_block
//             | MOD path ';'               // if global
ast::DeclModule* Parser::decl_module(bool is_global) {
	trace("decl_module");
	size_t start = curr_tok.span().lo_bit;

	if (expect_keyword(TokenType::MOD))
		bug("decl_sub_module not checked before invoking");

	// Split global and non-global recoveries
	Recovery rec = is_global ? Recovery{';', '{'} : Recovery{'{'};

	auto mod_path = path(recover::decl_start + rec);

	std::vector<std::unique_ptr<ast::Decl>> decls;	
	if (curr_tok.type() == ';') {
		// Handle non-global file modules
		// Make an error
		// Cut the module short
		if (!is_global) {
			auto err = handler.make_error_higligted("invalid file module declaration", concat_span(start, curr_tok.span()));
			err->add_help("you can only use file module declaration in the global scope");
			bump();
		}
		else {
			// Collect the proceding declaraions under this module
			bump();
			trace("module_block");
			while (curr_tok != TokenType::END)
				decls.push_back(std::unique_ptr<ast::Decl>(decl(false)));
			end_trace();
		}
	}
	if (curr_tok.type() == '{') {
		decls = module_block();
	}

	// Get the modules span
	auto sp = concat_span(start, curr_tok.span());
	auto decl = new ast::DeclModule(mod_path, decls, sp);
	DEFAULT_PARSE_END(decl);
}

// module_block : '{' decl* '}'
std::vector<std::unique_ptr<ast::Decl>> Parser::module_block() {
	trace("module_block");

	if (expect_symbol('{'))
		bug("module_block not checked before invoking");

	// Keep collection declarations until a bracket is found
	// Fail if the file abruptly ends 
	std::vector<std::unique_ptr<ast::Decl>> decls;
	while (curr_tok.type() != '}') {
		if (curr_tok == TokenType::END) {
			// Fail if the file ends inside the module block
			handler.emit_fatal_higligted("unexpected end of file", curr_tok.span());
			return decls;
		}
		// Save declarations
		decls.push_back(std::unique_ptr<ast::Decl>(decl(false)));
	}
	expect_sym_recheck('}', recover::decl_start);

	DEFAULT_PARSE_END(decls);
}

// decl_import_item : IMPORT MOD path ';'
//                  | IMPORT PACKAGE path ';'
ast::Decl* Parser::decl_import_item() {
	trace("decl_import_item");
	size_t start = curr_tok.span().lo_bit;

	if (expect_keyword(TokenType::IMPORT))
		bug("decl_import_item not checked before invoking");

	enum {
		MOD,
		PACKAGE
	} import_ty;

	if (curr_tok == TokenType::MOD) {
		import_ty = MOD;
		bump();
	}
	else if (curr_tok == TokenType::PACKAGE) {
		import_ty = PACKAGE;
		bump();
	}
	else {
		expect_mod_or_package(recover::decl_start + Recovery{';'});
		if (curr_tok.type() == ';')
			bump();
		DEFAULT_PARSE_END(nullptr);
	}

	auto import_path = path(recover::decl_start + recover::semi);

	expect_sym_recheck(';', recover::decl_start);

	auto sp = concat_span(start, curr_tok.span());
	auto decl = (import_ty == MOD) ?
		(ast::Decl*) new ast::DeclModuleImport(import_path, sp) :
		(ast::Decl*) new ast::DeclPackageImport(import_path, sp);

	DEFAULT_PARSE_END(decl);
}

// decl_var : VAR ident (':' type_with_lf)? ('=' expr)? ';'
ast::DeclVar* Parser::decl_var(bool is_const, bool is_static) {
	trace("decl_var");
	size_t start = curr_tok.span().lo_bit;

	if (expect_keyword(TokenType::VAR))
		bug("decl_var not checked before invoking");

	bool has_type = true;
	bool has_init = true;

	auto id_ret = ident(recover::decl_start + Recovery{':', '=', ';'});

	ast::Lifetime* lifetime = nullptr;
	ast::Type* type = nullptr;
	ast::Expr* value = nullptr;

	auto type_pos = curr_tok.span();
	if (curr_tok.type() == ':') {
		bump();
		type_pos = curr_tok.span();

		auto ty_lt_ret = type_with_lt(recover::decl_start + Recovery{'=', ';'});

		lifetime = std::get<1>(ty_lt_ret);
		type = std::get<2>(ty_lt_ret);
	}
	else has_type = false;

	if (curr_tok.type() == '=') {
		bump();
		auto expr_ret = expr(1, recover::decl_start + recover::semi);
		value = std::get<1>(expr_ret);
	}
	else has_init = false;

	if (!has_type && !has_init) {
		auto err = handler.make_error_higligted("variable missing type", type_pos);
		if (is_const)
			err->add_note("a constant variable needs to have a type deducable at compile time");
		if (is_static)
			err->add_note("a static variable needs to have a type deducable at compile time");
	}

	expect_sym_recheck(';', recover::decl_start);

	auto sp = concat_span(start, curr_tok.span());
	auto decl = new ast::DeclVar(id_ret, lifetime, type, value, sp);
	DEFAULT_PARSE_END(decl);
}

// decl_type : TYPE ident ';'
//           | TYPE ident '=' type ';'
ast::DeclType* Parser::decl_type() {
	trace("decl_type");
	size_t start = curr_tok.span().lo_bit;

	if (expect_keyword(TokenType::TYPE))
		bug("decl_type not checked before invoking");

	auto id_ret = ident(recover::decl_start + recover::semi);

	ast::Type* ty = nullptr;
	if (curr_tok.type() == '=') {
		bump();

		auto ty_ret = type(recover::decl_start + recover::semi);
		ty = std::get<1>(ty_ret);
	}

	expect_sym_recheck(';', recover::decl_start);

	auto sp = concat_span(start, curr_tok.span());
	auto decl = new ast::DeclType(id_ret, ty, sp);
	DEFAULT_PARSE_END(decl);
}

// decl_use : USE path ';'
ast::DeclUse* Parser::decl_use() {
	trace("decl_use");
	size_t start = curr_tok.span().lo_bit;

	if (expect_keyword(TokenType::USE))
		bug("decl_use not checked before invoking");

	auto path = this->path(recover::decl_start + recover::semi);

	expect_sym_recheck(';', recover::decl_start);

	auto sp = concat_span(start, curr_tok.span());
	auto decl = new ast::DeclUse(path, sp);
	DEFAULT_PARSE_END(decl);
}

// decl_fun : FUN ident generic_params? param_list (RARROW return_type)? ';'
//          | FUN ident generic_params? param_list (RARROW return_type)? fun_block
ast::DeclFun* Parser::decl_fun(bool is_method) {
	trace("decl_fun");
	size_t start = curr_tok.span().lo_bit;

	if (expect_keyword(TokenType::FUN))
		bug("decl_fun not checked before invoking");

	auto id_ret = ident(recover::decl_start + Recovery{(int)TokenType::ID, '<', '(', (int)TokenType::RARROW, ';', '{'});
	if (curr_tok.type() == (int)TokenType::ID)
		id_ret = ident();

	auto generics = generic_params(recover::decl_start + Recovery{'(', (int)TokenType::RARROW, '{'});

	auto params = param_list(is_method, {(int)TokenType::RARROW, '{'});
	
	auto ret_type = return_type(recover::decl_start + Recovery{'{', ';'});

	FunBlock block;
	if (curr_tok.type() == ';') {
		bump();
	}
	else if (curr_tok.type() != '{') {
		// Attempt recovery to the next declaration or
		// to the continuation of the function definition
		expect_symbol('{', recover::decl_start + Recovery{'{', ';'});
		if (curr_tok.type() != '{') {
			if (curr_tok.type() == ';')
				bump();
		}
		else block = fun_block();
	}
	else block = fun_block();
	
	auto sp = concat_span(start, curr_tok.span());
	auto decl = new ast::DeclFun(id_ret, generics, params, std::get<1>(ret_type), block, sp);
	DEFAULT_PARSE_END(decl);
}

// fun_block : '{' stmt* '}'
FunBlock Parser::fun_block() {
	trace("fun_block");
	size_t start = curr_tok.span().lo_bit;

	if (expect_symbol('{'))
		bug("fun_block not checked before invoking");

	// Collect statements 
	StmtVec stmts;
	while (curr_tok.type() != '}') {
		stmts.push_back(std::unique_ptr<ast::Stmt>(stmt({'}'})));
	}
	expect_sym_recheck('}', recover::decl_start);

	auto sp = concat_span(start, curr_tok.span());
	FunBlock block = FunBlock(stmts, sp);
	DEFAULT_PARSE_END(block);
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
		
		struct_tuple_item(recover::decl_start + Recovery{',', ')'});

		while (curr_tok.type() == ',') {
			bump();

			if (curr_tok.type() == ')')
				break;

			struct_tuple_item(recover::decl_start + Recovery{',', ')'});
		}
	}
	expect_sym_recheck(')', recover::decl_start);

	end_trace();
}

// struct_tuple_item  : attributes? type
void Parser::struct_tuple_item(const Recovery& recovery) {
	trace("struct_tuple_item");

	Attributes attr = attributes();

	type_with_lt(recovery);

	end_trace();
}

// struct_named_block : '{' struct_named_item* '}'
void Parser::struct_named_block() {
	trace("struct_named_block");

	if (expect_symbol('{'))
		bug("struct_named_block not checked before invoking");

	while (curr_tok.type() != '}') {
		struct_named_item(recover::decl_start + Recovery{'}'});
	}
	expect_sym_recheck('}', recover::decl_start);

	end_trace();
}

// struct_named_item  : attributes? ident ':' type_with_lt ';' 
void Parser::struct_named_item(const Recovery& recovery) {
	trace("struct_named_item");

	auto attr = attributes();

	auto id_ret = ident(recovery + Recovery{':', ';'});
	if (!id_ret) {
		if (curr_tok.type() != ':') {
			if (curr_tok.type() == ';')
				bump();
			DEFAULT_PARSE_END();
		}
		else bump();
	}

	if (expect_symbol(':', recovery + Recovery{':', ';'})) {
		if (curr_tok.type() != ':') {
			if (curr_tok.type() == ';')
				bump();
			DEFAULT_PARSE_END();
		}
		else bump();
	}

	type_with_lt(recovery + recover::semi);

	expect_sym_recheck(';', recover::decl_start);

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
	expect_sym_recheck('}', recover::decl_start);

	end_trace();
}

// enum_item : ident
//           | ident '=' expr
//           | ident struct_tuple_block
Error* Parser::enum_item(const Recovery& recovery) {
	trace("enum_item");

	auto id_ret = ident(recovery + Recovery{'=', '('});
	if (!id_ret) {
		if (curr_tok.type() != '=' && curr_tok.type() != '(')
			DEFAULT_PARSE_END(&handler.last());
	}

	if (curr_tok.type() == '=') {
		bump();
		auto expr_ret = expr(1);
		if (std::get<0>(expr_ret)) {
			recover_to(recovery);
			DEFAULT_PARSE_END(std::get<0>(expr_ret));
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
	expect_sym_recheck('}', recover::decl_start);

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

	auto id_ret = ident(recover::decl_start + Recovery{'{', '<', ';'});
	if (!id_ret) {
		if (curr_tok.type() != '{' && curr_tok.type() != '<' && curr_tok.type() != ';')
			DEFAULT_PARSE_END();
	}
	if (curr_tok.type() == '<')
		generic_params({'{', ';'});

	if (curr_tok == TokenType::FOR) {
		bump();

		auto id_ret = ident(recover::decl_start + Recovery{'{', '<', ';'});
		if (!id_ret) {
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
	expect_sym_recheck('}', recover::decl_start);

	end_trace();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////    Stmt    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

ast::Stmt* Parser::stmt(const Recovery& recovery) {
	trace("stmt");
	
	ast::Stmt* stmt = nullptr;

	auto attr = attributes();
	bool is_const = attr.contains(TokenType::CONST);
	bool is_static = attr.contains(TokenType::STATIC);

	switch (curr_tok.type()) {
		case (int)TokenType::VAR:
			stmt = decl_var(is_const, is_static);
			break;

		case (int)TokenType::FUN:
			stmt = nullptr; decl_fun(false);
			break;

		case (int)TokenType::IF:
			stmt = nullptr; stmt_if(recover::stmt_start + recovery);
			break;

		case (int)TokenType::ELSE:
			handler.make_error_higligted("missing 'if' statement", curr_tok.span());
			stmt = nullptr; stmt_else(recover::stmt_start + recovery);
			break;

		case (int)TokenType::LOOP:
			stmt = nullptr; stmt_loop(recover::stmt_start + recovery);
			break;

		case (int)TokenType::WHILE:
			stmt = nullptr; stmt_while(recover::stmt_start + recovery);
			break;

		case (int)TokenType::DO:
			stmt = nullptr; stmt_do(recover::stmt_start + recovery);
			break;

		case (int)TokenType::FOR:
			stmt = nullptr; stmt_for(recover::stmt_start + recovery);
			break;

		case (int)TokenType::MATCH:
			stmt = nullptr; stmt_match(recover::stmt_start + recovery);
			break;

		case (int)TokenType::SWITCH:
			stmt = nullptr; stmt_switch(recover::stmt_start + recovery);
			break;

		case (int)TokenType::CASE:
			stmt = nullptr; stmt_case(recover::stmt_start + recovery);
			break;

		case (int)TokenType::RETURN:
			stmt = nullptr; stmt_return(recover::stmt_start + recovery);
			break;

		case (int)TokenType::BREAK:
			stmt = nullptr; stmt_break(recover::stmt_start + recovery);
			break;

		case (int)TokenType::CONTINUE:
			stmt = nullptr; stmt_continue(recover::stmt_start + recovery);
			break;

		default: {
				size_t start = curr_tok.span().lo_bit;

				// Attempt to parse an expression, since no other statemnts match
				auto expr_ret = expr(1);

				// If we haven't moved forward after parsing an expression,
				// somthing is wrong, so make an error about expecting a statement.
				// 
				// After that try to recover to the given recovery or a semicolon.
				if (start == curr_tok.span().lo_bit) {
					std::get<0>(expr_ret)->cancel();
					err_expected(translate::tk_type(curr_tok), "a statement");
					recover_to(recovery + Recovery{';'});
				}

				expect_sym_recheck(';', recovery);
			}
	}

	DEFAULT_PARSE_END(stmt);
}

// FIXME:  add error handling
void Parser::stmt_if(const Recovery& recovery) {
	trace("stmt_if");

	if (expect_keyword(TokenType::IF))
		bug("stmt_if not checked before invoking");

	expr(1);

	expect_symbol('{');

	while (curr_tok.type() != '}')
		stmt({'}'});

	expect_symbol('}');

	if (curr_tok == TokenType::ELSE)
		stmt_else(recovery);

	end_trace();
}

// FIXME:  add error handling
void Parser::stmt_else(const Recovery& recovery) {
	trace("stmt_else");

	if (expect_keyword(TokenType::ELSE))
		bug("stmt_else not checked before invoking");

	if (curr_tok == TokenType::IF)
		expr(1);

	expect_symbol('{');

	while (curr_tok.type() != '}')
		stmt({'}'});

	expect_symbol('}');

	(void)recovery;
	end_trace();
}

// FIXME:  add error handling
void Parser::stmt_loop(const Recovery& recovery) {
	trace("stmt_loop");

	if (expect_keyword(TokenType::LOOP))
		bug("stmt_loop not checked before invoking");

	expect_symbol('{');

	while (curr_tok.type() != '}')
		stmt({'}'});

	expect_symbol('}');

	(void)recovery;
	end_trace();
}

// FIXME:  add error handling
void Parser::stmt_while(const Recovery& recovery) {
	trace("stmt_while");

	if (expect_keyword(TokenType::WHILE))
		bug("stmt_while not checked before invoking");

	expr(1);

	expect_symbol('{');

	while (curr_tok.type() != '}')
		stmt({'}'});

	expect_symbol('}');

	(void)recovery;
	end_trace();
}

// FIXME:  add error handling
void Parser::stmt_do(const Recovery& recovery) {
	trace("stmt_do");

	if (expect_keyword(TokenType::DO))
		bug("stmt_do not checked before invoking");

	expect_symbol('{');

	while (curr_tok.type() != '}')
		stmt({'}'});

	expect_symbol('}');

	expect_keyword(TokenType::WHILE);

	expect_symbol('(');

	while (curr_tok.type() != ')')
		stmt({')', ';'});

	expect_sym_recheck(')', recover::stmt_start + recovery + Recovery{';'});
	expect_sym_recheck(';', recover::stmt_start + recovery);

	end_trace();
}

// FIXME:
void Parser::stmt_for(const Recovery& recovery) {
	trace("stmt_for");

	if (expect_keyword(TokenType::FOR))
		bug("stmt_for not checked before invoking");

	unimpl("stmt_for");

	(void)recovery;
	end_trace();
}

// FIXME:
void Parser::stmt_match(const Recovery& recovery) {
	trace("stmt_match");

	if (expect_keyword(TokenType::MATCH))
		bug("stmt_match not checked before invoking");

	unimpl("stmt_match");

	(void)recovery;
	end_trace();
}

// FIXME:
void Parser::stmt_switch(const Recovery& recovery) {
	trace("stmt_switch");

	if (expect_keyword(TokenType::SWITCH))
		bug("stmt_switch not checked before invoking");

	unimpl("stmt_switch");

	(void)recovery;
	end_trace();
}

// stmt_case : CASE expr ':'
void Parser::stmt_case(const Recovery& recovery) {
	trace("stmt_case");

	if (expect_keyword(TokenType::CASE))
		bug("stmt_case not checked before invoking");

	expr(1);

	expect_sym_recheck(':', recovery);

	end_trace();
}

// stmt_return : RETURN expr? ';'
void Parser::stmt_return(const Recovery& recovery) {
	trace("stmt_return");

	if (expect_keyword(TokenType::RETURN))
		bug("stmt_return not checked before invoking");

	if (curr_tok.type() != ';')
		expr(1);

	expect_sym_recheck(';', recovery);

	end_trace();
}

// stmt_break : BREAK ';'
void Parser::stmt_break(const Recovery& recovery) {
	trace("stmt_break");

	if (expect_keyword(TokenType::BREAK))
		bug("stmt_break not checked before invoking");

	expect_sym_recheck(';', recovery);

	end_trace();
}

// stmt_continue : CONTINUE ';'
void Parser::stmt_continue(const Recovery& recovery) {
	trace("stmt_continue");

	if (expect_keyword(TokenType::CONTINUE))
		bug("stmt_continue not checked before invoking");

	expect_sym_recheck(';', recovery);

	end_trace();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////    Expr    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// TODO:  we want fun_blocks to also work as expressions
//        e.g 'var foo: Bar = { ... };'
// expr : val (binop expr)*
std::tuple<Error*, ast::Expr*> Parser::expr(int min_prec) {
	trace("expr");
	
	std::tuple<Error*, ast::Expr*> ret;

	auto val_ret = val(recover::expr_end + Recovery{'+', '-', '*', '/', '^'});
	auto err = std::get<0>(val_ret);
	if (err) {
		if (!is_binop(curr_tok)) {
			ret = std::tuple(err, nullptr);
			bump();
		}
	}

	while (is_binop(curr_tok)) {
		
		auto opinfo = op_find(curr_tok.type());
		if (!opinfo)
			bug("inconsistent binary operator definitions");

		if (opinfo->value.prec < min_prec)
			break;

		trace("binop: " + std::string(curr_tok.raw()));
		end_trace();

		bump();
		int next_prec = opinfo->value.assoc == OPInfo::LEFT ? opinfo->value.prec + 1 : opinfo->value.prec;

		auto expr_ret = expr(next_prec);
		auto err = std::get<0>(expr_ret);
		if (err)
			ret = std::tuple(err, nullptr);
	}

	DEFAULT_PARSE_END(ret);
}

// expr : val (binop expr)*
std::tuple<Error*, ast::Expr*> Parser::expr(int min_prec, const Recovery& recovery) {
	auto ret = expr(min_prec);
	if (std::get<0>(ret)) recover_to(recovery);
	return ret;
}

// val  : unaryop val
//      | literal
//      | path
//      | path arg_list              // fun call
//      | path '!' arg_list          // macro invocation
//      | path struct_init           // struct creation
//      | '[' expr (',' expr)* ']'
//      | '(' expr (',' expr)* ')'
//      | '(' ')'
std::tuple<Error*, ast::Value*> Parser::val(const Recovery& recovery) {
	trace("val");
	std::tuple<Error*, ast::Value*> ret;

	if (curr_tok == TokenType::ID) {				// path
		size_t start = curr_tok.span().lo_bit;

		auto val_path = path(recovery);

		switch(curr_tok.type()) {
			case '!': {
				bump();
				auto args = arg_list(recovery);

				auto sp = concat_span(start, curr_tok.span());
				auto val = new ast::ValueMacroInvoc(val_path, args, sp);

				ret = std::tuple(nullptr, val);
				break;
			}

			case '(': {
				auto args = arg_list(recovery);

				auto sp = concat_span(start, curr_tok.span());
				auto val = new ast::ValueFunCall(val_path, args, sp);

				ret = std::tuple(nullptr, val);
				break;
			}
			case '{': {
				auto fields = struct_init(recovery);

				auto sp = concat_span(start, curr_tok.span());
				auto val = new ast::ValueStruct(val_path, fields, sp);

				ret = std::tuple(nullptr, val);
				break;
			}

			default:
				auto sp = concat_span(start, curr_tok.span());
				auto val = new ast::ValuePath(val_path, sp);

				ret = std::tuple(nullptr, val);
		}
	}
	else if (curr_tok.type() == '(') {				// '('
		size_t start = curr_tok.span().lo_bit;
		bump();

		ExprVec exprs;
		Error* err = nullptr;

		if (curr_tok.type() != ')') {				// '(' ')'
			auto expr_ret = expr(1);
			err = std::get<0>(expr_ret);
			auto expr_1 = std::get<1>(expr_ret);
			exprs.push_back(std::unique_ptr<ast::Expr>(expr_1));

			while (curr_tok.type() == ',') {		// '(' expr (',' expr)* ')'
				bump();

				auto expr_ret = expr(1);
				if (!err) { err = std::get<0>(expr_ret); }
				auto expr_1 = std::get<1>(expr_ret);
				exprs.push_back(std::unique_ptr<ast::Expr>(expr_1));
			}
		}
		expect_sym_recheck(')', recovery);

		auto sp = concat_span(start, curr_tok.span());
		auto decl = new ast::ValueTuple(exprs, sp);
		ret = std::tuple(err, decl);
	}
	else if (curr_tok.type() == '[') {
		size_t start = curr_tok.span().lo_bit;
		bump();

		ExprVec exprs;
		Error* err = nullptr;

		if (curr_tok.type() != ']') {				// '(' ')'
			auto expr_ret = expr(1);
			err = std::get<0>(expr_ret);
			auto expr_1 = std::get<1>(expr_ret);
			exprs.push_back(std::unique_ptr<ast::Expr>(expr_1));

			while (curr_tok.type() == ',') {		// '(' expr (',' expr)* ')'
				bump();

				auto expr_ret = expr(1);
				if (!err) { err = std::get<0>(expr_ret); }
				auto expr_1 = std::get<1>(expr_ret);
				exprs.push_back(std::unique_ptr<ast::Expr>(expr_1));
			}
		}
		expect_sym_recheck(']', recovery);

		auto sp = concat_span(start, curr_tok.span());
		auto decl = new ast::ValueArray(exprs, sp);
		ret = std::tuple(err, decl);
	}
	else if (is_literal(curr_tok)) {				// literal
		auto lit_ret = literal();

		ret = lit_ret ?
			std::tuple((Error*)nullptr, lit_ret) :
			std::tuple(&handler.last(), nullptr);
	}
	else if (is_unaryop(curr_tok)) {				// unaryop val
		auto uop = unary_op();

		auto val_ret = val(recovery);
		if (std::get<1>(val_ret))
			std::get<1>(val_ret)->add_uop(uop);

		ret = val_ret;
	}
	else {
		auto err = err_expected(translate::tk_type(curr_tok), "an expression");
		recover_to(recovery);
		ret = std::tuple(err, nullptr);
	}

	DEFAULT_PARSE_END(ret);
}

ast::UnaryOp* Parser::unary_op() {
	trace("unary_op: " + translate::tk_info(curr_tok));

	ast::UnaryOp* uop = nullptr;
	switch (curr_tok.type()) {
		case '-': {
			auto sp = Span(curr_tok.span());
			uop = new ast::UopNeg(sp);
			bump();
			break;
		}
		case '!': {
			auto sp = Span(curr_tok.span());
			uop = new ast::UopNot(sp);
			bump();
			break;
		}
		case '&': {
			auto sp = Span(curr_tok.span());
			uop = new ast::UopAddr(sp);
			bump();
			break;
		}
		case '*': {
			auto sp = Span(curr_tok.span());
			uop = new ast::UopDeref(sp);
			bump();
			break;
		}
		default:
			if (is_unaryop(curr_tok))
				bug("inconsistent unary operator definitions");
	}

	DEFAULT_PARSE_END(uop);
}

// struct_init : '{' '}'
//             | '{' struct_init_item (',' struct_init_item)*     '}'
//             | '{' struct_init_item (',' struct_init_item)* ',' '}'
StructFieldVec Parser::struct_init(const Recovery& recovery) {
	trace("struct_init");

	StructFieldVec fields;

	if (expect_symbol('{'))
		bug("struct_init not checked before invoking");

	if (curr_tok.type() != '}') {
		
		auto field_ret = struct_field(recovery + Recovery{',' , '}'});
		if (std::get<0>(field_ret))
			fields.push_back(std::move(std::get<1>(field_ret)));

		while (curr_tok.type() == ',') {
			bump();

			if (curr_tok.type() == '}')
				break;

			auto field_ret = struct_field(recovery + Recovery{',' , '}'});
			if (std::get<0>(field_ret))
				fields.push_back(std::move(std::get<1>(field_ret)));
		}

	}
	expect_sym_recheck('}', recovery);

	DEFAULT_PARSE_END(fields);
}

// struct_field : ident (':' expr)?
std::tuple<Error*, IDExprPair> Parser::struct_field(const Recovery& recovery) {
	trace("struct_field");

	Error* err = nullptr;
	ast::Expr* expr_end = nullptr;

	auto id_ret = ident(recovery + Recovery{':'});

	if (!id_ret)
		err = &handler.last();

	if (curr_tok.type() == ':') {
		bump();
		auto expr_ret = expr(1);
		if (!err) { err = std::get<0>(expr_ret); }
		expr_end = std::get<1>(expr_ret);
	}

	auto pair = IDExprPair(id_ret, expr_end);
	auto ret = std::tuple(err, std::move(pair));

	DEFAULT_PARSE_END(ret);
}

// arr_init : '[' ']'
//          | '[' arr_field (',' arr_field)*     ']'
//          | '[' arr_field (',' arr_field)* ',' ']'
ExprVec Parser::arr_init(const Recovery& recovery) {
	trace("arr_init");

	if (expect_symbol('['))
		bug("arr_init not checked before invoking");

	ExprVec fields;

	if (curr_tok.type() != ']') {
		
		auto field_ret = arr_field();
		if (std::get<0>(field_ret))
			fields.push_back(std::unique_ptr<ast::Expr>(std::get<1>(field_ret)));

		else {
			while (curr_tok.type() == ',') {
				bump();

				if (curr_tok.type() == ']')
					break;

				auto field_ret = arr_field();
				if (std::get<0>(field_ret))
					fields.push_back(std::unique_ptr<ast::Expr>(std::get<1>(field_ret)));
			}
		}
	}
	expect_sym_recheck(']', recovery);

	DEFAULT_PARSE_END(fields)
}

// arr_field : expr
std::tuple<Error*, ast::Expr*> Parser::arr_field() {
	trace("arr_field");
	auto ret = expr(1);
	DEFAULT_PARSE_END(ret);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////    Type    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// type : type_ref
//      | type_ptr
//      | type_arr_or_slice
//      | type_tuple
//      | type_path
//      | type_infer
//      | primitive
std::tuple<Error*, ast::Type*> Parser::type(const Recovery& recovery) {
	trace("type");

	std::tuple<Error*, ast::Type*> ret;

	switch (curr_tok.type()) {
		case '&':
			ret = type_ref(recovery);
			break;

		case '*':
			ret = type_ptr(recovery);
			break;

		case '[':
			ret = type_arr_or_slice(recovery);
			break;

		case '(': {
			ret = type_tuple(recovery);
			break;
		}

		case (int)TokenType::ID: {
			ret = std::tuple(nullptr, type_path(recovery));
			break;
		}

		case '_': {
			ret = std::tuple(nullptr, type_infer());
			break;
		}

		default: {
			if (is_primitive(curr_tok)) {
				ret = std::tuple(nullptr, type_primitive());
			}
			else {
				auto err = err_expected(translate::tk_type(curr_tok), "a type");
				recover_to(recovery);

				ret = std::tuple(err, nullptr);
 			}
		}
	}

	DEFAULT_PARSE_END(ret);
}

std::tuple<Error*, ast::TypeRef*> Parser::type_ref(const Recovery& recovery) {
	trace("type_ref");
	size_t start = curr_tok.span().lo_bit;

	if (expect_symbol('&'))
		bug("type_ptr not checked before invoking");

	auto mut = Mutability::Immutable;
	if (curr_tok == TokenType::MUT) {
		bump();
		mut = Mutability::Mutable;
	}

	auto ty_ret = type(recovery);

	auto sp = concat_span(start, curr_tok.span());
	auto ty = new ast::TypeRef(std::get<1>(ty_ret), mut, sp);

	auto ret = std::tuple(std::get<0>(ty_ret), ty);
	DEFAULT_PARSE_END(ret);
}

std::tuple<Error*, ast::TypePtr*> Parser::type_ptr(const Recovery& recovery) {
	trace("type_ptr");
	size_t start = curr_tok.span().lo_bit;

	if (expect_symbol('*'))
		bug("type_ptr not checked before invoking");

	auto mut = Mutability::Immutable;
	if (curr_tok == TokenType::MUT) {
		bump();
		mut = Mutability::Mutable;
	}

	auto ty_ret = type(recovery);

	auto sp = concat_span(start, curr_tok.span());
	auto ty = new ast::TypePtr(std::get<1>(ty_ret), mut, sp);

	auto ret = std::tuple(std::get<0>(ty_ret), ty);
	DEFAULT_PARSE_END(ret)
}

// type_tuple : '(' ')'                      // TypeVoid
//            | '(' typle ')'                // Type
//            | '(' typle (',' type)* ')'    // TypeTuple
std::tuple<Error*, ast::Type*> Parser::type_tuple(const Recovery& recovery) {
	trace("type_tuple");
	size_t start = curr_tok.span().lo_bit;

	std::tuple<Error*, ast::Type*> ret;

	if (expect_symbol('('))
		bug("type_tuple not checked before invoking");

	if (curr_tok.type() == ')') {
		bump();	

		auto sp = concat_span(start, curr_tok.span());
		auto ty = new ast::TypeVoid(sp);

		ret = std::tuple(nullptr, ty);			
	}
	else {
		auto ty_ret = type(recovery + Recovery{',', ')'});
		Error* err = std::get<0>(ty_ret);

		TypeVec types;
		types.push_back(std::unique_ptr<ast::Type>(std::get<1>(ty_ret)));

		while(curr_tok.type() != ')') {
			
			if (curr_tok.type() == ',') {
				bump();
			} else break;

			auto ty_ret = type(recovery + Recovery{',', ')'});
			if (!err) { err = std::get<0>(ty_ret); }
			types.push_back(std::unique_ptr<ast::Type>(std::get<1>(ty_ret)));
		}

		expect_sym_recheck(')', recovery);

		auto sp = concat_span(start, curr_tok.span());
		ast::Type* ty = types.size() == 1 ? types[0].release() : (ast::Type*)new ast::TypeTuple(types, sp);

		ret = std::tuple(err, ty);
	}

	DEFAULT_PARSE_END(ret);
}

// type_arr_or_slice : '[' type ']'
//                   | '[' typle ';' expr ']'
std::tuple<Error*, ast::Type*> Parser::type_arr_or_slice(const Recovery& recovery) {
	trace("type_arr_or_slice");
	size_t start = curr_tok.span().lo_bit;

	if (expect_symbol('['))
		bug("type_arr_or_slice not checked before invoking");

	auto ty_ret = type(recover::decl_start + Recovery{';', ']'});
	Error* err = std::get<0>(ty_ret);

	ast::Type* type = nullptr;

	if (curr_tok.type() == ';') {
		bump();

		auto expr_ret = expr(1, recovery + Recovery{']'});
		if (!err) { err = std::get<0>(expr_ret); }

		auto sp = concat_span(start, curr_tok.span());
		type = new ast::TypeArray(std::get<1>(ty_ret), std::get<1>(expr_ret), sp);
	}
	else {
		auto sp = concat_span(start, curr_tok.span());
		type = new ast::TypeSlice(std::get<1>(ty_ret), sp);
	}

	expect_sym_recheck(']', recover::decl_start);

	auto ret = std::tuple(err, type);
	DEFAULT_PARSE_END(ret);
}

// type_path : path
ast::TypePath* Parser::type_path(const Recovery& recovery) {
	trace("type_path");
	size_t start = curr_tok.span().lo_bit;

	auto p = path(recovery);
	auto generics = generic_params(recovery);

	auto sp = concat_span(start, curr_tok.span());
	auto ty = new ast::TypePath(p, generics, sp);
	DEFAULT_PARSE_END(ty);
}

// type_infer : '_'
ast::TypeInfer* Parser::type_infer() {
	trace("type_infer");
	auto sp = Span(curr_tok.span());

	if (expect_symbol('_'))
		bug("type_infer not checked before invoking");

	auto ty = new ast::TypeInfer(sp);
	DEFAULT_PARSE_END(ty);
}

// type_primitive : primitive
ast::TypePrimitive* Parser::type_primitive() {
	trace("type_primitive");
	auto ret = primitive();
	DEFAULT_PARSE_END(ret);
}

// type_or_lt : type
//            | lifetime
std::tuple<Error*, ast::GenericParam*> Parser::type_or_lt(const Recovery& recovery) {
	trace("type_or_lt");

	Error* err = nullptr;
	ast::GenericParam* ty_or_lf = nullptr;

	if (is_lifetime(curr_tok)) {
		auto lf_ret = lifetime();
		if (!lf_ret)
			err = &handler.last();

		ty_or_lf = new ast::GenericLifetime(lf_ret, lf_ret->span);
	}
	else {
		auto ty_ret = type(recovery);

		if (auto err = std::get<0>(ty_ret)) {
			auto tmp_err = err_expected(translate::tk_type(curr_tok), "a type or lifetime", 0);
			err->set_msg(tmp_err->message());
			if (!err) { err = std::get<0>(ty_ret); }
		}

		ty_or_lf = new ast::GenericType(std::get<1>(ty_ret), std::get<1>(ty_ret)->span);
	}

	auto ret = std::tuple(err, ty_or_lf);
	DEFAULT_PARSE_END(ret);
}

// type_with_lt : lifetime? type
std::tuple<Error*, ast::Lifetime*, ast::Type*> Parser::type_with_lt(const Recovery& recovery) {
	trace("type_with_lt");

	ast::Lifetime* lf = nullptr;
	Error* err = nullptr;

	// A lifetime is not manditory, so expect to find it only if it is given
	// If the lifetime returns with an error, store that aswell
	if (is_lifetime(curr_tok)) {
		auto lf_ret = lifetime();
		if (!lf_ret)
			err = &handler.last();
		lf = lf_ret;
	}
	auto type_ret = type(recovery);
	// If the lifetime didn't give errors,
	// store a possible type error
	if (!err) { err = std::get<0>(type_ret); }

	auto ret = std::tuple(err, lf, std::get<1>(type_ret));
	DEFAULT_PARSE_END(ret);
}