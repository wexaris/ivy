#include "lexer.hpp"
#include "source/source_map.hpp"
#include "driver/session.hpp"
#include "token/token_translate.hpp"
#include "util/ranges.hpp"
#include <iostream>

/* A single keyword.
 * Key - string
 * Value - int (TokenType). */
struct Keyword {
	const char* key;
	int value;
};
/* A map of keywords. */
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

constexpr const Keyword* key_find(const char* key) {
	for (unsigned long i = 0; i < sizeof(keywords)/sizeof(*keywords); i++)
		if (strcmp(keywords[i].key, key) == 0)
			return &keywords[i];
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////      SourceReader      ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void SourceReader::reset(TranslationUnit* tu) {
	// Set internal translation unit pointer to the new one
	translation_unit = tu;
	tu->source();

	// Set the position tracking to the beginning of the file
	index = SIZE_MAX;
	curr_ln = 1;
	curr_col = 1;

	// Set the next character and bump it to the current one
	// Bump because that updates the newline indexes in the Translation Unit
	// Reset the column counter for it to correspond to the character 'curr'
	next = read_next_char();
	bump();
}

char SourceReader::read_next_char() {
	// If the current index is out of bounds, return '\0'
	if (index >= src.length())
		return '\0';

	// Return the next character
	return src[index++];
}

