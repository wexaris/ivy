#pragma once
#include <string>
#include <vector>

/* An item containing source code, ususally a file. 
 * Stores the path to the file of origin and the source.
 * Has a position inside the larger SourceMap. */
class TranslationUnit {
	
private:
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
	explicit TranslationUnit(const std::string& src) 
		: src(src) {}

	TranslationUnit(const std::string& path, const std::string& src, size_t start_pos) 
		: path(path), src(src), start_position(start_pos) {}

	/* An structure for passing around lines and columns. */
	struct TextPos {
		size_t line;
		size_t col;
	};

	std::string this_source_line(size_t index) const;

	/* Returns the line of source that the index was from.
	 * Beginning and ending whitespace is removed. */
	TextPos pos_from_index(size_t index) const;

	std::string get_line(size_t ln) const;

	/* Returns the line of source that the index was from.
	 * Beginning and ending whitespace is removed. */
	void save_newline(size_t index) {
		newlines.push_back(index);
	}

	/* Returns the path to Translation Unit. */
	inline const std::string& filepath() const	{ return path; }
	/* A view into the file's source code. */
	inline std::string_view source() const		{ return src; }

	/* Start position in the CodeMap. */
	inline int start_pos() const				{ return start_position; }
	/* End position in the CodeMap. */
	inline size_t end_pos() const				{ return start_position + src.length(); }
};