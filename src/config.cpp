#include "config.h"
#include <sys/stat.h>

#define CONFIG_DEBUG 1
int Config::counter = 0;

enum class States {
	SECTION_EXPECT,
	SECTION_NAME,
	PARAMS_EXPECT,
	PARAM_NAME,
	DELIM_EXPECT,
	PARAM_VALUE_EXPECT,
	PARAM_VALUE,
	SUCCESS,
};

std::string place_builder (int lines_cnt, int char_cnt, std::string filename) {
	return filename+":"+std::to_string (lines_cnt)+":"+std::to_string (char_cnt);
}

void Config::fatal_fail (int lines_cnt, int char_cnt, char c, int c_eof, std::string filename) {
	Printer::fatal(
		"Unexpected character", 
		place_builder (lines_cnt, char_cnt, filename), 
		{
			{"Character", to_string (c_eof)},
			{"Character code", std::to_string (c_eof)}
		}
	);
}

void Config::load_file (std::ifstream &config_file, std::string filename) {
	char c;
	char last_c = EOL;
	std::string section;
	config.clear ();
	int char_cnt = 0;
	int lines_cnt = 0;
	int c_eof;
	// Hash params;
	std::string param_name;
	std::string param_value;
	States state = States::SECTION_EXPECT;
	while (state != States::SUCCESS) {
		// read next character
		c_eof = config_file.get ();
		c = c_eof;
		char_cnt++;
		if (last_c == EOL) {
			lines_cnt++;
			char_cnt = 1;
		}
		last_c = c;
		switch (state) {
			case States::SECTION_EXPECT:

				#if CONFIG_DEBUG
				Printer::debug (to_string (c), "SECTION_EXPECT");
				#endif
				if (isspace (c))
					;
				else if (c == OPENING_BRACE) {
					state = States::SECTION_NAME;
					section.clear ();
				}
				else if (c_eof == std::char_traits<char>::eof()) {
					state = States::SUCCESS;
				}
				else {
					fatal_fail (lines_cnt, char_cnt, c, c_eof, filename);
				}
				break;
			case States::SECTION_NAME:
				#if CONFIG_DEBUG
				Printer::debug (to_string (c), "SECTION_NAME");
				#endif
				if (isalpha (c) || isdigit (c) || c == UNDERSCORE) {
					section.push_back (c);
					
				}
				else if (c == CLOSING_BRACE) {
					state = States::PARAMS_EXPECT;
					// params = config[section];
					#if CONFIG_DEBUG
					Printer::debug (section, "New section");
					#endif
				}
				else {
					fatal_fail (lines_cnt, char_cnt, c, c_eof, filename);
				}
				break;
			case States::PARAMS_EXPECT:
				#if CONFIG_DEBUG
				Printer::debug (to_string (c), "PARAMS_EXPECT");
				#endif
				if (isspace (c))
					;
				else if (c == OPENING_BRACE) {
					state = States::SECTION_NAME;
					section.clear ();
				}
				else if (isalpha (c) || isdigit (c) || c == UNDERSCORE) {
					state = States::PARAM_NAME;
					param_name.clear ();
					param_name.push_back (c);
				}
				else if (c_eof == std::char_traits<char>::eof()) {
					state = States::SUCCESS;
				}
				else {
					fatal_fail (lines_cnt, char_cnt, c, c_eof, filename);
				}
				break;
			case States::PARAM_NAME:
				#if CONFIG_DEBUG
				Printer::debug (to_string (c), "PARAM_NAME");
				#endif
				if (isalpha (c) || isdigit (c) || c == UNDERSCORE) {
					param_name.push_back (c);
				}
				else if (isspace (c)) {
					state = States::DELIM_EXPECT;
				}
				else if (c == DELIM) {
					state = States::PARAM_VALUE_EXPECT;
				}
				else {
					fatal_fail (lines_cnt, char_cnt, c, c_eof, filename);
				}
				break;
			case States::DELIM_EXPECT:
				#if CONFIG_DEBUG
				Printer::debug (to_string (c), "DELIM_EXPECT");
				#endif
				if (isspace (c))
					;
				else if (c == DELIM)
					state = States::PARAM_VALUE_EXPECT;
				else {
					fatal_fail (lines_cnt, char_cnt, c, c_eof, filename);
				}
				break;
			case States::PARAM_VALUE_EXPECT:
				#if CONFIG_DEBUG
				Printer::debug (to_string (c), "PARAM_VALUE_EXPECT");
				#endif
				if (isspace (c))
					;
				else if (c == QUOTES) {
					state = States::PARAM_VALUE;
					param_value.clear ();
				}
				else {
					fatal_fail (lines_cnt, char_cnt, c, c_eof, filename);
				}
				break;
			case States::PARAM_VALUE:
				#if CONFIG_DEBUG
				Printer::debug (to_string (c), "PARAM_VALUE");
				#endif
				if (c == QUOTES) {
					state = States::PARAMS_EXPECT;
					config[section][param_name] = param_value;
					#if CONFIG_DEBUG
					Printer::debug ("", section, config[section]);
					#endif
				}
				else if (c == EOL) {
					Printer::note("Odd EOL character", place_builder (lines_cnt, char_cnt, filename));
				}
				else if (c_eof == std::char_traits<char>::eof()) {
					fatal_fail (lines_cnt, char_cnt, c, c_eof, filename);
				}
				else {
					param_value.push_back (c);
				}
				break;
		}
	}

}

Config::Config (std::ifstream &config_file, std::string filename) {
	load_file (config_file, filename);
}

Config &Config::load (const std::string &filename) {
	// input file stream
	std::ifstream config_file;
	if (counter == 0) {
		// load file for the first time
		config_file.open (filename);
		if (!config_file.is_open ()) {
			Printer::fatal (filename, "Could not open configuration file");
		}
		#if CONFIG_DEBUG
		Printer::debug (filename, "Found config file");
		#endif
		counter = 1;
	}
	static Config cfg (config_file, filename);
	return cfg;
}

Hash & Config::operator[] (const std::string &section) {
	return config.at(section);
}

Hash &Config::section (std::string section_name) {
	return Config::get()[section_name];
}
#if CONFIG_DEBUG
void Config::out () const {
	for (const auto &section: config) {
		#if CONFIG_DEBUG
		Printer::debug ("", std::string ("[") + section.first + std::string ("]"), section.second);
		#endif
	}
}
#endif
