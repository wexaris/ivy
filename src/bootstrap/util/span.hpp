#pragma once
#include <optional>

class TranslationUnit;

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
		int line;
		int col;
	} lo, hi;

	Span(const TranslationUnit* tu, size_t lo_bit, int lo_line, int lo_col, size_t hi_bit, int hi_line, int hi_col)
		: tu(tu),
		lo(LinePos{ lo_bit, lo_line, lo_col }),
		hi(LinePos{ hi_bit, hi_line, hi_col })
	{}
};