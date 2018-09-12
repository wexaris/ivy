#include "lexer.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////      SourceReader      ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void SourceReader::reset(SourceFile* file) {
	// Set internal SourceFile pointer to the new one
	source = file;

	// Set the reading index to the begining of the file
	index = 0;

	// Set the next character and bump it to the current one
	// Bump because that updates the newline indexes in the SourceFile
	// Reset the column counter for it to correspond to the character 'curr'
	next = read_next_char();
	bump();
}

int SourceReader::read_next_char() {
	// If the current index is out of bounds, return '\0'
	if (index == source->len())
		return '\0';

	// Return the next character
	return source->at(index++);
}

void SourceReader::bump(int n) {
	for (int i = 0; i < n; i++) {
		// Move the next character up
		curr = next;

		// Increment reading position
		if (curr == '\n') source->save_newline(bitpos());

		// Read the next character
		next = read_next_char();
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////      Helpers      /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/*	Check if the value is within a certain range of numbers. */
inline bool in_range(char c, char lo, char hi) {
	return lo <= c && c <= hi;
}

/*	Check if the value is whitespace. */
inline bool is_whitespace(char c) {
	return 	c == ' '	||
			c == '\t'	||
			c == '\r'	||
			c == '\n';
}

/*	Check if the value is binary. */
inline bool is_bin(char c) { return c == '0' || c == '1'; }
/*	Check if the value is octal. */
inline bool is_oct(char c) { return in_range(c, '0', '7'); }
/*	Check if the value is decimal. */
inline bool is_dec(char c) { return in_range(c, '0', '9'); }
/*	Check if the value is hexadecimal. */
inline bool is_hex(char c) {
	return  in_range(c, '0', '9')	||
			in_range(c, 'a', 'f')	||
			in_range(c, 'A', 'F');
}

/*	Check if the value is a character. */
inline bool is_alpha(char c) { return in_range(c, 'a', 'z') || in_range(c, 'A', 'Z'); }
/*	Check if the value is alphanumeric. */
inline bool is_alnum(char c) { return is_alpha(c) || is_dec(c); }

/*	Get value of binary digit */
inline int val_bin(char c) { return c == '0' ? 0 : 1; }
/*	Get value of octal digit */
inline int val_oct(char c) { return c - '0'; }
/*	Get value of decimal digit */
inline int val_dec(char c) { return c - '0'; }
/*	Get value of hex digit */
inline int val_hex(char c) {
	if (in_range(c, '0', '9')) return c - '0';
	if (in_range(c, 'a', 'f')) return c - 'a' + 10;
	if (in_range(c, 'A', 'F')) return c - 'A' + 10;
	throw std::runtime_error("Tried to get hex value of " + c);
}

/*	Returns true if the input value corresponds to EOF. */
bool is_valid(Token tk) { return tk.type() != TokenType::END; }

/*	Loops through whitespace and comments.
	Works by bumping until the current character isn't a whitespace or comment. */
void eat_ws_and_comments(SourceReader* reader) {
	// Eat all whitespace
	while (is_whitespace(reader->curr_c())) reader->bump();

	// Eat comments
	if (reader->curr_c() == '/') {
		if (reader->next_c() == '/') {
			// Eat comment until line ends or EOF
			while (reader->curr_c() != '\n' || reader->curr_c() != '\0') reader->bump();
			// Go back and check for whitespace again
			eat_ws_and_comments(reader);
		}
		else if (reader->next_c() == '*') {
			reader->bump(2);
			// Eat block comment until closed
			while (true) {
				// Throw an exception if the block is not terminated at the end of the file
				if (reader->curr_c() == '\0') throw std::runtime_error("Unterminated block comment at end of file");
				if (reader->curr_c() == '*' && reader->next_c() == '/') { reader->bump(2); break; }
				reader->bump();
			}
		}
	}
}

/*	Scans and adds characters to a string as long as their base is under the one specified. */
std::string scan_digits(SourceReader* reader, int base) {
	// String for storing digits
	std::string ret;
	// Get digits as long as they are hex
	while (is_hex(reader->curr_c())) {
		// If the hex value is less than the base, add it to the return
		// If not, stop scanning
		if (val_hex(reader->curr_c()) < base) { ret += reader->curr_c(); reader->bump(); }
		else break;
	}
	return ret;
}
/*	Scans and adds characters that correspond to an exponent. */
std::string scan_exponent(SourceReader* reader) {
	// String for storing exponent sign
	std::string ret;
	if (reader->curr_c() == 'e' || reader->curr_c() == 'E') {
		ret += reader->curr_c();
		reader->bump();
		if (reader->curr_c() == '-' || reader->curr_c() == '+') {
			ret += reader->curr_c();
			reader->bump();
		}
		// String for storing exponent value
		std::string expo = scan_digits(reader, 10);

		// Handle no exponent value
		if (expo.length() == 0) throw std::runtime_error("Bad floating point number literal");

		return ret + expo;
	}
	// No exponent
	return "";
}
/*	Scans and identifies numeric escape characters. */
char scan_num_escape(SourceReader* reader, int digit_num) {
	// Number to build
	int build_num = 0;
	// Loop until requested number of digits
	for (; digit_num != 0; digit_num--) {
		reader->bump();
		// Throw an exception if the character is not hex
		if (!is_hex(reader->curr_c())) throw std::runtime_error("Unknown numeric escape sequence : \\" + reader->curr_c());
		build_num *= 16;
		build_num += val_hex(reader->curr_c());
	}
	return (char)build_num;
}

/*	Responsible for constructing numbers from read characters. */
Token scan_number(SourceReader* reader) {
	// The base for counting
	int base = 10;

	// Handle possible number bases
	if (reader->curr_c() == '0' && (reader->next_c() == 'x' || reader->next_c() == 'X')) 	{ base = 16; reader->bump(2); }
	else if (reader->curr_c() == '0' && reader->next_c() == 'b') 							{ base = 2; reader->bump(2); }
	else if (reader->curr_c() == '0' && reader->next_c() == 'o') 							{ base = 8; reader->bump(2); }

	// The number string
	std::string num_str = scan_digits(reader, base);

	// Handle int declaration
	if (reader->curr_c() == 'i' || reader->curr_c() == 'u') {
		// Set whether integer is signed
		bool sign = (reader->curr_c() == 'i');

		// TODO: Create AST node - signed or unsigned

		reader->bump();

		// Set integer size
		if (reader->curr_c() == '8') { reader->bump();
			// TODO: Create AST node - i8 or u8
		}
		else if (reader->curr_c() == '1' && reader->next_c() == '6') { reader->bump(2);
			// TODO: Create AST node - i16 or u16
		}
		else if (reader->curr_c() == '3' && reader->next_c() == '2') { reader->bump(2);
			// TODO: Create AST node - i32 or u32
		}
		else if (reader->curr_c() == '6' && reader->next_c() == '4') { reader->bump(2);
			// TODO: Create AST node - i64 or u64
		}
		// The parsed number
		int parsed;
		{
			int i = num_str.length() - 1;
			int power = 1u, n = 0;
			while (true) {
				int digit = val_hex(num_str[i]);
				if (digit >= base) throw std::runtime_error("Digit larger than base");
				n += digit * power;
				power *= base;
				if (i == 0) parsed = n;
				i--;
			}
		}
		return sign ? Token(LIT_INT, (intmax_t)parsed) : Token(LIT_UINT, (uintmax_t)parsed);
	}
	// Whether the number is a float
	bool is_float = false;

	// Get decimal place
	if (reader->curr_c() == '.' && !is_alpha(reader->next_c())) {
		is_float = true;
		reader->bump();
		std::string dec_str = scan_digits(reader, 10);
		num_str += "." + dec_str;
	}

	// Get exponent
	std::string expo_str = scan_exponent(reader);
	if (!expo_str.empty()) {
		is_float = true;
		num_str += expo_str;
	}

	// Handle float declaration
	if (reader->curr_c() == 'f') {
		reader->bump();
		if (reader->curr_c() == '3' && reader->next_c() == '2') {
			reader->bump(2);
			// TODO: Create AST node - f32
			return Token(LIT_FLOAT, num_str);
		}
		else if (reader->curr_c() == '6' && reader->next_c() == '4') {
			reader->bump(2);
			// TODO: Create AST node - f64
			return Token(LIT_FLOAT, num_str);
		}
		else
			is_float = true;
	}

	if (is_float)
		// TODO: Create AST node - float
		return Token(LIT_FLOAT, num_str);

	// The parsed number
	int parsed;
	{
		int i = num_str.length();
		int power = 1u, n = 0;
		while (i > 0) {
			int digit = val_hex(num_str[i-1]);
			if (digit >= base) throw std::runtime_error("Digit larger than base");
			n += digit * power;
			power *= base;
			i--;
		}
		parsed = n;
	}
	// TODO: Create AST node - int
	return Token(LIT_INT, (intmax_t)parsed);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////      Lexer      //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void Lexer::load_keywords() {
	keywords["thing"] = THING;
	keywords["str"] = STR;
	keywords["char"] = CHAR;
	keywords["int"] = INT;
	keywords["i8"] = I8;
	keywords["i16"] = I16;
	keywords["i32"] = I32;
	keywords["i64"] = I64;
	keywords["uint"] = UINT;
	keywords["u8"] = U8;
	keywords["u16"] = U16;
	keywords["u32"] = U32;
	keywords["u64"] = U64;
	keywords["float"] = FLOAT;
	keywords["f32"] = F32;
	keywords["f64"] = F64;

	keywords["pack"] = PACK;
	keywords["module"] = MODULE;
	keywords["impost"] = IMPORT;
	keywords["export"] = EXPORT;
	keywords["class"] = CLASS;
	keywords["struct"] = STRUCT;
	keywords["enum"] = ENUM;
	keywords["union"] = UNION;
	keywords["fun"] = FUN;
	keywords["macro"] = MACRO;
	keywords["return"] = RETURN;
	keywords["var"] = VAR;

	keywords["loop"] = LOOP;
	keywords["while"] = WHILE;
	keywords["do"] = DO;
	keywords["for"] = FOR;
	keywords["in"] = IN;

	keywords["match"] = MATCH;
	keywords["switch"] = SWITCH;
	keywords["case"] = CASE;
	keywords["where"] = WHERE;

	keywords["pub"] = PUB;
	keywords["priv"] = PRIV;

	keywords["mut"] = MUT;
	keywords["const"] = CONST;
	keywords["static"] = STATIC;
	keywords["final"] = FINAL;
}

Token Lexer::next_token() {
	// Get rid of whitespace and comments
	eat_ws_and_comments(this);

	// Save position
	int abs = bitpos();
	int ln = lineno();
	int col = colno();

	// If the current character is valid, build the next token
	// If not, make an EOF/END token
	// Set the token's source, position and length
	return (is_valid(curr) ? next_token_inner() : Token(END))
			.loc(src_file()->path(), abs, ln, col, bitpos() - abs);
}

Token Lexer::next_token_inner() {
	// If alnum, build a string, check for keyword
	if (is_alpha(curr) || curr == '_') {
		// String for building words
		std::string build_str;

		// Build string as long as alnum or underscore
		while (is_alnum(curr) || curr == '_') {
			build_str += curr;
			bump();
		}
		// If only underscore, return
		if (build_str == "_") return Token('_');
		// TODO: Check if followed by scope operator, markas scope
		//bool is_scope = (curr == ':') && (next == ':');

		// Check for keywords
		auto item = keywords.find(build_str);
		if (item != keywords.end())
			return Token(item->second);

		// Return string as identifier
		return Token(ID, build_str);
	}
	// If decimal, build number
	if (is_dec(curr))
		return scan_number(this);

	// Check for symbol tokens
	switch (curr) {
		case ',': bump(); return Token(',');
		case ';': bump(); return Token(';');
		case '?': bump(); return Token('?');
		case '(': bump(); return Token('(');
		case ')': bump(); return Token(')');
		case '{': bump(); return Token('{');
		case '}': bump(); return Token('}');
		case '[': bump(); return Token('[');
		case ']': bump(); return Token(']');
		case '~': bump(); return Token('~');
		case '#': bump(); return Token('#');
		case '@': bump(); return Token('@');
		case '.': 
			bump(); if (curr == '.') {
				bump(); if (curr == '.') { bump(); return Token(DOTDOTDOT); }	// ...
				return Token(DOTDOT);											// ..
			} return Token('.');												// .
		case ':':
			if (next == ':') { bump(2); return Token(SCOPE); }		// ::
			bump(); return Token(':');								// :
	
		case '=':
			if (next == '=') { bump(2); return Token(EQEQ); }		// ==
			if (next == '>') { bump(2); return Token(FATARROW);	}	// =>
			bump(); return Token('=');								// =
		case '!':
			if (next == '=') { bump(2); return Token(NE); }			// !=
			bump(); return Token('!');								// !
		case '+':
			if (next == '+') { bump(2); return Token(PLUSPLUS); }	// ++
			if (next == '=') { bump(2); return Token(SUME); }		// +=
			bump(); return Token('+');								// +
		case '-':
			if (next == '-') { bump(2); return Token(MINUSMINUS); }	// --
			if (next == '>') { bump(2); return Token(RARROW); }		// --
			if (next == '=') { bump(2); return Token(SUBE); }		// -=
			bump(); return Token('-');								// -
		case '*':
			if (next == '=') { bump(2); return Token(MULE); }		// *=
			bump(); return Token('*');								// *
		case '/':
			if (next == '=') { bump(2); return Token(DIVE); }		// /=
			bump(); return Token('/');								// /
		case '%':
			if (next == '=') { bump(2); return Token(MODE); }		// %=
			bump(); return Token('%');								// %
		case '^':
			if (next == '=') { bump(2); return Token(CARE); }		// ^=
			bump(); return Token('^');								// ^
		case '&':
			if (next == '&') { bump(2); return Token(AND); }		// &&
			if (next == '=') { bump(2); return Token(ANDE);	}		// &=
			bump(); return Token('&');								// &
		case '|':
			if (next == '|') { bump(2); return Token(OR); }			// ||
			if (next == '=') { bump(2); return Token(ORE); }		// |=
			bump(); return Token('|');								// |
		case '>':
			switch (next) {
				case '=': bump(2); return Token(GE);		// >=
				case '>': bump(2); return Token(SHR);		// >>
				default:  bump(2); return Token('>');		// >
			}
		case '<':
			switch (next) {
				case '=': bump(2); return Token(LE);		// <=
				case '-': bump(2);
					if (curr == '>') { 
						bump(); return Token(DARROW);		// <->
					}
					return Token(LARROW);					// <-
				case '<': bump(2); return Token(SHL);		// <<
				default:  bump(2); return Token('<');		// <
			}

		case '\'':
		{
			// Save character, so that we stay on 'curr'
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
					default: throw std::runtime_error("Unknown character escape sequence: \\" + next);
				}
				// Throw an exception if the character is not terminated
				bump();
				if (curr != '\'') throw std::runtime_error("Unterminated character literal");
				bump();
				return Token(LIT_INT, (intmax_t)c);
			}
			// If the character is in quotes or is a number, it's a literal
			if (is_dec(c) || curr == '\'') {
				// Throw an exception if the character is not terminated
				if (curr != '\'') throw std::runtime_error("Unterminated character literal");
				bump();
				return Token(LIT_INT, (intmax_t)c);
			}

			// If the character isn't in quotes and isn't a number, it's a name
			std::string build_str = std::to_string(c);
			while (is_alnum(curr) || curr == '_') {
				build_str += curr;
				bump();
			}
			return Token(NAME, build_str);
		}

		case '"':
		{
			// String for building words
			std::string build_str;

			bump();
			while (curr != '"') {
				// Throw an exception if the string is not terminated
				if (curr == END) throw std::runtime_error("Unterminated string");

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
						case '\n': while (is_whitespace(curr) && curr != END) bump(); break;
						default: throw std::runtime_error("Unknown string escape sequence: \\" + next);
					}
				}
				// Build string, if character is not escape sequence
				else build_str += curr;
				bump();
			}
			return Token(LIT_STRING, build_str);
		}

		default:
			std::string msg = std::string("Could not identify token with character '") + curr + "'";
			throw std::runtime_error(msg.c_str());
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////  Tests  //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "../source_map.hpp"
#include "token_translate.hpp"
#include <iostream>

void Lexer::test_print_tokens(std::string file) {

	// Stop if the test file is not found
	if (!FileLoader::file_exists(file))
		throw std::runtime_error("TEST_PRINT_TOKENS failed. The file at the provided path could not be opened");

	// Create the SourceFile and Lexer
	SourceFile source(file, FileLoader::read_file(file), 0);
	Lexer lex(&source);

	// Loop through and print all of the tokens
	Token tk = lex.next_token();
	while (tk.type() != TokenType::END) {
		std::cout << translate::tk_str(tk) << std::endl;
		tk = lex.next_token();
	}
}