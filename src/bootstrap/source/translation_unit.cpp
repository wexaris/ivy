#include "translation_unit.hpp"
#include "util/ranges.hpp"
#include "errors/handler.hpp"

TranslationUnit::TextPos TranslationUnit::pos_from_index(size_t index) const {

	// File has no newlines or it hasn't been entirely lexed
	if (newlines.empty() || index < newlines[0])
		return TextPos{0, index};

	TextPos pos;

	// Find the line that the index is from
	for (size_t i = 0; i <= newlines.size(); i++) {
		if (i == newlines.size()) {
			pos.line = i + 1;
			pos.col = index - newlines[pos.line - 2];
			return pos;
		}
		else if (newlines[i] > index) {
			pos.line = i + 1;
			pos.col = index - newlines[pos.line - 2] + 1;
			return pos;
		}
	}

	handler->new_error(BUG, "failed to retrieve the line corresponding to index " + std::to_string(index), 0).emit();
	return pos;
}

std::string TranslationUnit::get_line(size_t ln) const {

	std::string str;

	// File has no newlines or it hasn't been entirely lexed
	if (newlines.empty()) {
		str = src;
	}
	// We know when the next line begins
	else if (newlines.size() > ln) {
		str = src.substr(newlines[ln - 1], newlines[ln] - newlines[ln - 1] - 1);
	}
	else {
		// Find end of line
		size_t i = 1;
		while (src.length() > newlines[ln - 1] + i && src[newlines[ln - 1] + i] != '\n')
			i++;

		// Get the line in a string
		str = src.substr(newlines[ln - 1], i);
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