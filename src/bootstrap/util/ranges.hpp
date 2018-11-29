#pragma once
#include "lexer/lexer.hpp"
#include <optional>
#include <assert.h>

namespace range {

	/*	Check if the value is a whitespace. */
	static inline bool is_whitespace(char c) {
		return 	c == ' '	||
				c == '\t'	||
				c == '\r'	||
				c == '\n';
	}

	/* Check if the given integer can be a chracter.  */
	static inline bool is_char(uint i) {
		return i < 256;
	}

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

	/* Check if the character can start an identifier. */
	static inline bool is_ident_start(char c) { return is_alpha(c) || c == '_'; }
	/* Check if the character can continue an identifier. */
	static inline bool is_ident_cont(char c) { return is_alnum(c) || c == '_'; }

	/* Attempts to get a number from a character.
	 * Checks the character for the right base.
	 * If the character can't be a number in the given base,
	 * a 'nullopt' is retuned. */
	static inline std::optional<uint> get_num(char c, uint base) {
		assert(base <= 36);

		if (base == 10) {
			if (is_dec(c))
				return c - '0';
			return std::nullopt;
		}
		uint val;
		if (base < 10) {
			if (is_dec(c))
				val = c - '0';
			else return std::nullopt;
		}
		else {
			if (in_range(c, '0', '9'))
				val =  c - '0';
			else if (in_range(c, 'a', 'f'))
				val = c - 'a' + 10;
			else if (in_range(c, 'A', 'F'))
				val = c - 'A' + 10;
			else return std::nullopt;
		}

		if (val < base) return val;
		else return std::nullopt;
	}
}