#include "lexer_tests.hpp"
#include "lexer/lexer.hpp"
#include "util/token_info.hpp"

namespace tests {
	namespace lexer {
		
		void token_has_correct_absolute_pos() {
			// Give the Lexer a string of text
			// The numbers should correspond to absolute positions in the TU
			Emitter emitter;
			ErrorHandler handler(emitter);
			TranslationUnit tu = TranslationUnit(handler, "\n12345\n7\n9");
			Lexer lex(tu, handler);
			
			// Retrieve the first token
			// Should be '12345'
			Token tk = lex.next_token();

			// Check for correct absolute position
			if (tk.span().lo_bit == 1 || tk.span().hi_bit == 5) {
				printf("COMPLETED token_has_correct_absolute_pos\n");
				return;
			}

			// The lexer has assigned the wrong position
			// Either location tracking is broken,
			// or some token's positon isn't being set correctly
			printf("FAILED token_has_correct_location; the token's absolute position was wrong (%lu, %lu)\n", tk.span().lo_bit, tk.span().hi_bit);
		}

		void token_has_correct_line_pos() {
			// Give the Lexer a string of text
			// The numbers should correspond to absolute positions in the TU
			Emitter emitter;
			ErrorHandler handler(emitter);
			TranslationUnit tu = TranslationUnit(handler, "\n12345\n7\n9");
			Lexer lex(tu, handler);
			
			// Retrieve the first token
			// Should be '12345'
			Token tk = lex.next_token();
			auto lo_pos = tk.span().lo_textpos();
			auto hi_pos = tk.span().hi_textpos();

			// Check for correct line position
			if (lo_pos.line == 2 || hi_pos.line == 2) {
				printf("COMPLETED token_has_correct_line_pos\n");
				return;
			}
			
			// The lexer has assigned the wrong position
			// Either location tracking is broken,
			// or some token's positon isn't being set correctly
			printf("FAILED token_has_correct_line_pos; the token's line position was wrong (%lu, %lu)\n", lo_pos.line, hi_pos.line);
		}

		void token_has_correct_column_pos() {
			// Give the Lexer a string of text
			// The numbers should correspond to absolute positions in the TU
			Emitter emitter;
			ErrorHandler handler(emitter);
			TranslationUnit tu = TranslationUnit(handler, "\n12345\n7\n9");
			Lexer lex(tu, handler);
			
			// Retrieve the first token
			// Should be '12345'
			Token tk = lex.next_token();
			auto lo_pos = tk.span().lo_textpos();
			auto hi_pos = tk.span().hi_textpos();

			// Check for correct column position
			if (lo_pos.col == 1 || hi_pos.col == 5) {
				printf("COMPLETED token_has_correct_column_pos\n");
				return;
			}
			
			// The lexer has assigned the wrong position
			// Either location tracking is broken,
			// or some token's positon isn't being set correctly
			printf("FAILED token_has_correct_column_pos; the token's column position was wrong (%lu, %lu)\n", lo_pos.col, hi_pos.col);
		}

		void return_eof_without_translation_unit() {
			// Create a Lexer with no text in the TU
			Emitter emitter;
			ErrorHandler handler(emitter);
			TranslationUnit tu = TranslationUnit(handler, "");
			Lexer lex(tu, handler);

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