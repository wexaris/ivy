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
			pos.line = i;
			pos.col = index - newlines[pos.line-1];
			return pos;
		}
		else if (newlines[i] > index) {
			pos.line = i;
			for (auto e : newlines)
				printf("%lu\n", e);
			pos.col = index - newlines[pos.line];
			return pos;
		}
	}

	auto err = ErrorHandler::new_error(ErrSeverity::BUG, "failed to find the line that character " + std::to_string(index) + " is on");
	Emitter::emit(err);
	return pos;
}

std::string TranslationUnit::get_line(size_t ln) const {

	// File has no newlines or it hasn't been entirely lexed
	if (newlines.empty())
		return src;

	// We know when the next line begins
	if (newlines.size() > ln)
		return src.substr(newlines[ln - 1], newlines[ln] - newlines[ln - 1] - 1);

	size_t i = 1;
	while (src.length() > newlines[ln - 1] + i && src[newlines[ln - 1] + i] != '\n')
		i++;

	return src.substr(newlines[ln - 1], i);
}