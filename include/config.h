#pragma once

#include <string>
#include <fstream>
#include <unordered_map>

#include "printer.hpp"

class Config {
	std::unordered_map <std::string, Hash> config;
	static int counter;

	static const char OPENING_BRACE = '[';
	static const char CLOSING_BRACE = ']';
	static const char QUOTES = '"';
	static const char SPACE = ' ';
	static const char EOL = '\n';
	static const char TAB = '\t';
	static const char DELIM = '=';
	static const char UNDERSCORE = '_';


	Config (std::ifstream &config_file);
	Config (Config &) = delete;
	Config (Config &&) = delete;
	#if DEBUG
	std::string to_string (char c) { 
		if (isspace (c)) {
			switch (c) {
				case SPACE: return std::string ("SPACE");
				case TAB: return std::string ("TAB");
				case EOL: return std::string ("EOL");
			}
		}
		else if (c == std::char_traits<char>::eof()) 
			return std::string ("EOF");
		return std::string(1, c);
	}
	#endif
	void load_file (std::ifstream &config_file);
public:
	Hash &operator[] (const std::string &section);
	static Config &load (const std::string &filename);
	static Config &get () { return load (""); }
	static Hash &section (std::string);
	#if DEBUG
	void out () const;
	#endif
};
