#include "lexer.hpp"
#include "driver/session.hpp"
#include "util/ranges.hpp"
#include "util/token_info.hpp"
#include <iostream>

/* A single keyword.
 * Key - string
 * Value - int (TokenType). */
struct Keyword {
	const char* key;
	int value;
};
/* A map of keywords.
 * Matches a string to a keyword type. */
constexpr static const Keyword keywords[] = {
	{ "thing",		(int)TokenType::THING },
	{ "str",		(int)TokenType::STR },
	{ "char",		(int)TokenType::CHAR },
	{ "int",		(int)TokenType::INT },
	{ "i8",			(int)TokenType::I8 },
	{ "i16", 		(int)TokenType::I16 },
	{ "i32",		(int)TokenType::I32 },
	{ "i64",		(int)TokenType::I64 },
	{ "uint",		(int)TokenType::UINT },
	{ "u8",			(int)TokenType::U8 },
	{ "u16",		(int)TokenType::U16 },
	{ "u32",		(int)TokenType::U32 },
	{ "u64",		(int)TokenType::U64 },
	{ "float",		(int)TokenType::FLOAT },
	{ "f32",		(int)TokenType::F32 },
	{ "f64",		(int)TokenType::F64 },
	{ "package",	(int)TokenType::PACKAGE },
	{ "mod",		(int)TokenType::MOD },
	{ "use",		(int)TokenType::USE },
	{ "import",		(int)TokenType::IMPORT },
	{ "export",		(int)TokenType::EXPORT },
	{ "var",		(int)TokenType::VAR },
	{ "fun",		(int)TokenType::FUN },
	{ "struct",		(int)TokenType::STRUCT },
	{ "enum",		(int)TokenType::ENUM },
	{ "union",		(int)TokenType::UNION },
	{ "macro",		(int)TokenType::MACRO },
	{ "impl",		(int)TokenType::IMPL },
	{ "const",		(int)TokenType::CONST },
	{ "static",		(int)TokenType::STATIC },
	{ "type",		(int)TokenType::TYPE },
	{ "loop",		(int)TokenType::LOOP },
	{ "while",		(int)TokenType::WHILE },
	{ "do",			(int)TokenType::DO },
	{ "for",		(int)TokenType::FOR },
	{ "in",			(int)TokenType::IN },
	{ "match",		(int)TokenType::MATCH },
	{ "switch",		(int)TokenType::SWITCH },
	{ "case",		(int)TokenType::CASE },
	{ "where",		(int)TokenType::WHERE },
	{ "return",		(int)TokenType::RETURN },
	{ "pub",		(int)TokenType::PUB },
	{ "priv",		(int)TokenType::PRIV },
	{ "mut",		(int)TokenType::MUT }
};
/* Attempts to find the given key in the keyword array.
 * If no keywords match, a nullptr is returned. */
