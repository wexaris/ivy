#pragma once
#include "source_file.hpp"
#include <fstream>
#include <sstream>
#include <memory>

/*	A static class with functions to check if a file exists
	and to get all of the text in a file. */
class FileLoader {

private:
	FileLoader() {}

public:
	/* Returns true if there exists a file at the given path. */
	static bool file_exists(const std::string& path) {
		std::ifstream f (path, std::ios::in | std::ios::binary);
		return f.good();
	}

	/* 	Returns a string with all of the text in the file at the give path
		Throws an exception if the file doesn't exist.
		Checking 'file_exists()' before reading is advised. */
	static std::string read_file(const std::string& path);

	static std::string name_from_path(const std::string& path);
};

/*	A map containing all of the source files in a package.
	Contains a vector of all of the SourceFile objects.
	The SourceMap provides a continious map of all of the
	parsed source code. */
class SourceMap {

private:
	/* 	All of the SourceFiles in the SourceMap. 
		Managed by std::unique_ptrs. */
	std::vector<std::unique_ptr<SourceFile>> source_files;

	/*	Create a new SourceFile object and insert it into the SourceMap.
		Does not guard against multiple insertions of the same file. */
	SourceFile* new_source_file(std::string path, std::string src);

public:
	SourceMap() {}

	/*	Load a file at a given path into the SourceMap.
		Does not guard against multiple insertions of the same file. */
	SourceFile* load_file(const std::string& path);

	/* The next free index in the SourceMap. */
	inline int next_start_pos() const { 
		return source_files.size() == 0 ? 0 : (*source_files.end())->end_pos() + 1;
	}

	/* A constant referance to the SourceFiles in the SourceMap. */
	inline const std::vector<std::unique_ptr<SourceFile>>& files() const { return source_files; }
};