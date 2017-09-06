#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>

#include "config.h"
#include "helpers.h"
#include "filestat.h"
using namespace std;

// const string DELIM = "\r\n";
const string HeaderField::DELIM = "\r\n";

string Response::response_code (int errcode) {
	string code;
	switch(errcode)
	{
		case 200:
		{
			code = "200 OK";
			break;
		}
		case 400:
		{
			code = "400 Bad Request";
			break;
		}
		case 403:
		{
			code = "403 Forbidden";
			break;
		}
		case 404:
		{
			code = "404 Not Found";
			break;
		}
		case 500:
		{
			code = "500 Internal Server Error";
			break;
		}
		case 501:
		{
			code = "501 Not Implemented";
			break;
		}
		case 503:
		{
			code = "503 Service Unavailable";
			break;
		}
	}
	return code;
}

string Response::response_body (int errcode) {
	string info;
	switch(errcode)
	{
		case 400:
		{
			info = "This is a Bad Request\n";
			break;
		}
		case 403:
		{
			info = "You have no permission to access this directory or file\n";
			break;
		}
		case 404:
		{
			info = "The specified location is not found\n";
			break;
		}
		case 500:
		{
			info = "Server error. Service unavailable\n";
			break;
		}
		case 501:
		{
			info = "This method is not supported.\n";
			info += "The supported methods are: "+Config::section("internal")["supported_methods"]+"\n";
			break;
		}
		case 503:
		{
			info = "Too many clients. Service is Unavailable\n";
			break;
		}
	}
	return info;
}

string Response::response_200 (string method, time_t modif_date, string content_type, int content_length) {
	/*
	Date, Server,
	Content-type-для GET, 
	Content-length – для GET,
	Last-modified,
	тело – для GET
	*/
	stringstream resp;
	resp << Config::section("internal")["server_protocol"] << " " << response_code (200) << HeaderField::DELIM;
	resp << HeaderField::date_field ();
	resp << HeaderField::server_field ();
	if (method == "GET") {
		resp << HeaderField::content_type_field (content_type);
		resp << HeaderField::content_length_field (content_length);
	}
	resp << HeaderField::last_modified_field (modif_date);
	resp << HeaderField::DELIM;
	return resp.str();
}
string Response::response_4xx_5xx (int errcode, string method) {
	/*
	Date, Server,
	Content-type,
	Content-length,
	тело
	*/
	string body = response_body (errcode);
	int content_length = body.size ();
	string content_type = "text/plain";

	stringstream resp;
	resp << Config::section("internal")["server_protocol"] << " " << response_code (errcode) << HeaderField::DELIM;
	resp << HeaderField::date_field ();
	resp << HeaderField::server_field ();
	if (errcode == 501) {
		resp << HeaderField::allow_field ();
	}
	if (method == "GET") {
		resp << HeaderField::content_type_field (content_type);
		resp << HeaderField::content_length_field (content_length);
	}
	resp << HeaderField::DELIM;
	if (method == "GET") {
		resp << body;
	}
	return resp.str();
}

string UrlEncoder::url_encode(const string &value, bool encode_slashes) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || (c == '/' && !encode_slashes)) {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char) c);
        escaped << nouppercase;
    }

    return escaped.str();
}
string UrlEncoder::url_decode(const string &value) {
    ostringstream unescaped;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);
        // cout << "I can see |"<<c<<"| !" << endl;
        if (c == '%') {
            // read 2 hex digits
            char c1 = *(++i);
            char c2 = *(++i);
            unescaped << to_c (c1, c2);
        }
        else {
            unescaped << c;
        }
    }
    return unescaped.str();
}
char UrlEncoder::to_c (char c1, char c2) {
    string hexed = "00";
    hexed[0] = c1;
    hexed[1] = c2;
    int n;
    std::istringstream(hexed) >> std::hex >> n;
    return char(n);
}

