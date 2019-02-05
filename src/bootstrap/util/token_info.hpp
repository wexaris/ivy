#pragma once
#include "token/token.hpp"

namespace translate {

	/* Returns the type of the token as a string. */
	std::string tk_type(int type);

	/* Returns the type of the token as a string. */
	inline std::string tk_type(TokenType type) {
		return tk_type((int)type);
	}

	/* Returns the type of the token as a string. */
	inline std::string tk_type(const Token& tk) {
		return tk_type(tk.type());
	}

	/* Returns the value of the token as a string. */
	std::string tk_info(const Token& tk);
	/* Returns the value of the type. */
	std::string tk_info(int ty);
}