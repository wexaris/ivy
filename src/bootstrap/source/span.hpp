#pragma once
#include <cstddef>

class TranslationUnit;

/* An structure for passing around lines and columns. */
struct TextPos {
	size_t line = 0;
	size_t col = 0;
};

/* A large position in a file.
 * Stores a pointer to the Translation Unit of origin,
 * the absolute start and end positions in the file, as well as
 * the line and column of the start and end. */
struct WideSpan {

	const TranslationUnit* tu;

	struct FilePos {
		size_t bit;
		size_t line;
		size_t col;
	} lo, hi;

	WideSpan(const TranslationUnit& tu, size_t lo_bit, size_t lo_line, size_t lo_col, size_t hi_bit, size_t hi_line, size_t hi_col)
		: tu(&tu),
		lo({lo_bit, lo_line, lo_col}),
		hi({hi_bit, hi_line, hi_col})
	{}
};

/* A position in a file.
 * Stores a pointer to the Translation Unit of origin and
 * the absolute start and end positions in the file. */
struct Span {

	const TranslationUnit* tu;

	size_t lo_bit = 0;
	size_t hi_bit = 0;

	Span() : tu(nullptr) {}

	Span(const TranslationUnit& tu, size_t lo_bit, size_t hi_bit)
		: tu(&tu), lo_bit(lo_bit), hi_bit(hi_bit)
	{}

	/* Finds the line and column that the Span starts at.
	 * The position is not stored in the Span, rather it is searched for in the TU. */
	TextPos lo_textpos() const;
	/* Finds the line and column that the Span ends at.
	 * The position is not stored in the Span, rather it is searched for in the TU. */
	TextPos hi_textpos() const;

	WideSpan into_wide() const {
		auto lo_pos = lo_textpos();
		auto hi_pos = hi_textpos();
		return WideSpan(*tu, lo_bit, lo_pos.line, lo_pos.col, hi_bit, hi_pos.line, hi_pos.col);
	}
};