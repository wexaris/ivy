#pragma once
#include "token.hpp"

namespace translate {

	/* Returns the type of the token as a string.
	 * Used mostly for debugging and error logging.
	 */
	std::string tk_str(int type);

	/* Returns the type of the token as a string.
	 * Used mostly for debugging and error logging.
	 */
	inline std::string tk_str(TokenType type) {
		return tk_str((int)type);
	}

	/* Returns the type of the token as a string.
	 * Used mostly for debugging and error logging.
	 */
	std::string tk_str(Token tk);
}