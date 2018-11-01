#pragma once
#include <optional>

class SourceFile;

/* A position in a file.
 * Keeps a pointer to the file of origin,
 * an absolute position in the file, as well as
 * the line and column at the start and end of the span.
 */
struct Span {
	/* The SourceFile of origin. */
	SourceFile& sf;

	/* A position in a file.
	 * Holds the absolute bit position,
	 * as well as the line and column. */
	struct LinePos { 
		int bit;
		int line;
		int col;
	} lo, hi;

	Span(SourceFile& sf, int lo_bit, int lo_line, int lo_col, int hi_bit, int hi_line, int hi_col)
		: sf(sf),
		lo(LinePos{ lo_bit, lo_line, lo_col }),
		hi(LinePos{ hi_bit, hi_line, hi_col })
	{}
};