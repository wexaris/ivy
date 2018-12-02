#include "translation_unit.hpp"
#include "util/ranges.hpp"
#include "errors/handler.hpp"

TranslationUnit::TextPos TranslationUnit::pos_from_index(size_t index) const {

	if (newlines.empty() || index < newlines[0])
		return TextPos{0, index};

	TextPos pos;

	// Find the line that the index is from
	for (size_t i = 0; i < newlines.size(); i++) {
		if (i == newlines.size())
			pos.line = i;
		else if (newlines[i] > index)
			pos.line = i - 1;
		pos.col = index - newlines[pos.line];
	}

	auto err = ErrorHandler::new_error(ErrSeverity::BUG, "failed to find the line that character " + std::to_string(index) + " is on");
	Emitter::emit(err);
	return pos;
}

std::string TranslationUnit::get_line(uint ln) const {

	if (newlines.empty())
		return src;

	size_t i = 1;
	while (src[newlines[ln] + i] != '\n')
		i++;

	return src.substr(newlines[ln], i);
}

std::string TranslationUnit::this_source_line(size_t index) const {
	// Declate a start index
	// It's starting value is one less than 'index' because,
	// if the char at 'index' is a newline, it would be saved as the start,
	// enven though it should have been the line's end.
	size_t start = index - 1;

	// Find the place where the line starts
	// Keep decreasing the start index until we reach a '\n'
	char c = src[start];
	while (true) {
		if (c == '\n')
			break;
		c = src[--start];
	}
	// Remove beginning whitespace
	while (range::is_whitespace(src[start]))
		start++;

	// Find the place where the line ends
	// Keep increasing the end index until we reach a '\n'
	c = src[index];
	while (true) {
		if (c == '\n')
			break;
		c = src[++index];
	}
	// Remove ending whitespace
	while (range::is_whitespace(src[index]))
		index--;

	return src.substr(start, index - start + 1);
}