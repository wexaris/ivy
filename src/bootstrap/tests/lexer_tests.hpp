#pragma once

namespace tests {
	namespace lexer {

		void token_has_correct_absolute_pos();
		void token_has_correct_line_pos();
		void token_has_correct_column_pos();
		
		void return_eof_without_translation_unit();

	}
}