void SourceReader::bump(int n) {
	for (int i = 0; i < n; i++) {
		// Move the next character up
		curr = next;

		// Increment reading position
		if (curr == '\n') { 
			translation_unit->save_newline(static_cast<int>(bitpos()));
			curr_col = 0;
			curr_ln++;
		} else {
			curr_col++;
		}

		// Read the next character
		next = read_next_char();
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////      Helpers      /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/*	Check if the value is whitespace. */
inline bool is_whitespace(char c) {
	return 	c == ' '	||
			c == '\t'	||
			c == '\r'	||
			c == '\n';
}

/*	Returns true if the input token's type corresponds to EOF. */
bool is_valid(Token tk) { return tk != TokenType::END; }
/*	Returns true if the input value corresponds to EOF. */
bool is_valid(TokenType tk) { return tk != TokenType::END; }

/*	Loops through whitespace and comments.
	Works by bumping until the current character isn't a whitespace or comment. */
void eat_ws_and_comments(Lexer* lex) {
	// Eat all whitespace
	while (is_whitespace(lex->curr_c()))
		lex->bump();

	// Eat comments
	if (lex->curr_c() == '/') {
		if (lex->next_c() == '/') {
			// Eat comment until line ends or EOF
			while (lex->curr_c() != '\n' && lex->curr_c() != '\0') 
				lex->bump();
			// Go back and check for whitespace again
			eat_ws_and_comments(lex);
		}
		else if (lex->next_c() == '*') {
			lex->bump(2);
			// Eat block comment until closed
			while (true) {
				// Throw an error if the block is not terminated at the end of the file
				if (lex->curr_c() == '\0') { 
					Span sp(lex->trans_unit(), lex->bitpos()-1, lex->lineno(), lex->colno()-1, lex->bitpos(), lex->lineno(), lex->colno());
					lex->err("unterminated block comment at end of file", sp);
				}
				if (lex->curr_c() == '*' && lex->next_c() == '/') { 
					lex->bump(2);
					break;
				}
				lex->bump();
			}
		}
		eat_ws_and_comments(lex);
	}
}

/*	Scans and identifies numeric escape characters. */
char scan_num_escape(Lexer* lex, int digit_num) {
	// Number to build
	int build_num = 0;
	// Loop until requested number of digits
	for (; digit_num != 0; digit_num--) {
		lex->bump();
		// Throw an exception if the character is not hex
		if (!range::is_hex(lex->curr_c())) throw std::runtime_error("Unknown numeric escape sequence : \\" + std::to_string(lex->curr_c()));
		build_num *= 16;
		build_num += range::val_hex(lex->curr_c(), lex);
	}
	return (char)build_num;
}

/*	Reads and extracts a number literal. */
std::string read_number(Lexer* lex) {
	std::string num;
	auto c = lex->curr_c();
	while (!is_whitespace(c) && c != ';') {
		num += c;
		lex->bump();
		c = lex->curr_c();
	}
	return num;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////      Lexer      //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

Token Lexer::next_token() {
	// Get rid of whitespace and comments
	eat_ws_and_comments(this);

	// Save file positions for token span
	curr_start.abs = bitpos();
	curr_start.ln = lineno();
	curr_start.col = colno();

	// If the current character is valid, build the next token
	// If not, make an EOF/END token
	return is_valid((TokenType)curr) ? next_token_inner() : 
		Token(TokenType::END, "\\0",
			Span(translation_unit, curr_start.abs, curr_start.ln, curr_start.col, bitpos(), lineno(), colno())
		);
	// Set the token's source, position and length
	//ret.set_span(Span(translation_unit, curr_start.abs, curr_start.ln, curr_start.col, bitpos(), lineno(), colno()));

	//return ret;
}

Token Lexer::next_token_inner() {
	// If alphanumeric, build a string, check for it being a keyword
	if (range::is_alpha(curr) || curr == '_') {
		// String for building words
		std::string build_str;

		// Build string as long as alphanumeric or underscore
		do {
			build_str += curr;
			bump();
		} while (range::is_alnum(curr) || curr == '_');

		// If only underscore, return
		if (build_str == "_") return Token('_', curr_span());

		// Check if the identifier is a keyword
		// If it is, return a keyword token
		auto item = key_find(build_str.c_str());
		if (item != nullptr)
			return Token(item->value, build_str, curr_span());

		// Return string as identifier
		return Token(TokenType::ID, build_str, curr_span());
	}

	// If decimal, build number
	if (range::is_dec(curr))
		return Token(TokenType::LIT_NUMBER, read_number(this), curr_span());

	// Check for symbol tokens
	switch (curr) {
		case ',': bump(); return Token(',', curr_span());
		case ';': bump(); return Token(';', curr_span());
		case '?': bump(); return Token('?', curr_span());
		case '(': bump(); return Token('(', curr_span());
		case ')': bump(); return Token(')', curr_span());
		case '{': bump(); return Token('{', curr_span());
		case '}': bump(); return Token('}', curr_span());
		case '[': bump(); return Token('[', curr_span());
		case ']': bump(); return Token(']', curr_span());
		case '~': bump(); return Token('~', curr_span());
		case '#': bump(); return Token('#', curr_span());
		case '@': bump(); return Token('@', curr_span());

		case '.': 
			if (next == '.') {
				bump(2);
				if (curr == '.') {
					bump(); return Token(TokenType::DOTDOTDOT, "...", curr_span()); }			// ...
				return Token(TokenType::DOTDOT, "..", curr_span());								// ..
			}
			if (!range::is_dec(next)) {
				bump(); return Token('.', curr_span());											// .
			}
			bump();
			return Token(TokenType::LIT_NUMBER, "." + read_number(this), curr_span());			// LIT_NUMBER

		case ':':
			if (next == ':') { bump(2); return Token(TokenType::SCOPE, "::", curr_span()); }	// ::
			bump(); return Token(':', curr_span());												// :
	
		case '=':
			if (next == '=') { bump(2); return Token(TokenType::EQEQ, "==", curr_span()); }			// ==
			if (next == '>') { bump(2); return Token(TokenType::FATARROW, "=>", curr_span());	}	// =>
			bump(); return Token('=', curr_span());													// =

		case '!':
			if (next == '=') { bump(2); return Token(TokenType::NE, "!=", curr_span()); }		// !=
			bump(); return Token('!', curr_span());												// !

		case '+':
			if (next == '+') { bump(2); return Token(TokenType::PLUSPLUS, "++", curr_span()); }		// ++
			if (next == '=') { bump(2); return Token(TokenType::SUME, "+=", curr_span()); }			// +=
			bump(); return Token('+', curr_span());													// +

		case '-':
			if (next == '-') { bump(2); return Token(TokenType::MINUSMINUS, "--", curr_span()); }	// --
			if (next == '>') { bump(2); return Token(TokenType::RARROW, "->", curr_span()); }		// ->
			if (next == '=') { bump(2); return Token(TokenType::SUBE, "-=", curr_span()); }			// -=
			bump(); return Token('-', curr_span());													// -

		case '*':
			if (next == '=') { bump(2); return Token(TokenType::MULE, "*=", curr_span()); }		// *=
			bump(); return Token('*', curr_span());												// *

		case '/':
			if (next == '=') { bump(2); return Token(TokenType::DIVE, "/=", curr_span()); }		// /=
			bump(); return Token('/', curr_span());												// /

		case '%':
			if (next == '=') { bump(2); return Token(TokenType::MODE, "%=", curr_span()); }		// %=
			bump(); return Token('%', curr_span());												// %

		case '^':
			if (next == '=') { bump(2); return Token(TokenType::CARE, "^=", curr_span()); }		// ^=
			bump(); return Token('^', curr_span());												// ^

		case '&':
			//if (next == '&') { bump(2); return Token(TokenType::AND, "&&", curr_span()); }		// &&
			if (next == '=') { bump(2); return Token(TokenType::ANDE, "&=", curr_span());	}		// &=
			bump(); return Token('&', curr_span());													// &

		case '|':
			if (next == '|') { bump(2); return Token(TokenType::OR, "||", curr_span()); }		// ||
			if (next == '=') { bump(2); return Token(TokenType::ORE, "|=", curr_span()); }		// |=
			bump(); return Token('|', curr_span());												// |

		case '>':
			switch (next) {
				case '=': bump(2); return Token(TokenType::GE, ">=", curr_span());			// >=
				//case '>': bump(2); return Token(TokenType::SHR, ">>", curr_span());		// >>
				default:  bump(2); return Token('>', curr_span());							// >
			}
			
		case '<':
			switch (next) {
				case '=': bump(2); return Token(TokenType::LE, "<=", curr_span());		// <=
				case '-': bump(2);
					if (curr == '>') { 
						bump(); return Token(TokenType::DARROW, "<->", curr_span());	// <->
					}
					return Token(TokenType::LARROW, "<-", curr_span());					// <-
				case '<': bump(2); return Token(TokenType::SHL, "<<", curr_span());		// <<
				default:  bump(); return Token('<', curr_span());						// <
			}

		case '\'':
		{
			// Save next character, so that we can stay on the character after the next one
			char c = next;
			bump(2);

			// Handle character escape sequences
			if (c == '\\') {
				switch (curr) {
					case 'n': c = '\n'; break;
					case 'r': c = '\r'; break;
					case 't': c = '\t'; break;
					case '\\': c = '\\'; break;
					case '\'': c = '\''; break;
					case 'x': c = scan_num_escape(this, 2); break;
					case 'u': c = scan_num_escape(this, 4); break;
					case 'U': c = scan_num_escape(this, 8); break;
					default:
						err("unknown character escape sequence: \\" + std::to_string(curr),
							Span(translation_unit,
								bitpos() - 1,
								lineno(),
								colno() - 1,
								bitpos(),
								lineno(),
								colno()
							)
						);
				}
				// Throw an exception if the character is not terminated
				bump();
				if (curr != '\'') {
					err("unterminated character literal", curr_span());
				}
				bump();
				return Token(TokenType::LIT_INT, std::to_string(c), curr_span());
			}
			// If the character is in quotes or is a number, it's a literal
			if (range::is_dec(c) || curr == '\'') {
				// Throw an exception if the character is not terminated
				if (curr != '\'') {
					err("unterminated character literal", curr_span());
				}
				bump();
				return Token(TokenType::LIT_INT, std::to_string(c), curr_span());
			}

			// If the first character isn't alpha or an underscore, it can't be a lifetime
			// We presume it's an unterminated character literal
			if (!range::is_alpha(c) && c != '_') {
				err("unterminated character literal or invalid lifetime", curr_span());
			}

			// If the character isn't in quotes and isn't a number, it's a lifetime
			std::string build_str = std::string(1, c);
			while (range::is_alnum(curr) || curr == '_') {
				build_str += curr;
				bump();
			}
			// Since it's a lifetime, check if it's static
			if (build_str == "static")
				return Token(TokenType::STATIC_LF, build_str, curr_span());
			return Token(TokenType::LF, build_str, curr_span());
		}

		case '"':
		{
			// String for building words
			std::string build_str;

			bump();
			while (curr != '"') {
				// Throw an exception if the string is not terminated
				if (curr == (int)TokenType::END)
					err("unterminated string literal", curr_span());

				// Save character, so that we stay on 'curr'
				char c = curr;
				bump();

				// Handle string escape sequences
				if (c == '\\') {
					switch (curr) {
						case 'n': build_str += '\n'; break;
						case 'r': build_str += '\n'; break;
						case 't': build_str += '\n'; break;
						case '\\': build_str += '\n'; break;
						case '"': build_str += '\n'; break;
						case 'x': build_str += scan_num_escape(this, 2); break;
						case 'u': build_str += scan_num_escape(this, 4); break;
						case 'U': build_str += scan_num_escape(this, 8); break;
						case '\n': while (is_whitespace(curr) && curr != (int)TokenType::END) bump(); break;
						default:
							err("unknown string escape sequence: \\" + std::to_string(curr), curr_span());
					}
				}
				// Build string if character is not an escape sequence
				else build_str += curr;
				bump();
			}
			return Token(TokenType::LIT_STRING, build_str, curr_span());
		}

		default:
			err("unrecognised character: " + std::to_string(curr), curr_span());
			return Token(TokenType::UNKNOWN, std::to_string(curr), curr_span());
	}
}