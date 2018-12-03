#include "translation_unit.hpp"
#include "util/ranges.hpp"
#include "errors/handler.hpp"

TranslationUnit::TextPos TranslationUnit::pos_from_index(size_t index) const {

	if (newlines.empty() || index < newlines[0])
		return TextPos{0, index};

	TextPos pos;

	// Find the line that the index is from
	for (size_t i = 0; i <= newlines.size(); i++) {
		if (i == newlines.size()) {
			pos.line = i;
			pos.col = index - newlines[pos.line];
			return pos;
		}
		else if (newlines[i] > index) {
			pos.line = i - 1;
			pos.col = index - newlines[pos.line];
			return pos;
		}
	}

	auto err = ErrorHandler::new_error(ErrSeverity::BUG, "failed to find the line that character " + std::to_string(index) + " is on");
	Emitter::emit(err);
	return pos;
}

std::string TranslationUnit::get_line(uint ln) const {

	if (newlines.empty())
		return src;

	size_t i = 1;
	while (src[newlines[ln - 1] + i] != '\n')
		i++;

	return src.substr(newlines[ln - 1], i);
}