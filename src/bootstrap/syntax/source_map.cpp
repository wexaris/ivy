#include "source_map.hpp"

std::string FileLoader::read_file(const std::string& path) {
	// Open the file at the path
	std::ifstream fs(path);

	// If it opens successfully, read it to a string
	if (fs.good()) {
		std::string str;

		// Initialize string size to file length
		fs.seekg(0, std::ios::end);
		str.resize(fs.tellg());
		fs.seekg(0, std::ios::beg);

		// Read to the string
		fs.read(&str[0], str.size());

		return(str);
	}

	std::string msg = std::string("Failed to open a file at ") + path;
	throw std::runtime_error(msg);
}

std::string FileLoader::name_from_path(const std::string& path) {
	std::string name;
	// Build the name of the file
	// If encounter a '/', start over
	for(size_t i = 0; i < path.length(); i++) {
		name += path[i];
		if(path[i] == '/')
			name.clear();
	}
	return name;
}

SourceFile* SourceMap::new_source_file(std::string path, std::string src) {
	// Create unique_ptr to a new SourceFile in the file vector
	source_files.push_back(std::unique_ptr<SourceFile>(new SourceFile(path, src, next_start_pos())));
	// Return the managed pointer
	return source_files.back().get();
}

SourceFile* SourceMap::load_file(const std::string& path) {
	// Return text from file, if it opens
	if (FileLoader::file_exists(path))
		return new_source_file(path, FileLoader::read_file(path));

	std::string msg = std::string("Failed to open a file at ") + path;
	throw std::runtime_error(msg);
}