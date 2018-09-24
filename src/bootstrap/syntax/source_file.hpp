#pragma once
#include <string>
#include <vector>

class SourceFile {
	
private:
	/* The path to the file from which the source code has been read */
	std::string filepath;
	/* The full source code from a file */
	std::string src;

	/* 	The indexes of newline characters.
		Useful for quickly correlating line and absolute positions. */
	std::vector<int> newline_pos;

	/* Start position in the CodeMap */
	int start_position;

	/* The origin of the SourceFile' source code. */
	enum SFOrigin {
		// SourceFile source is from a file on the system
		FILE,
		// SourceFile source is hand written, from the termial, or elsewhere
		RAW,
	} src_origin;

public:
	/* 	Create a new SourceFile.
		Assumed to be from a file. */
	SourceFile(std::string path, std::string src, int start_pos) 
		: filepath(path), src(src), start_position(start_pos), src_origin(FILE) { }
	/* 	Create a new SourceFile.
		Assumed to be raw. */
	SourceFile(std::string src, int start_pos) 
		: src(src), start_position(start_pos), src_origin(RAW) { }

	inline std::string path() const { return filepath; }

	/* Start position in the CodeMap. */
	inline int start_pos() const { return start_position; }
	/* End position in the CodeMap. */
	inline int end_pos() const { return start_position + src.length(); }

	/* 	Get character from the file at a given index.
		Index is relative to the file, not the CodeMap. */
	inline char at(int index) const { return src[index]; }

	/* Length of the source code in the file. */
	inline int len() const { return src.length(); }

	/* Origin of the SourceFiles' source. */
	inline SFOrigin origin() const { return src_origin; }

	/*	Returns the line position corresponding to the given index.
		The index must be relative to the source file. */
	int line_from_index(int index) const;

	/*	Returns the column position corresponding to the given index.
		The index must be relative to the source file. */
	int col_from_index(int index) const;

	inline void save_newline(int index) {
		newline_pos.push_back(index);
	}
};