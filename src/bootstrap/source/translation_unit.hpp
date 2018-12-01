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

	/* The indexes of newline characters.
	 * Useful for getting a to line of code without re-reading the entire source. */
	std::vector<size_t> newline_pos = std::vector<size_t>();

	/* Start position in the CodeMap */
	int start_position;

public:
	/* Create a new Translation Unit. */
	explicit TranslationUnit(const std::string& src) 
		: src(src) {}
	/* Create a new Translation Unit. */
	TranslationUnit(const std::string& path, const std::string& src, size_t start_pos) 
		: path(path), src(src), start_position(start_pos) {}

	/* Returns the path to Translation Unit. */
	inline std::string filepath() const			{ return path; }

	/* Start position in the CodeMap. */
	inline int start_pos() const				{ return start_position; }
	/* End position in the CodeMap. */
	inline size_t end_pos() const				{ return start_position + src.length(); }

	/* A view into the file's source code. */
	inline std::string_view source() const		{ return src; }

	/* Length of the source code in the file. */
	inline size_t len() const					{ return src.length(); }
};