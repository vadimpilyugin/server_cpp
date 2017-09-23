#include "config.h"
#include "datetime.h"
#include "excp.h"

#include <vector>
using namespace std;

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
	static string range_field () {
		return string ("Accept-Ranges: bytes") + DELIM;
	}
	static string content_range_field (size_t file_size, int first_byte_pos = -1, int last_byte_pos = -1) {
		// если ни начало, ни конец не указаны
		if (first_byte_pos == -1 && last_byte_pos == -1) {
			// это для ошибки 416
			return string ("Content-Range: bytes */")+to_string(file_size) + DELIM;
		}
		else if (first_byte_pos != -1 && last_byte_pos != -1) {
			// это для обычного 206
			return string ("Content-Range: bytes ")+
				to_string(first_byte_pos)+
				"-"+
				to_string(last_byte_pos)+
				"/"+
				to_string(file_size) + DELIM;
		}
		else {
			// это сигнализирует об ошибке
			Printer::error (
				string ("Content-Range: bytes ")+
				to_string(first_byte_pos)+
				"-"+
				to_string(last_byte_pos)+
				"/"+
				to_string(file_size),
				"[content_range_field]"
			);
			throw ServerException ("Каким-то образом в content-range попал полуинтервал");
		}
	}
	static string content_type_field (string type) {
		return string ("Content-Type: ") + type + DELIM;
	}
	static string content_length_field (int content_length) {
		return string ("Content-Length: ") + std::to_string (content_length) + DELIM;
	}
	static string last_modified_field (time_t modif_date) {
		return string ("Last-Modified: ") + DateTime::getGmtDate(modif_date) + DELIM;
	}
};

class Response {
public:
	static string response_code (int errcode);
	static string response_body (int errcode);
	static string response_200 (string method, time_t modif_date, string content_type = "", int content_length = 0);
	static string response_206 (	time_t modif_date, string content_type, int content_length,
																size_t file_size, size_t first_byte_pos, size_t last_byte_pos);
	static string response_4xx_5xx (int errcode, string method, size_t file_size = 0);
};

class UrlEncoder {
public:
	// Encodes the URL using %xx notation
    static string url_encode(const string &value, bool encode_slashes = true);
    // decodes the URL
    static string url_decode(const string &value);
private:
	// returns a character represented by 2 hex digits
    static char to_c (char c1, char c2);
};

class HtmlHelpers {
public:
	// returns a string with html <a> tag
	static string link (string src, string descr);
	// returns a string with filled <head>
	static string header (string name);
	// returns a string with html table with bootstrap styles
	static string table (const vector <string> &thead, const vector <vector <string> > &tbody);
	// returns an <img> tag
	static string img (const string &src, string text = "No picture");
	// returns an html table filled with directory files
	static string dir_to_table (const string &dir_path);
	// returns a string containing html document which lists all files in a directory
	static string htmlDirList (const string &dir_path);
};