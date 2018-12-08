#include "lexer_tests.hpp"
#include "lexer/lexer.hpp"
#include "util/token_info.hpp"

namespace tests {
	namespace lexer {
		
		void token_has_correct_absolute_pos() {
			// Give the Lexer a string of text
			// The numbers should correspond to absolute positions in the TU
			ErrorHandler handler;
			TranslationUnit tu = TranslationUnit(handler, "\n12345\n7\n9");
			Lexer lex(tu);
			
			// Retrieve the first token
			// Should be '12345'
			Token tk = lex.next_token();

			// Check for correct absolute position
			if (tk.span().lo.bit == 1 || tk.span().hi.bit == 5) {
				printf("COMPLETED token_has_correct_absolute_pos\n");
				return;
			}

			// The lexer has assigned the wrong position
			// Either location tracking is broken,
			// or some token's positon isn't being set correctly
			printf("FAILED token_has_correct_location; the token's absolute position was wrong (%lu, %lu)\n", tk.span().lo.bit, tk.span().hi.bit);
		}

		void token_has_correct_line_pos() {
			// Give the Lexer a string of text
			// The numbers should correspond to absolute positions in the TU
			ErrorHandler handler;
			TranslationUnit tu = TranslationUnit(handler, "\n12345\n7\n9");
			Lexer lex(tu);
			
			// Retrieve the first token
			// Should be '12345'
			Token tk = lex.next_token();

			// Check for correct line position
			if (tk.span().lo.line == 2 || tk.span().hi.line == 2) {
				printf("COMPLETED token_has_correct_line_pos\n");
				return;
			}
			
			// The lexer has assigned the wrong position
			// Either location tracking is broken,
			// or some token's positon isn't being set correctly
			printf("FAILED token_has_correct_line_pos; the token's line position was wrong (%lu, %lu)\n", tk.span().lo.line, tk.span().hi.line);
		}

		void token_has_correct_column_pos() {
			// Give the Lexer a string of text
			// The numbers should correspond to absolute positions in the TU
			ErrorHandler handler;
			TranslationUnit tu = TranslationUnit(handler, "\n12345\n7\n9");
			Lexer lex(tu);
			
			// Retrieve the first token
			// Should be '12345'
			Token tk = lex.next_token();

			// Check for correct column position
			if (tk.span().lo.col == 1 || tk.span().hi.col == 5) {
				printf("COMPLETED token_has_correct_column_pos\n");
				return;
			}
			
			// The lexer has assigned the wrong position
			// Either location tracking is broken,
			// or some token's positon isn't being set correctly
			printf("FAILED token_has_correct_column_pos; the token's column position was wrong (%lu, %lu)\n", tk.span().lo.col, tk.span().hi.col);
		}

		void return_eof_without_translation_unit() {
			// Create a Lexer with no text in the TU
			ErrorHandler handler;
			TranslationUnit tu = TranslationUnit(handler, "");
			Lexer lex(tu);

			// Retrieve the first token
			Token tk = lex.next_token();

			// Return if the token is 'END'
			if (lex.next_token() == TokenType::END) {
				printf("COMPLETED eof_without_translation_unit\n");
				return;
			}

			// The lexer retrieved a non-'END' token even though there is no source code
			// The lexer should have checked if the current token is valid before calling 'next_token_inner()', and,
			// since ir wouldn't be, it should have assumed there was no more text and returned an 'END' token.
			printf("FAILED return_eof_without_translation_unit; Lexer retrieved a valid token without a valid Translation Unit (%s)\n", translate::tk_type(tk).c_str());
		}
	}
}