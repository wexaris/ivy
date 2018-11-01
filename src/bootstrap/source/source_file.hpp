#pragma once
#include <string>
#include <vector>

class SourceFile {
	
private:
	/* The path to the file from which the source code has been read */
	std::string path;
	/* The full source code from a file */
	std::string src;

	/* The indexes of newline characters.
	 * Useful for quickly correlating line and absolute positions.
	 */
	std::vector<int> newline_pos;

	/* Start position in the CodeMap */
	int start_position;

	/* The origin of the SourceFile' source code. */
	enum Origin {
		// SourceFile source is from a file on the system
		FILE,
		// SourceFile source is hand written, from the terminal, or elsewhere
		RAW,
	} src_origin;

public:
	/* Create a new SourceFile.
	 * Assumed to be from a file.
	 */
	SourceFile(const std::string& path, const std::string& src, int start_pos) 
		: path(std::move(path)), src(std::move(src)), newline_pos(), start_position(start_pos), src_origin(FILE) { }
	/* Create a new SourceFile.
	 * Assumed to be raw.
	 * */
	SourceFile(const std::string& src, int start_pos) 
		: path(), src(std::move(src)), newline_pos(), start_position(start_pos), src_origin(RAW) { }

	inline std::string filepath() const { return path; }

	/* Start position in the CodeMap. */
	inline int start_pos() const { return start_position; }
	/* End position in the CodeMap. */
	inline unsigned long end_pos() const { return start_position + src.length(); }

	/* A view into the file's source code. */
	inline const std::string& source() { return src; }

	/* Length of the source code in the file. */
	inline unsigned long len() const { return src.length(); }

	/* The type of origin of the file's source code. */
	inline Origin origin() const { return src_origin; }


	/* Save a index as a newline position.
	 * Called when the source is being tokenized.
	 * Newline indexing is used for error reporting,
	 * as it provides a fast way to get to the needed line. */
	inline void save_newline(int index) {
		newline_pos.push_back(index);
	}
};