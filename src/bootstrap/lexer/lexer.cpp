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

/*	Returns true if the input token's type corresponds to EOF. */
bool is_valid(Token tk) { return tk.type() != (int)TokenType::END; }
/*	Returns true if the input value corresponds to EOF. */
bool is_valid(TokenType tk) { return tk != TokenType::END; }
/*	Returns true if the input value corresponds to EOF. */
bool is_valid(int tk) { return tk != (int)TokenType::END; }

/*	Loops through whitespace and comments.
	Works by bumping until the current character isn't a whitespace or comment. */
void eat_ws_and_comments(SourceReader* lex) {
	// Eat all whitespace
	while (range::is_whitespace(lex->curr_c()))
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
					Session::span_err("unterminated block comment at end of file", sp);
					std::exit(6);
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
char scan_num_escape(SourceReader* lex, int digit_num) {
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
std::string read_number(SourceReader* lex) {
	std::string num;
	auto c = lex->curr_c();
	while (!range::is_whitespace(c) && c != ';') {
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

	// If the current character is EOF, return an END token
	// It's span is a singel position, since the file ends there
	if (!is_valid(curr))
		return Token(TokenType::END, "\\0", Span(trans_unit(), bitpos(), lineno(), colno(), bitpos(), lineno(), colno()));

	// Save file positions for token span
	curr_start.abs = bitpos();
	curr_start.ln = lineno();
	curr_start.col = colno();

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
		if (build_str == "_") return Token('_', curr_span());

		// Check if the identifier is a keyword
		// If it is, return a keyword token
		auto item = key_find(build_str.c_str());
		if (item != nullptr)
			return Token(item->value, build_str, curr_span());

		// Return string as ab identifier
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

		// Very messy cases
		// Refer to the comments on the side
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
			// Save next character
			char c = next;
			bool valid = true;
			bump(2);

			// Character literal is empty
			if (c == '\'') {
				valid = false;
				Session::span_err("character must have a value", curr_span());
			}

			// If it starts like an indentifier and doesnt close,
			// assume it's a lifetime
			if (range::is_ident_start(c) || curr != '\'') {
				std::string lf_str = std::string(1, c);

				// Collect lifetime name
				while (range::is_ident_cont(curr)) {
					lf_str += curr;
					bump();
				}

				// A lifetime should't end with a '
				// In that case we assume it's an unterminated character literal
				if (curr == '\'') {
					bump();
					Session::span_err("character literal may contain only one symbol", curr_span());
					Session::warn("if you meant to create a string literal, use double quotes");
					return Token(TokenType::LIT_STRING, "??", curr_span());
				}

				return Token(TokenType::LF, lf_str, curr_span());
			}

			// Newlines and tabs aren't allowed inside characters
			if ((c == '\n' || c == '\r' || c == '\t') && curr == '\'') {
				valid = false;
				Session::span_err("special characters need to be escaped", curr_span());
			}

			// The character is an escape sequence
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
						valid = false;
						Session::span_err("unknown escape sequence: '\\" + std::string(1, curr) + "'", curr_span());
				}
				bump();
			}

			// The character hasn't ended after one symbol
			if (curr != '\'') {
				std::string ch_str = std::string{ c, curr };

				while (true) {
					bump();
					ch_str += curr;
					// There are more than one symbols in the character literal
					// Return it as a string literal
					if (curr == '\'') {
						bump();
						Session::span_err("character literal may contain only one symbol", curr_span());
						Session::warn("if you wanted a string literal, use double quotes");
						return Token(TokenType::LIT_STRING, ch_str, curr_span());
					}
					// The character literal goes to EOF or newline
					if (!is_valid(curr) || curr == '\n') {
						// This is a critical failure
						// We can't expect what to do with the missing quote
						err("character literal missing end quote", curr_span());
					}
				}
			}
			bump(); // move off end quote

			// Invalid characters are set to '0'
			if (!valid) c = '0';

			return Token(TokenType::LIT_CHAR, std::string(1, c), curr_span());
		}

		case '"':
		{
			bump();
			bool valid = true;
			std::string build_str;

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
						case 'n': c = '\n'; break;
						case 'r': c = '\r'; break;
						case 't': c = '\t'; break;
						case '\\': c = '\\'; break;
						case '\'': c = '\''; break;
						case 'x': c = scan_num_escape(this, 2); break;
						case 'u': c = scan_num_escape(this, 4); break;
						case 'U': c = scan_num_escape(this, 8); break;
						default:
							valid = false;
							Session::span_err("unknown escape sequence: '\\" + std::string(1, curr) + "'", curr_span());
					}
					bump();
				}

				build_str += c;
			}
			bump(); // move off end quote

			// Invalid strings are set to "??"
			if (!valid) build_str = "??";

			return Token(TokenType::LIT_STRING, build_str, curr_span());
		}

		default:
			Session::span_err("unrecognised character: " + std::string(1, curr), curr_span());
			return Token(TokenType::UNKNOWN, std::string(1, curr), curr_span());
	}
}