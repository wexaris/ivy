#include "translation_unit.hpp"
#include "util/ranges.hpp"

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

	return src.substr(start, index - start);
}