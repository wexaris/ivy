#include "parser.hpp"
#include "util/ranges.hpp"
#include "ast/ast.hpp"

bool Attributes::contains(TokenType ty) {
	for (auto attr : *this)
		if (attr.ty == ty)
			return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////    Expects    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void Parser::expected_msg(const std::string& expected) {
	err("unexpected token: " + translate::tk_type(curr_tok) + "; expected " + expected);
}

inline void Parser::expect(int exp) { 
	if (curr_tok.type() != exp)
		expected_msg(translate::tk_type(exp));
	bump();
}

void Parser::expect(const std::vector<TokenType>& exp) {
	// Loop through vector of allowed tokens
	// Check if the current token matches any of them
	// Return on first match
	for (auto i : exp) {
		if (curr_tok == i) {
			bump();
			return;
		}
	}

	// Collect all of the acceptable types
	std::string expected = translate::tk_type(exp[0]);
	for (size_t i = 1; i < exp.size(); i++)
		expected += ", " + translate::tk_type(exp[i]);

	expected_msg("one of " + expected);
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

/* True if the provided token's type is a literal. */
inline bool is_literal(const Token& tk) {
	return tk == TokenType::LIT_STRING	||
			tk == TokenType::LIT_NUMBER	||
			tk == TokenType::LIT_INT	||
			tk == TokenType::LIT_UINT	||
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
////////////////////////////////////      Helper Methods      /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// attributes: PUB | PRIV | MUT | CONST
Attributes Parser::attributes() {
	Attributes attributes;

	while (is_attr(curr_tok))
	{
		switch (curr_tok.type()) {
			case (int)TokenType::PUB:
				attributes.push_back({ TokenType::PUB, curr_tok.span() });
				break;
			case (int)TokenType::PRIV:
				attributes.push_back({ TokenType::PRIV, curr_tok.span() });
				break;
			case (int)TokenType::MUT:
				attributes.push_back({ TokenType::MUT, curr_tok.span() });
				break;
			case (int)TokenType::CONST:
				attributes.push_back({ TokenType::CONST, curr_tok.span() });
				break;

			default:
				Session::bug("attribute parser could't identify an attribute");
		}
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
void Parser::ident() {
	trace("ident: " + std::string(curr_tok.raw()));
	expect(TokenType::ID);
	end_trace();
}
// lifetime : LF
void Parser::lifetime() {
	trace("lifetime: " + std::string(curr_tok.raw()));

	auto name = { curr_tok.raw() };
	expect(TokenType::LF);

	end_trace();
}

// generic_params : '<' type_or_lt (',' type_or_lt)* '>'
void Parser::generic_params() {
	trace("generic_params");
	expect('<');

	if(curr_tok.type() != '>') {
		type_or_lt();
		while (curr_tok.type() == ',') {
			bump();
			type_or_lt();
		}
	}
	
	expect('>');
	end_trace();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////    Parse    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// file : module_decl item*
Node* Parser::parse() {
	trace("parse");

	// FIXME: Create appropriate root node type
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

	expect(TokenType::MOD);
	Path mod_path = path();
	expect(';');

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
	expect(TokenType::IMPORT);

	switch (curr_tok.type()) {
		case (int)TokenType::MOD:
			this->path();
			expect(';');
			break;
		case (int)TokenType::PACKAGE:
			this->path();
			expect(';');
			break;
		default:
			expect({TokenType::MOD, TokenType::PACKAGE});
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
	expect(TokenType::STATIC);

	auto name = curr_tok.raw();
	ident();

	expect(':');
	type();

	expect('=');
	expr();

	expect(';');
	end_trace();
}

// item_const : CONST ident ':' type '=' expr ';'
void Parser::item_const() {
	trace("item_const");
	expect(TokenType::CONST);

	auto name = curr_tok.raw();
	ident();

	expect(':');
	type();

	expect('=');
	expr();

	expect(';');
	end_trace();
}

// item_type : TYPE ident ''=' type_sum ';'
void Parser::item_type() {
	trace("item_type");
	expect(TokenType::TYPE);

	auto name = curr_tok.raw();
	ident();

	expect('=');
	type();

	expect(';');
	end_trace();
}

// view_item : USE path ';'
void Parser::view_item() {
	trace("view_item");

	expect(TokenType::USE);
	auto path = this->path();

	expect(';');
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
			expect({
				TokenType::FUN,
				TokenType::STRUCT,
				TokenType::ENUM,
				TokenType::UNION,
				TokenType::TRAIT,
				TokenType::IMPL
			});
	}

	end_trace();
}

void Parser::item_fun() {
	trace("item_fun");
	expect(TokenType::FUN);

	auto name = curr_tok.raw();
	ident();

	Session::unimpl("item_fun");

	end_trace();
}

// item_struct : STRUCT ident generic_params? ';'
//             | STRUCT ident generic_params? struct_tuple_block ';'
//             | STRUCT ident generic_params? struct_decl_block
void Parser::item_struct() {
	trace("item_struct");
	expect(TokenType::STRUCT);

	auto name = curr_tok.raw();
	ident();

	if (curr_tok.type() == '<')
		generic_params();

	if (curr_tok.type() == ';')
		bump();
	else if (curr_tok.type() == '(') {
		struct_tuple_block();
		expect(';');
	}
	else if (curr_tok.type() == '{')
		struct_decl_block();

	end_trace();
}

// struct_tuple_block : '(' struct_tuple_items? ')'
void Parser::struct_tuple_block() {
	trace("struct_tuple_block");
	expect('(');

	if (curr_tok.type() != ')')
		struct_tuple_items();

	expect(')');
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
	expect('{');

	if (curr_tok.type() != '}')
		struct_decl_items();

	expect('}');
	end_trace();
}

// struct_decl_items : struct_decl_item*
void Parser::struct_decl_items() {
	trace("struct_decl_items");

	struct_decl_item();
	while (curr_tok.type() != '}') {
		struct_decl_item();
	}

	end_trace();
}

// struct_decl_item : attributes? ident ':' type_with_lt ';' 
void Parser::struct_decl_item() {
	trace("struct_decl_item");

	Attributes attr = attributes();

	auto name = curr_tok.raw();
	expect(TokenType::ID);

	expect(':');
	type_with_lt();

	expect(';');
	end_trace();
}

// item_enum : attributes? ENUM ident generic_params? ';'
//           | attributes? ENUM ident generic_params? enum_defs
void Parser::item_enum() {
	trace("item_enum");
	expect(TokenType::ENUM);

	auto name = curr_tok.raw();
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
	expect('{');

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

	expect('}');
	end_trace();
}

void Parser::enum_def() {
	trace("enum_def");

	auto name = curr_tok.raw();
	ident();

	if (curr_tok.type() == '=') {
		bump();
		expr();
	}
	else if (curr_tok.type() == '(') {
		bump();
		Session::unimpl("enum unions");
	}

	end_trace();
}

void Parser::item_union() {
	trace("item_union");
	expect(TokenType::UNION);

	auto name = curr_tok.raw();
	ident();

	Session::unimpl("item_union");

	end_trace();
}

void Parser::item_trait() {
	trace("item_trait");
	expect(TokenType::TRAIT);

	auto name = curr_tok.raw();
	ident();

	Session::unimpl("item_trait");

	end_trace();
}

void Parser::item_impl() {
	trace("item_impl");
	expect(TokenType::IMPL);

	Session::unimpl("item_impl");

	end_trace();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////    Expr    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void Parser::expr() {
	trace("expr");

	Session::unimpl("expressions");

	end_trace();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////    Type    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/* True if the provided token is a primitive. */
inline bool is_primitive(const Token& tk) {
	return tk == TokenType::THING || tk == TokenType::STR || tk == TokenType::CHAR ||
		tk == TokenType::INT ||
		tk == TokenType::I64 || tk == TokenType::I32 || tk == TokenType::I16 || tk == TokenType::I8 ||
		tk == TokenType::UINT ||
		tk == TokenType::U64 || tk == TokenType::U32 || tk == TokenType::U16 || tk == TokenType::U8;
}

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
		// referance type
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
				expect(']');
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
				expect(')');
			}
			break;

		// custom type
		case static_cast<int>(TokenType::ID):
			//std::string type_name = curr_tok.raw();
			ident();							// ident
			break;

		// primitive type
		default:
			if (!is_primitive(curr_tok))
				expected_msg("a type");
			primitive();
	}
	
	end_trace();
}

// type_or_lt : type
//            | lifetime
void Parser::type_or_lt() {
	trace("type_or_lt");

	if (is_lifetime(curr_tok)) lifetime();
	else if (is_type(curr_tok)) type();
	else err("expected type or lifetime, found " + translate::tk_type(curr_tok));

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
	trace("primitive: " + curr_tok.raw());

	expect({ TokenType::THING, TokenType::STR, TokenType::CHAR,
		TokenType::INT, TokenType::I64, TokenType::I32, TokenType::I16, TokenType::I8,
		TokenType::UINT, TokenType::U64, TokenType::U32, TokenType::U16, TokenType::U8 });

	end_trace();
}