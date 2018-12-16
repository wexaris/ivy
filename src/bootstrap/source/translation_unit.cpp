#include "translation_unit.hpp"
#include "util/ranges.hpp"
#include "errors/handler.hpp"

TextPos TranslationUnit::pos_from_index(size_t index) const {

	// File has no newlines or it hasn't been entirely lexed
	if (newlines.empty() || index < newlines[0])
		return TextPos{0, index};

	TextPos pos;

	// Find the line that the index is from
	for (size_t i = 0; i <= newlines.size(); i++) {
		if (i == newlines.size()) {
			pos.line = i + 1;
			pos.col = index - newlines[pos.line - 2] + 1;
			return pos;
		}
		else if (newlines[i] > index) {
			pos.line = i + 1;
			pos.col = index - newlines[pos.line - 2] + 1;
			return pos;
		}
	}

	handler->make_bug("failed to retrieve the line corresponding to index " + std::to_string(index)).emit();
	return pos;
}

std::string TranslationUnit::get_line(size_t ln) const {

	size_t ln_index = ln - 1;
	std::string str;

	// File has no newlines or it hasn't been entirely lexed
	if (newlines.empty()) {
		str = src;
	}
	else if (ln_index == 0) {
		size_t len = newlines[0] - 1;
		str = src.substr(0, len);
	}
	// We know when the next line begins
	else if (newlines.size() > ln_index) {
		size_t start = newlines[ln_index - 1];
		size_t len = newlines[ln_index] - newlines[ln_index - 1] - 1;
		str = src.substr(start, len);
	}
	else {
		size_t i = 1;
		size_t line_start = newlines[ln_index - 1];
		// Add to length 'i' until newline or EOF
		while (src.length() > line_start + i && src[line_start + i] != '\n')
			i++;

		// Get the line
		str = src.substr(line_start, i);
	}

	// Replace '\t' with four spaces
	// Makes debugging message lengths consistent 
	size_t pos = 0;
	while ((pos = str.find("\t")) != str.npos) {
		str.replace(pos, 1, "    ");
		pos += 4;
	}

	return str;
}