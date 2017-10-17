#include "config.h"
#include <sys/stat.h>

#define CONFIG_DEBUG 0
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
void Config::load_file (std::ifstream &config_file) {
	char c;
	std::string section;
	config.clear ();
	int char_cnt = 0;
	int lines_cnt = 1;
	int char_eof;
	// Hash params;
	std::string param_name;
	std::string param_value;
	States state = States::SECTION_EXPECT;
	while (state != States::SUCCESS) {
		// read next character
		char_eof = config_file.get ();
		c = char_eof;
		char_cnt++;
		if (c == '\n') {
			lines_cnt++;
			char_cnt = 0;
		}
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
				else if (char_eof == std::char_traits<char>::eof()) {
					state = States::SUCCESS;
				}
				else {
					Printer::error ("Unexpected character", "SECTION_EXPECT", {{"Character", to_string (c)}});
					throw std::string ("Unexpected character: ") + to_string (c);
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
					Printer::error ("Unexpected character", "SECTION_NAME", {{"Character", to_string (c)}});
					throw std::string ("Unexpected character: ") + to_string (c);
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
				else if (char_eof == std::char_traits<char>::eof()) {
					state = States::SUCCESS;
				}
				else {
					int k = int(c);
					std::string foo = std::to_string(k);
					//Printer::fatal("Foobar!");
					Printer::debug(foo,"This is the character");
					Printer::error ("Unexpected character", "PARAMS_EXPECT", {{"Character", to_string (c) + ", has "+foo+" character code, at "+std::to_string(char_cnt)+" place in file, at "+std::to_string(lines_cnt)+" line, while EOF="+std::to_string(std::char_traits<char>::eof())}});
					throw std::string ("Unexpected character: ") + to_string (c) + ", has "+to_string(int(c))+"character code";
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
					Printer::error ("Unexpected character", "PARAM_NAME", {{"Character", to_string (c)}});
					throw std::string ("Unexpected character: ") + to_string (c);
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
					Printer::error ("Unexpected character", "DELIM_EXPECT", {{"Character", to_string (c)}});
					throw std::string ("Unexpected character: ") + to_string (c);
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
					Printer::error ("Unexpected character", "PARAM_VALUE_EXPECT", {{"Character", to_string (c)}});
					throw std::string ("Unexpected character: ") + to_string (c);
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
				else {
					param_value.push_back (c);
				}
				break;
		}
	}

}

Config::Config (std::ifstream &config_file) {
	load_file (config_file);
}

Config &Config::load (const std::string &filename) {
	// input file stream
	std::ifstream config_file;
	if (counter == 0) {
		// load file for the first time
		config_file.open (filename);
		if (!config_file.is_open ()) {
			Printer::error (filename, "Could not open configuration file");
			throw std::string ("Could not open configuration file");
		}
		#if CONFIG_DEBUG
		Printer::debug (filename, "Found config file");
		#endif
		counter = 1;
	}
	static Config cfg (config_file);
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
