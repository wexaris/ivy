#include "parser.hpp"
#include "token_translate.hpp"

void Parser::expect(int type) { 
	if (token.type() != type) {
		std::string msg = std::string("Expected ") + translate::tk_str(type) + " type, found " + translate::tk_str(token.type());
		throw std::runtime_error(msg);
	}
}