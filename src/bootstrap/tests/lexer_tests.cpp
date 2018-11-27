#include "lexer_tests.hpp"

namespace tests {
	namespace lexer {
		
		void token_has_correct_absolute_pos() {
			// Give the Lexer a string of text
			// The numbers should correspond to absolute positions in the TU
			TranslationUnit tu = TranslationUnit("\n12345\n7\n9");
			Lexer lex(tu);
			
			// Retrieve the first token
			// Should be '12345'
			Token tk = lex.next_token();

			// Check for correct absolute position
			if (tk.span().lo.bit == 1 || tk.span().hi.bit == 5) {
				Session::msg("COMPLETED token_has_correct_absolute_pos");
				return;
			}

			// The lexer has assigned the wrong position
			// Either location tracking is broken,
			// or some token's positon isn't being set correctly
			Session::err("FAILED token_has_correct_location; the token's absolute position was wrong (" \
						+ std::to_string(tk.span().lo.bit) + "," + std::to_string(tk.span().hi.bit) + ")");
		}

		void token_has_correct_line_pos() {
			// Give the Lexer a string of text
			// The numbers should correspond to absolute positions in the TU
			TranslationUnit tu = TranslationUnit("\n12345\n7\n9");
			Lexer lex(tu);
			
			// Retrieve the first token
			// Should be '12345'
			Token tk = lex.next_token();

			// Check for correct line position
			if (tk.span().lo.line == 2 || tk.span().hi.line == 2) {
				Session::msg("COMPLETED token_has_correct_line_pos");
				return;
			}
			
			// The lexer has assigned the wrong position
			// Either location tracking is broken,
			// or some token's positon isn't being set correctly
			Session::err("FAILED token_has_correct_line_pos; the token's line position was wrong (" \
						+ std::to_string(tk.span().lo.line) + "," + std::to_string(tk.span().hi.line) + ")");
		}

		void token_has_correct_column_pos() {
			// Give the Lexer a string of text
			// The numbers should correspond to absolute positions in the TU
			TranslationUnit tu = TranslationUnit("\n12345\n7\n9");
			Lexer lex(tu);
			
			// Retrieve the first token
			// Should be '12345'
			Token tk = lex.next_token();

			// Check for correct column position
			if (tk.span().lo.col == 1 || tk.span().hi.col == 5) {
				Session::msg("COMPLETED token_has_correct_column_pos");
				return;
			}
			
			// The lexer has assigned the wrong position
			// Either location tracking is broken,
			// or some token's positon isn't being set correctly
			Session::err("FAILED token_has_correct_column_pos; the token's column position was wrong (" \
						+ std::to_string(tk.span().lo.col) + "," + std::to_string(tk.span().hi.col) + ")");
		}

		void return_eof_without_translation_unit() {
			// Create a Lexer with no text in the TU
			TranslationUnit tu = TranslationUnit("");
			Lexer lex(tu);

			// Retrieve the first token
			Token tk = lex.next_token();

			// Return if the token is 'END'
			if (lex.next_token() == TokenType::END) {
				Session::msg("COMPLETED eof_without_translation_unit");
				return;
			}

			// The lexer retrieved a non-'END' token even though there is no source code
			// The lexer should have checked if the current token is valid before calling 'next_token_inner()', and,
			// since ir wouldn't be, it should have assumed there was no more text and returned an 'END' token.
			Session::err("FAILED return_eof_without_translation_unit; Lexer retrieved a valid token without a valid Translation Unit (" \
						+ translate::tk_type(tk) + ")");
		}
	}
}