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


	Config (std::ifstream &config_file, std::string filename);
	Config (Config &) = delete;
	Config (Config &&) = delete;
	#if DEBUG
	std::string to_string (int c) { 
		if (isspace (c)) {
			switch (c) {
				case SPACE: return std::string ("SPACE");
				case TAB: return std::string ("TAB");
				case EOL: return std::string ("EOL");
			}
		}
		else if (c == std::char_traits<char>::eof()) 
			return std::string ("EOF");
		return std::string(1, char(c));
	}
	#endif
	void fatal_fail (int lines_cnt, int char_cnt, char c, int c_eof, std::string filename);
	void load_file (std::ifstream &config_file, std::string filename);
public:
	Hash &operator[] (const std::string &section);
	static Config &load (const std::string &filename);
	static Config &get () { return load (""); }
	static Hash &section (std::string);
	#if DEBUG
	void out () const;
	#endif
};
