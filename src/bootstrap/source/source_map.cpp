#include "source_map.hpp"

#include <memory>

bool FileLoader::file_exists(const std::string& path) {
	std::ifstream f (path, std::ios::in | std::ios::binary);
	return f.good();
}

std::string FileLoader::read_file(const std::string& path) {
	// Open the file at the path
	std::ifstream fs(path);

	// If it opens successfully, read it to a string
	if (fs.good()) {
		std::string str;

		// Initialize string size to file length
		fs.seekg(0, std::ios::end);
		str.resize((unsigned int)fs.tellg());
		fs.seekg(0, std::ios::beg);

		// Read to the string
		fs.read(&str[0], str.size());

		fs.close();
		return(str);
	}

	std::string msg = std::string("Failed to open file ") + path;
	throw std::runtime_error(msg);
}

TranslationUnit& SourceMap::new_translation_unit(const std::string& path, const std::string& src) {
	// Create unique_ptr to a new Translation Unit in the file vector
	translation_units.push_back(std::make_unique<TranslationUnit>(path, src, next_start_pos()));
	// Return the managed pointer
	return *translation_units.back().get();
}

TranslationUnit& SourceMap::load_file(const std::string& path) {
	// Return text from file, if it opens
	if (FileLoader::file_exists(path))
		return new_translation_unit(path, FileLoader::read_file(path));

	std::string msg = std::string("Failed to open a file at ") + path;
	throw std::runtime_error(msg);
}