#pragma once
#include "translation_unit.hpp"
#include <fstream>
#include <sstream>
#include <memory>

/* A static class with functions to check if a file exists
 * and to get all of the text in a file. */
class FileLoader {

private:
	FileLoader() = default;

public:
	/* Returns true if there exists a file at the given path. */
	static bool file_exists(const std::string& path);

	/* Returns a string with all of the text in the file at the give path
	 * Throws an exception if the file doesn't exist.
	 * Checking 'file_exists()' before reading is advised. */
	static std::string read_file(const std::string& path);
};

/* A map containing all of the source files in a package.
 * Contains a vector of all of the Translation Units.
 * The SourceMap provides a continuous map of all of the
 * parsed source code. */
class SourceMap {

private:
	ErrorHandler& handler;

	/* All of the Translation Units in the SourceMap.
	 * Managed by std::unique_ptrs. */
	std::vector<std::unique_ptr<TranslationUnit>> translation_units;

	/* Creates and returns a new Translation Unit.
	 * It is automatically added to the SourceMap.
	 * Does not guard against multiple insertions of the same file. */
	TranslationUnit& new_translation_unit(const std::string& path, const std::string& src);

public:
	SourceMap(ErrorHandler& handler) : handler(handler), translation_units() {}

	/* Load a file at a given path into the SourceMap.
	 * A reference to the new Translation Unit is returned.
	 * Does not guard against multiple insertions of the same file. */
	TranslationUnit& load_file(const std::string& path);

	/* The next free index in the SourceMap. */
	inline size_t next_start_pos() const {
		return translation_units.empty() ? 0 : (*translation_units.end())->end_pos();
	}

	/* A constant reference to the Translation Units in the SourceMap. */
	inline const std::vector<std::unique_ptr<TranslationUnit>>& trans_units() const { return translation_units; }
};