#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>

#include "config.h"
#include "helpers.h"
#include "filestat.h"
#include "embedded.h"
using namespace std;

const string HeaderField::DELIM = "\r\n";
const string HeaderField::CUSTOM_HEADER = "X-Custom";

const string Methods::GET_METHOD = "GET";
const string Methods::HEAD_METHOD = "HEAD";


string Response::response_code (int errcode) {
	string code;
	switch(errcode)
	{
		case 200:
		{
			code = "200 OK";
			break;
		}
		case 206:
		{
			code = "206 Partial Content";
			break;
		}
		case 400:
		{
			code = "400 Bad Request";
			break;
		}
		case 401:
		{
			code = "401 Unauthorized";
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
		case 416:
		{
			code = "416 Range Not Satisfiable";
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
		case 401:
		{
			info = "You need to pass authentication process\n";
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
		case 416:
		{
			info = "Requested range not satisfiable\n";
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

string Response::response_200 (string method, time_t modif_date, string content_type, 
		int content_length, bool isDirectory, bool isApi) {
	/*
	Date, Server,
	Content-type-для GET, 
	Content-length – для GET,
	Content-Range - "bytes 21010-47021/47022"
	Last-modified,
	тело – для GET
	*/
	stringstream resp;
	resp << Config::section("internal")["server_protocol"] << " " << response_code (200) << HeaderField::DELIM;
	resp << HeaderField::date_field ();
	resp << HeaderField::server_field ();
	if (method == Methods::GET_METHOD) {
		if (!isDirectory)
			resp << HeaderField::range_field ();
		resp << HeaderField::content_type_field (content_type);
		resp << HeaderField::content_length_field (content_length);
	}
	// if (isApi)
	resp << HeaderField::expires_now_field ();
	resp << HeaderField::last_modified_field (modif_date);
	resp << HeaderField::DELIM;
	return resp.str();
}
string Response::response_206 (time_t modif_date, string content_type, int content_length,
																size_t file_size, size_t first_byte_pos, size_t last_byte_pos) {
	/*
	Date, Server,
	Content-type-для GET, 
	Content-length – для GET,
	Last-modified,
	тело – для GET
	*/
	stringstream resp;
	resp << Config::section("internal")["server_protocol"] << " " << response_code (206) << HeaderField::DELIM;
	resp << HeaderField::date_field ();
	resp << HeaderField::server_field ();
	resp << HeaderField::content_range_field (file_size, first_byte_pos, last_byte_pos);
	resp << HeaderField::content_type_field (content_type);
	resp << HeaderField::content_length_field (content_length);
	resp << HeaderField::last_modified_field (modif_date);
	resp << HeaderField::DELIM;
	return resp.str();
}
string Response::response_4xx_5xx (int errcode, string method, size_t file_size) {
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
	if (errcode == 416) {
		resp << HeaderField::content_range_field (file_size);
	}
	if (errcode == 401) {
		resp << HeaderField::authenticate_field ();
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
	return string ("<a href='")+pointer+"'>"+name+"</a>";
}
string HtmlHelpers::header (string name) {
	string head;
  	head += "<head>\n";
    head += "<meta charset='utf-8'>\n";
    head += string ("<title>")+name+"</title>\n";
  	head += string ("<style media='screen' type='text/css'>")+div_image(folder_base64, ".folder")+"</style>\n";
  	head += string ("<style media='screen' type='text/css'>")+div_image(file_base64, ".file")+"</style>\n";
  	head += string ("<style media='screen' type='text/css'>")+div_image(back_base64, ".back")+"</style>\n";
  	head += string ("<style media='screen' type='text/css'>")+bootstrap_min_css+"</style>\n";
  	head += string ("<style media='screen' type='text/css'>")+bootstrap_sortable_css+"</style>\n";
  	head += string ("<link rel='shortcut icon' type='image/png' href='data:image/png;base64,")+favicon_base64+"'/>\n";
  	head += string ("<script>")	+jquery_min_js					+"</script>\n";
  	head += string ("<script src='https://unpkg.com/vue'></script>");
  	head += string ("<script src='https://cdn.jsdelivr.net/npm/vue-resource@1.3.4'></script>");
  	head += string ("<script>")	+bootstrap_min_js				+"</script>\n";
  	head += string ("<script>")	+myscript_js						+"</script>\n";
  	head += string ("<script>")	+bootstrap_sortable_js	+"</script>\n";
  	head += string ("<script>")	+moment_min_js					+"</script>\n";
  	head += "</head>\n";
  	return head;
}
string HtmlHelpers::table (const vector <string> &header, vector <Hash> &rows) {
	string table = "<table class='table table-hover sortable'>\n";
	table += "	  <thead>\n";
	table += "	 	  <tr>\n";
	for (auto &th: header)
		table += string ("			<th class='col-xs-1'>") + th + string ("</th>\n");
	table += "		  </tr>\n";
	table += "	  </thead>\n";
	table += "	<tbody>\n";
	for (auto &row: rows) {
		table += "		<tr class='clickable-row' data-href='"+row["item-link"]+"'>\n";
		table += string ("			<td class='col-xs-1' data-value='")+row["item-rank"]+"'>"+
			row["item-pic"]+"</td>\n";
		table += string ("			<td class='col-xs-6' data-value='") +
			row["item-name"]+"'> <a href='"+row["item-link"]+"'>"+row["item-name"]+"</a> </td>\n";
		table += string ("			<td class='col-xs-2' data-value='"+row["item-modif-date"]+"'>") + 
			row["item-hr-modif-date"]+"</td>\n";
		table += string ("			<td class='col-xs-2' data-value='"+row["item-size"]+"'>") + 
			row["item-hr-size"]+"</td>\n";
		table += "		</tr>\n";
	}
	table += "	</tbody>\n";
	table += "</table>\n";
	return table;
}
string HtmlHelpers::img (const string &src, string text) {
	return string ("<img src='")+src+"' alt='"+text+"'>";
}
string HtmlHelpers::img64 (const string image_64, string text) {
	return string ("<img src='data:image/png;base64,")+image_64+"' alt='"+text+"'>";
}

string HtmlHelpers::div_image (const string image_64, string class_name) {
	return class_name+string(" {width:30px;height:30px;background:url(data:image/png;")+
		string("base64,")+image_64+");}";
}

string HtmlHelpers::dir_to_table (const string &dir_path) {
	const string NO_INFO = "-";
	const string UP_NAME = "Up";
	const string ZERO_SIZE = "0";
	const string BACK_PIC = "<div class='back'></div>";
	const string FOLDER_PIC = "<div class='folder'></div>";
	const string FILE_PIC = "<div class='file'></div>";
	const string PARENT_RANK = "0";
	const string FOLDER_RANK = "1";
	const string FILE_RANK = "2";

	vector <FileStat> files = Directory::ls (dir_path);
	vector <Hash> entries;
	// Table row example:
	//		item-rank => Back - 0, Folder - 1, File - 2
	// 		item-pic => <div class="back"></div>
	// 		item-link => /Downloads/foobar.txt
	// 		item-name => foobar.txt
	// 		item-hr-modif-date => Tue, 26 Sep 18:25
	// 		item-modif-date => 1508679866
	// 		item-hr-size => 2.37 МБ
	// 		item-size => 2486173
	if (dir_path != Directory::ROOT) {
		entries.push_back ({
			{"item-rank", PARENT_RANK},
			{"item-pic", BACK_PIC},
			{"item-link", UrlEncoder::url_encode(files.front ().getPath (),false)},
			{"item-name", UP_NAME},
			{"item-hr-modif-date", NO_INFO},
			{"item-modif-date", std::to_string (files.front ().getModifDate ())},
			{"item-hr-size", NO_INFO},
			{"item-size", ZERO_SIZE},
		});
	}
	for (size_t i = 1; i < files.size (); i++) {
		string sendto = link (files[i].getPath (), files[i].getName ());
		if (files[i].isDirectory ()) {
			string icon = "<a href='"+files[i].getPath ()+"'><div class='folder'></div></a>";
			entries.push_back ({
				{"item-rank", FOLDER_RANK},
				{"item-pic", FOLDER_PIC},
				{"item-link", UrlEncoder::url_encode(files[i].getPath (),false)},
				{"item-name", files[i].getName ()},
				{"item-hr-modif-date", files[i].hrModifDate ()},
				{"item-modif-date", std::to_string (files[i].getModifDate ())},
				{"item-hr-size", NO_INFO},
				{"item-size", ZERO_SIZE}
			});
		}
		else {
			string icon = "<a href='"+files[i].getPath ()+"'><div class='file'></div></a>";
			entries.push_back ({
				{"item-rank", FILE_RANK},
				{"item-pic", FILE_PIC},
				{"item-link", UrlEncoder::url_encode(files[i].getPath (),false)},
				{"item-name", files[i].getName ()},
				{"item-hr-modif-date", files[i].hrModifDate ()},
				{"item-modif-date", std::to_string (files[i].getModifDate ())},
				{"item-hr-size", FileStat::pp_size (files[i].getSize ())},
				{"item-size", std::to_string (files[i].getSize ())}
			});
		}
	}
	return table ({"", "Имя", "Изменено", "Размер"}, entries);
}
string HtmlHelpers::htmlDirList (const string &dir_path) {

	// HTML Header
	string html_page = 
	"<!DOCTYPE html>\n"
	"<html lang='en'>\n";
	html_page += header (string("Index of ")+dir_path.substr(1, string::npos));

	// HTML Body
	html_page += "<body>\n";
	html_page += "<div class='container' id='app'>\n";
	html_page += "<div class='row'>\n";
	html_page += "<div class='col-md-1'></div>\n";
	html_page += "<div class='col-md-10'>\n";
	html_page += string("	<h3>")+"Index of "+dir_path.substr(1, string::npos)+"</h1>\n";

	// create Bootstrap table
  	html_page += dir_to_table (dir_path);

	//Apache Server at cmcstuff.esyr.org Port 80
	html_page += 	string("	<address style='font-style:italic'>")+Config::section("internal")["server_software"]+" at "+
					Config::section("network")["server_name"]+" Port "+
					Config::section("network")["server_port"]+"</address>";
	html_page += "</div>\n";
	html_page += "<div class='col-md-1'></div>\n";
	html_page += "</div>\n";
	html_page += "</div>\n";

	// close HTML body
	html_page += "</body>\n";
	html_page += "</html>\n";

	return html_page;
}
