#include "config.h"

#include <string>
#include <sys/time.h>

using namespace std;

class DateTime {
public:
	static string getGmtDate(time_t time_in_sec = -1);
};

class HeaderField {
public:
	static const string DELIM;
	static string server_field () {
		return string ("Server: ") + Config::section("internal")["server_software"] + DELIM;
	}
	static string allow_field () {
		return string ("Allow: ") + Config::section("internal")["supported_methods"] + DELIM;
	}
	static string date_field () {
		return string ("Date: ") + DateTime::getGmtDate () + DELIM;
	}
	static string content_type_field (string type) {
		return string ("Content-type: ") + type + DELIM;
	}
	static string content_length_field (int content_length) {
		return string ("Content-length: ") + std::to_string (content_length) + DELIM;
	}
	static string last_modified_field (time_t modif_date) {
		return string ("Last-modified: ") + DateTime::getGmtDate(modif_date) + DELIM;
	}
};

class Response {
public:
	static string response_code (int errcode);
	static string response_body (int errcode);
	static string response_200 (string method, time_t modif_date, string content_type = "", int content_length = 0);
	static string response_4xx_5xx (int errcode, string method);
};