#include "source_file.hpp"
#include <iostream>
#include <sstream>

int SourceFile::col_from_index(int index) const {
	// Check if index is within the source file bounds
	if (index >= len()) {
		std::string msg = std::string("Index ") + std::to_string(index) + " of source file " + filepath + " is out of bounds";
		throw std::runtime_error(msg);
	}
	
	// If there are no newlines or the index is on the first line,
	// Return the index itself
	if (newline_pos.size() == 0 || newline_pos[0] <= index)
		return index;

	// Subtract last newline index from requested index -
	// that's the column number
	for(size_t i = 0; i < newline_pos.size(); i++)
		if (newline_pos[i] > index)
			return index - newline_pos[i-1];

	throw std::runtime_error("Couldn't get column from index. This is a bug.");
}

int SourceFile::line_from_index(int index) const {
	for(size_t i = 0; i < newline_pos.size(); i++)
		if (newline_pos[i] > index)
			return i - 1;
	return newline_pos.size();
}