string HtmlHelpers::HtmlHelpers::link (string pointer, string name) {
	pointer = UrlEncoder::url_encode (pointer, false);
	return string ("<a href=\"")+pointer+"\">"+name+"</a>";
}
string HtmlHelpers::header (string name) {
	string head;
  	head += "<head>\n";
    head += "<meta charset=\"utf-8\">\n";
    head += string ("<title>")+name+"</title>\n";
  	head += "<link rel=\"stylesheet\" href=\"/internal/css/bootstrap.min.css\">\n";
  	head += "<link rel=\"stylesheet\" href=\"/internal/css/style.css\">\n";
  	// head += "<link rel=\"stylesheet\" href=\"/internal/css/font-awesome.css\">\n";
  	// head += "<link rel=\"stylesheet\" href=\"/internal/css/font-awesome.min.css\">\n";
  	head += "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css\">";
  	head += "<script type=\"text/javascript\" src=\"/internal/js/jquery.min.js\"></script>\n";
  	head += "<script type=\"text/javascript\" src=\"/internal/js/bootstrap.min.js\"></script>\n";
  	head += "</head>\n";
  	return head;
}
string HtmlHelpers::table (const vector <string> &thead, const vector <vector <string> > &tbody) {
	string table = "<table class=\"table table-hover\">\n";
	table += "	  <thead>\n";
	table += "	 	  <tr>\n";
	for (auto &i: thead) {
		table += string ("			<th>") + i + string ("</th>\n");
	}
	table += "		  </tr>\n";
	table += "	  </thead>\n";
	table += "	<tbody>\n";
	for (auto &row: tbody) {
		table += "		<tr>\n";
		for (auto &elem: row) {
			table += string ("			<td>") + elem + string ("</td>\n");
		}
		table += "		</tr>\n";
	}
	table += "	</tbody>\n";
	table += "</table>\n";
	return table;
}
string HtmlHelpers::img (const string &src, string text) {
	return string ("<img src=\"")+src+"\" alt=\""+text+"\">";
}
string HtmlHelpers::dir_to_table (const string &dir_path) {
	vector <FileStat> files = Directory::ls (dir_path);
	vector <vector <string> > entries;
	if (dir_path != Directory::ROOT) {
		string sendto = link (files[0].getPath (), "Up");
		entries.push_back ({img ("/internal/img/back.gif"), sendto, files[0].hrModifDate (), "-"});
	}
	for (size_t i = 1; i < files.size (); i++) {
		string sendto = link (files[i].getPath (), files[i].getName ());
		if (files[i].isDirectory ()) {
			string icon = "<i class=\"fa fa-folder fa-2x \" style=\"color:#0099CC\"></i>";
			entries.push_back ({icon, sendto, files[i].hrModifDate (), "-"});
		}
		else {
			const size_t KB_IN_BYTES = 1024;
			string size_in_kb = to_string (files[i].getSize () / KB_IN_BYTES) + " КБ";
			string icon = "<i class=\"fa fa-file-o\" style=\"font-size:24px\"></i>";
			entries.push_back ({icon, sendto, files[i].hrModifDate (), size_in_kb});
		}
	}
	return table ({img ("/internal/img/blank.gif"), "Имя", "Изменено", "Размер"}, entries);
}
string HtmlHelpers::htmlDirList (const string &dir_path) {
	// HTML Header
	string html_page = 
	"<!DOCTYPE html>\n"
	"<html lang=\"en\">\n";
	html_page += header (string("Index of ")+dir_path.substr(1, string::npos));
	// HTML Body
	html_page += "<body>\n";
	html_page += "<div class=\"container\">\n";
	// html_page += "<div class=\"section\">\n";
	// html_page += "<div class=\"row\">\n";
	// html_page += "<div class=\"col-md-1\"></div>\n";
	html_page += "<div class=\"col-6\">\n";
	html_page += string("	<h1>")+"Index of "+dir_path.substr(1, string::npos)+"</h1>\n";
	// create Bootstrap table
  	html_page += dir_to_table (dir_path);
	//Apache Server at cmcstuff.esyr.org Port 80
	html_page += 	string("	<address style=\"font-style:italic\">")+Config::section("internal")["server_software"]+" at "+
					Config::section("network")["server_name"]+" Port "+
					Config::section("network")["server_port"]+"</address>";
	html_page += "\n";
	html_page += "</div>\n";
	// html_page += "<div class=\"col-md-3\"></div>\n";
	html_page += "</div>\n";
	// close HTML body
  	html_page += "</body>\n";
  	html_page += "</html>\n";
	return html_page;
}
