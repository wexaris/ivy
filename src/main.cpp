
// Check Windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENV64
#else
#define ENV32
#endif
#endif

// Check GNU
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENV64
#else 
#define ENV32
#endif
#endif

#include "bootstrap/syntax/front/parser.hpp"

int main(/*int argc, char* argv[]*/) {

	// TODO: /lexer/numbers/ add AST nodes
	// TODO: Make the parser
	
	Lexer::test_print_tokens();
	
	return 0; 
}