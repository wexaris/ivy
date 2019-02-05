#pragma once
#include "errors/handler.hpp"
#include <string>
#include <vector>

/* An item containing source code, usually a file. 
 * Stores the path to the file of origin and the source.
 * Has a position inside the larger SourceMap. */
class TranslationUnit {
	
private:
	/* The Session's ErrorHandler */
	ErrorHandler* handler;

	/* The path to the file from which the source code has been read */
	const std::string path;
	/* The full source code from a file */
	const std::string src;

	/* This Translation Unit's start position in the CodeMap */
	size_t start_position;

	/* The positions of all of the newline markers in the source code.
	 * Used for getting positions quickly. */
	std::vector<size_t> newlines;

public:
	explicit TranslationUnit(ErrorHandler& handler, const std::string& src) 
		: handler(&handler), src(src) {}

	TranslationUnit(ErrorHandler& handler, const std::string& path, const std::string& src, size_t start_pos) 
		: handler(&handler), path(path), src(src), start_position(start_pos) {}

	std::string this_source_line(size_t index) const;

	/* Returns the line of source that the index was from.
	 * Beginning and ending whitespace is removed. */
	TextPos pos_from_index(size_t index) const;

	/* Returns the line of source that the index was from.
	 * If format is enabled beginning and ending whitespace is removed and
	 * Tab characters are replaced by four spaces. */
	std::string get_line(size_t ln, bool fmt) const;

	/* Save a newline position.
	 * Getting positions from an index relies on this. */
	void save_newline(size_t index) {
		newlines.push_back(index);
	}

	/* Returns the path to Translation Unit. */
	inline const std::string& filepath() const	{ return path; }
	/* A view into the file's source code. */
	inline std::string_view source() const		{ return src; }

	/* Start position in the CodeMap. */
	inline size_t start_pos() const				{ return start_position; }
	/* End position in the CodeMap. */
	inline size_t end_pos() const				{ return start_position + src.length(); }
};