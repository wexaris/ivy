#pragma once
#include <cstddef>

class TranslationUnit;

/* An structure for passing around lines and columns. */
struct TextPos {
	size_t line;
	size_t col;
};

/* A position in a file.
 * Keeps a pointer to the file of origin,
 * an absolute position in the file, as well as
 * the line and column at the start and end of the span. */
struct Span {
	/* The Translation Unit of the span's origin. */
	const TranslationUnit* tu;

	/* A position in a file.
	 * Holds the absolute bit position,
	 * as well as the line and column. */
	struct LinePos {
		size_t bit;
		size_t line;
		size_t col;
	} lo, hi;

	Span() : tu(nullptr), lo(LinePos{ 0, 0, 0 }), hi(LinePos{ 0, 0, 0 }) {}

	Span(const TranslationUnit& tu, size_t lo_bit, size_t lo_line, size_t lo_col, size_t hi_bit, size_t hi_line, size_t hi_col)
		: tu(&tu),
		lo(LinePos{ lo_bit, lo_line, lo_col }),
		hi(LinePos{ hi_bit, hi_line, hi_col })
	{}
};