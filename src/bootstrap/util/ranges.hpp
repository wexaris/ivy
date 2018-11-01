#pragma once
#include "lexer/lexer.hpp"

struct range {

	/* Check if the value is within a certain range of numbers. */
	static inline bool in_range(char c, char lo, char hi) {
		return lo <= c && c <= hi;
	}

	/* Check if the value is binary. */
	static inline bool is_bin(char c) { return c == '0' || c == '1'; }
	/* Check if the value is octal. */
	static inline bool is_oct(char c) { return in_range(c, '0', '7'); }
	/* Check if the value is decimal. */
	static inline bool is_dec(char c) { return in_range(c, '0', '9'); }
	/* Check if the value is hexadecimal. */
	static inline bool is_hex(char c) {
		return  in_range(c, '0', '9')	||
				in_range(c, 'a', 'f')	||
				in_range(c, 'A', 'F');
	}

	/* Check if the value is a character. */
	static inline bool is_alpha(char c) { return in_range(c, 'a', 'z') || in_range(c, 'A', 'Z'); }
	/* Check if the value is alphanumeric. */
	static inline bool is_alnum(char c) { return is_alpha(c) || is_dec(c); }

	/* Get value of binary digit */
	static inline int val_bin(char c) { return c == '0' ? 0 : 1; }
	/* Get value of octal digit */
	static inline int val_oct(char c) { return c - '0'; }
	/* Get value of decimal digit */
	static inline int val_dec(char c) { return c - '0'; }
	/* Get value of hex digit */
	static inline int val_hex(char c, Lexer* lex) {
		if (in_range(c, '0', '9')) return c - '0';
		if (in_range(c, 'a', 'f')) return c - 'a' + 10;
		if (in_range(c, 'A', 'F')) return c - 'A' + 10;

		Span sp(lex->src(), lex->bitpos()-1, lex->lineno(), lex->colno()-1, lex->bitpos(), lex->lineno(), lex->colno());
		lex->err("could not get hex value of '" + std::to_string(c) + "'", sp);
	}
};