constexpr const Keyword* key_find(const char* key) {
	for (unsigned long i = 0; i < sizeof(keywords)/sizeof(*keywords); i++)
		if (strcmp(keywords[i].key, key) == 0)
			return &keywords[i];
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////      SourceReader      ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

char SourceReader::read_next_char() {
	// If the current index is out of bounds, return '\0'
 	if (index >= src.length())
		return '\0';

	// Return the next character
	return src[index];
}

void SourceReader::bump(int n) {
	for (int i = 0; i < n; i++) {
		// Move the next character up
		curr = next;

		// Increment reading position
		if (curr == '\n') {
			curr_col = 0;
			curr_ln++;
		} else {
			curr_col++;
		}

		// Read the next character
		next = read_next_char();
		
		if (curr != '\0')
			index++;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////      Helpers      /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/*	Returns true if the input token's type corresponds to EOF. */
inline bool is_valid(Token tk) { return tk.type() != (int)TokenType::END; }
/*	Returns true if the input value corresponds to EOF. */
inline bool is_valid(TokenType tk) { return tk != TokenType::END; }
/*	Returns true if the input value corresponds to EOF. */
inline bool is_valid(int tk) { return tk != (int)TokenType::END; }

void Lexer::consume_ws_and_comments() {
	// Eat all whitespace
	while (range::is_whitespace(curr))
		bump();

	// Eat comments
	if (curr == '/') {
		if (next == '/') {
			save_curr_pos();

			// Eat comment until line ends or EOF
			while (curr != '\n' && curr != '\0') 
				bump();
			// Go back and check for whitespace again
			consume_ws_and_comments();
		}
		else if (next == '*') {
			save_curr_pos();

			bump(2);
			// Eat block comment until closed
			while (true) {
				// Throw an error if the block is not terminated at the end of the file
				if (curr == '\0') {
					// This is a critiacal 
					Session::span_err("unterminated block comment", curr_span());
					std::exit(6);
				}
				if (curr == '*' && next == '/') { 
					bump(2);
					break;
				}
				bump();
			}
		}
		consume_ws_and_comments();
	}
}

bool Lexer::scan_hex_escape(uint num, char delim) {
	bool valid = true;
	uint number = 0;

	// Check 'num' amount of characters
	for (uint i = 0; i < num; i++) {
		// File EOF in escape
		if (!is_valid(curr)) {
			// Citical failure
			// We can't guess how the literal was supposed to be terminated
			Session::span_err("numeric escape is too short", curr_span());
			std::exit(5);
		}
		// Escape is shorter than expected
		if (curr == delim) {
			Session::span_err("numeric escape is too short", curr_span());
			valid = false;
			break;
		}
		auto n = range::get_num(curr, 16);
		number *= 16;
		if (n.has_value())
			number += n.value();
		else {
			valid = false;
			Session::span_err("invalid character in numeric escape: " + std::string{curr}, curr_span());
		}
		bump();
	}

	if (range::is_char(number)) return valid;
	else return false;
}

void Lexer::scan_exponent() {
	if (curr == 'e' || curr == 'E') {
		bump();

		if (curr == '-' || curr == '+')
			bump();

		size_t digit_start = bitpos();
		scan_digits(10, 10);
		if (bitpos() == digit_start) {
			Session::err("exponent expects at least one digit");
			std::exit(7);
		}
	}
}

void Lexer::scan_digits(int base, int full_base) {
	assert(full_base <= 36);
	assert(base <= full_base);

	while (true) {
		if (range::get_num(curr, full_base).has_value()) {
			if (!range::get_num(curr, base).has_value()) {
				bump();
				Session::span_err("invalid digit in base " + std::to_string(base) + " literal", curr_span());
			}
			bump();
		}
		else return;
	}
}

Token Lexer::lex_number() {
	int base = 10;
	size_t start = bitpos();

	// Possible binary, ocatal or hex numbers
	if (curr == 0) {
		bump();
		switch (curr) {
			case 'b':
				bump();
				base = 2;
				scan_digits(2, 10);
				break;
			case 'o':
				bump();
				base = 8;
				scan_digits(8, 10);
				break;
			case 'x':
				bump();
				base = 16;
				scan_digits(16, 16);
				break;
			default:
				if (range::is_dec(curr) || curr == '.' || curr == 'e' || curr == 'E') {
					scan_digits(10, 10);
				}
				else return Token(TokenType::LIT_INTEGER, curr_src_view(), curr_span());
		}
	}
	// Only decimal
	else if (range::is_dec(curr))
		scan_digits(10, 10);

	// If there is a dot, it could be a float,
	// but it could also be a range or followed by a function call
	// '3.1415' or '0..9' or '42.foo()'
	if (curr == '.' && range::is_dec(next)) {
		scan_digits(10, 10);
		scan_exponent();
		if (base != 10) {
			Session::span_err("only decimal float literals are supported", curr_span());
		}
	}

	/* There were no numbers */
	if (bitpos() == start) {
		Session::span_err("no valid numbers", curr_span());
		return Token(TokenType::LIT_INTEGER, "0", curr_span());
	}

	if (curr == 'e' || curr == 'E') {
		scan_exponent();
		if (base != 10) {
			Session::span_err("exponent only supported for decimal numbers", curr_span());
		}
	}

	return Token(TokenType::LIT_INTEGER, curr_src_view(), curr_span());
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////      Lexer      //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

Token Lexer::next_token() {
	// Get rid of whitespace and comments
	consume_ws_and_comments();

	// If the current character is EOF, return an END token
	// It's span is a singel position, since the file ends there
	if (!is_valid(curr))
		return Token(TokenType::END, "\\0", Span(trans_unit(), bitpos(), lineno(), colno(), bitpos(), lineno(), colno()));

	save_curr_pos();

	return next_token_inner();
}

Token Lexer::next_token_inner() {
	// If it starts like an identifier,
	// it's either an identifier or a keyword
	if (range::is_ident_start(curr)) {
		// String for building words
		std::string build_str;

		// Build string as long as it could make an identifier
		do {
			build_str += curr;
			bump();
		} while (range::is_ident_cont(curr));

		// If only underscore, return
		if (build_str == "_") return Token('_', curr_src_view(), curr_span());

		// Check if the identifier is a keyword
		// If it is, return a keyword token
		auto item = key_find(build_str.c_str());
		if (item != nullptr)
			return Token(item->value, curr_src_view(), curr_span());

		// Return string as an identifier
		return Token(TokenType::ID, curr_src_view(), curr_span());
	}

	// If decimal, build number
	if (range::is_dec(curr))
		return lex_number();

	// Check for symbol tokens
	switch (curr) {
		case ',': bump(); return Token(',', curr_src_view(), curr_span());
		case ';': bump(); return Token(';', curr_src_view(), curr_span());
		case '?': bump(); return Token('?', curr_src_view(), curr_span());
		case '(': bump(); return Token('(', curr_src_view(), curr_span());
		case ')': bump(); return Token(')', curr_src_view(), curr_span());
		case '{': bump(); return Token('{', curr_src_view(), curr_span());
		case '}': bump(); return Token('}', curr_src_view(), curr_span());
		case '[': bump(); return Token('[', curr_src_view(), curr_span());
		case ']': bump(); return Token(']', curr_src_view(), curr_span());
		case '~': bump(); return Token('~', curr_src_view(), curr_span());
		case '#': bump(); return Token('#', curr_src_view(), curr_span());
		case '@': bump(); return Token('@', curr_src_view(), curr_span());

		// Very messy cases
		// Refer to the comments on the side
		case '.': 
			if (next == '.') {
				bump(2);
				if (curr == '.') {
					bump();
					return Token(TokenType::DOTDOTDOT, curr_src_view(), curr_span());		// ...
				}
				return Token(TokenType::DOTDOT, curr_src_view(), curr_span());				// ..
			}
			else if (!range::is_dec(next)) {
				bump();
				return Token('.', curr_span());												// .
			}
			return lex_number();															// Number

		case ':':
			if (next == ':') {
				bump(2);
				return Token(TokenType::SCOPE, curr_src_view(), curr_span());				// ::
			}
			bump();
			return Token(':', curr_span());													// :
	
		case '=':
			if (next == '=') {
				bump(2);
				return Token(TokenType::EQEQ, curr_src_view(), curr_span());				// ==
			}
			else if (next == '>') {
				bump(2);
				return Token(TokenType::FATARROW, curr_src_view(), curr_span());			// =>
			}
			bump();
			return Token('=', curr_span());													// =

		case '!':
			if (next == '=') {
				bump(2);
				return Token(TokenType::NE, curr_src_view(), curr_span());					// !=
			}
			bump();
			return Token('!', curr_span());													// !

		case '+':
			if (next == '+') {
				bump(2);
				return Token(TokenType::PLUSPLUS, curr_src_view(), curr_span());			// ++
			}
			else if (next == '=') {
				bump(2);
				return Token(TokenType::SUME, curr_src_view(), curr_span());				// +=
			}
			bump();
			return Token('+', curr_span());													// +

		case '-':
			if (next == '-') {
				bump(2);
				return Token(TokenType::MINUSMINUS, curr_src_view(), curr_span());			// --
			}
			else if (next == '>') {
				bump(2);
				return Token(TokenType::RARROW, curr_src_view(), curr_span());				// ->
			}
			else if (next == '=') {
				bump(2);
				return Token(TokenType::SUBE, curr_src_view(), curr_span());				// -=
			}
			bump();
			return Token('-', curr_span());													// -

		case '*':
			if (next == '=') {
				bump(2);
				return Token(TokenType::MULE, curr_src_view(), curr_span());				// *=
			}
			bump();
			return Token('*', curr_span());													// *

		case '/':
			if (next == '=') {
				bump(2);
				return Token(TokenType::DIVE, curr_src_view(), curr_span());				// /=
			}
			bump();
			return Token('/', curr_span());													// /

		case '%':
			if (next == '=') {
				bump(2);
				return Token(TokenType::MODE, curr_src_view(), curr_span());				// %=
			}
			bump();
			return Token('%', curr_span());													// %

		case '^':
			if (next == '=') {
				bump(2);
				return Token(TokenType::CARE, curr_src_view(), curr_span());				// ^=
			}
			bump();
			return Token('^', curr_span());													// ^

		case '&':
			//if (next == '&') {
			//	bump(2);
			//	return Token(TokenType::AND, curr_src_view(), curr_span());					// &&
			//}
			if (next == '=') {
				bump(2);
				return Token(TokenType::ANDE, curr_src_view(), curr_span());				// &=
			}
			bump();
			return Token('&', curr_span());													// &

		case '|':
			if (next == '|') {
				bump(2);
				return Token(TokenType::OR, curr_src_view(), curr_span());					// ||
			}
			else if (next == '=') {
				bump(2);
				return Token(TokenType::ORE, curr_src_view(), curr_span());					// |=
			}
			bump();
			return Token('|', curr_span());													// |

		case '>':
			if (next == '=') {
				bump(2);
				return Token(TokenType::GE, curr_src_view(), curr_span());					// >=
			}
			//else if (next == '>') {
			//	bump(2);
			//	return Token(TokenType::SHR, curr_src_view(), curr_span());					// >>
			//}
			bump(1);
			return Token('>', curr_span());													// >
			
		case '<':
			switch (next) {
				case '=':
					bump(2);
					return Token(TokenType::LE, curr_src_view(), curr_span());				// <=

				case '-':
					bump(2);
					if (curr == '>') { 
						bump();
						return Token(TokenType::DARROW, curr_src_view(), curr_span());		// <->
					}
					return Token(TokenType::LARROW, curr_src_view(), curr_span());			// <-

				case '<':
					bump(2);
					return Token(TokenType::SHL, curr_src_view(), curr_span());				// <<

				default:
					bump();
					return Token('<', curr_span());											// <
			}

		case '\'':
		{
			bump();
			char c = curr;
			bool valid = true;
			size_t start = bitpos();
			bump();

			// Character literal is empty
			if (c == '\'') {
				valid = false;
				Session::span_err("character must have a value", curr_span());
			}
			else {
				// If it starts like an indentifier and doesnt close,
				// assume it's a lifetime
				if (range::is_ident_start(c) && curr != '\'') {

					// Collect lifetime name
					while (range::is_ident_cont(curr)) {
						bump();
					}

					// A lifetime should't end with a '
					// In that case we assume it's an unterminated character literal
					if (curr == '\'') {
						bump();
						Session::span_err("character literal may contain only one symbol", curr_span());
						Session::warn("if you meant to create a string literal, use double quotes");
						return Token(TokenType::LIT_STRING, src.substr(start, bitpos() - start - 1), curr_span());
					}

					return Token(TokenType::LF, curr_src_view(), curr_span());
				}

				// Newlines and tabs aren't allowed inside characters
				if ((c == '\n' || c == '\r' || c == '\t') && curr == '\'') {
					valid = false;
					Session::span_err("special characters need to be escaped", curr_span());
				}

				// The character is an escape sequence
				if (c == '\\') {
					switch (curr) {
						case 'n': bump(); break;
						case 'r': bump(); break;
						case 't': bump(); break;
						case '\\': bump(); break;
						case '\'': bump(); break;
						case 'x': bump(); valid &= scan_hex_escape(2, '\''); break;
						case 'u': bump(); valid &= scan_hex_escape(4, '\''); break;
						case 'U': bump(); valid &= scan_hex_escape(8, '\''); break;
						default:
							valid = false;
							Session::span_err("unknown escape sequence: '\\" + std::string(1, curr) + "'", curr_span());
							bump(); 
					}
				}

				// The character hasn't ended after one symbol
				if (curr != '\'') {
					while (true) {
						bump();
						// There are more than one symbols in the character literal
						// Return it as a string literal
						if (curr == '\'') {
							bump();
							Session::span_err("character literal may contain only one symbol", curr_span());
							Session::warn("if you wanted a string literal, use double quotes");
							return Token(TokenType::LIT_STRING, src.substr(start, bitpos() - start - 1), curr_span());
						}
						// The character literal goes to EOF or newline
						if (!is_valid(curr) || curr == '\n') {
							// This is a critical failure
							// We can't expect what to do with the missing quote
							err("character literal missing end quote", curr_span());
						}
					}
				}
			}

			// Invalid characters are set to '0'
			std::string_view ret = valid ? src.substr(start, bitpos() - start) : std::string_view("0");

			bump(); // move off end quote
			return Token(TokenType::LIT_CHAR, ret, curr_span());
		}

		case '"':
		{
			bump();
			bool valid = true;
			size_t start = bitpos();

			while (curr != '"') {
				if (curr == (int)TokenType::END) {
					// This is a critical failure
					// We can't know where the end quote was supposed to be
					err("string literal missing end quote", curr_span());
				}

				char c = curr;
				bump();

				if (c == '\\') {
					switch (curr) {
						case 'n': bump(); break;
						case 'r': bump(); break;
						case 't': bump(); break;
						case '\\': bump(); break;
						case '\'': bump(); break;
						case 'x': bump(); valid &= scan_hex_escape(2, '\"'); break;
						case 'u': bump(); valid &= scan_hex_escape(4, '\"'); break;
						case 'U': bump(); valid &= scan_hex_escape(8, '\"'); break;
						default:
							valid = false;
							Session::span_err("unknown escape sequence: '\\" + std::string(1, curr) + "'", curr_span());
							bump(); 
					}
				}
			}

			// Invalid strings are set to "??"
			std::string_view ret = valid ? src.substr(start, bitpos() - start) : std::string_view("??");

			bump(); // move off end quote
			return Token(TokenType::LIT_STRING, ret, curr_span());
		}

		default:
			Session::span_err("unrecognised character: " + std::string(1, curr), curr_span());
			bump();
			return Token(TokenType::UNKNOWN, curr_src_view(), curr_span());
	}
}