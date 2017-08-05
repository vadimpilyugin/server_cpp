#include "client.h"
#include "helpers.h"
#include "config.h"
#include "filestat.h"
#include "dirent.h"

#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

int Client::_clients_num = 0;

class HtmlHelpers {
public:
	static string link (string pointer, string name) {
		return string ("<a href=\"") + pointer + string ("\">") + name + string ("</a>");
	}
	static string header (string name) {
		string head;
	  	head += "<head>\n";
	    head += "<meta charset=\"utf-8\">\n";
	    head += string ("<title>")+name+"</title>\n";
	  	head += "<link rel=\"stylesheet\" href=\"/css/bootstrap.min.css\">\n";
	  	head += "<link rel=\"stylesheet\" href=\"/css/style.css\">\n";
	  	head += "<script type=\"text/javascript\" src=\"/js/jquery.min.js\"></script>\n";
	  	head += "<script type=\"text/javascript\" src=\"/js/bootstrap.min.js\"></script>\n";
	  	head += "</head>\n";
	  	return head;
	}
	static string table (vector <string> &thead, vector <vector <string> > &tbody) {
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
};

class UrlEncoder {
public:
    static string url_encode(const string &value) {
        ostringstream escaped;
        escaped.fill('0');
        escaped << hex;

        for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
            string::value_type c = (*i);

            // Keep alphanumeric and other accepted characters intact
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
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
    static string url_decode(const string &value) {
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
        // if (unescaped.str ().empty ())
        //     cout << "No characters!" << endl;
        // else
        //     cout << "Size = " << unescaped.str ().size () << endl;
        return unescaped.str();
    }
private:
    static char to_c (char c1, char c2) {
        string hexed = "00";
        hexed[0] = c1;
        hexed[1] = c2;
        int n;
        std::istringstream(hexed) >> std::hex >> n;
        // cout << "Decoded character: |" << char (n) << "|" << endl;
        return char(n);
    }
};

class Directory {
public:
	static vector <string> dirls (string dir_path) {
		string parent_dir;
		if (dir_path == "/") {
			parent_dir = "/";
		}
		else {
			// if the last character is '/'
			// then drop it
			if (dir_path.back () == '/')
				dir_path.pop_back ();
			if (dir_path.find_last_of ('/') == string::npos) {
				parent_dir = ".";
			}
			else
				parent_dir = dir_path.substr (0, dir_path.find_last_of ('/'));
		}
		Printer::debug ("", "Listing directory", {	{"The directory", dir_path}, 
													{"Its parent", parent_dir} });
		vector <string> result;
		DIR *dir;
		struct dirent *ent;
		// push back parent directory
		result.push_back (string("/") + parent_dir);
		if ((dir = opendir (dir_path.c_str ())) != NULL) {
			while ((ent = readdir (dir)) != NULL) {
				if (string (ent->d_name) == ".") {
					;
				}
				else if (string (ent->d_name) == "..") {
					;
				}
				else if (ent->d_name[0] == '.') {
					;
				}
				else {
					// result.push_back (string("/")+dir_path+(dir_path.back () == '/' ? "" : "/") + ent->d_name);
					result.push_back (ent->d_name);
				}
			}
			std::sort (result.begin ()+1, result.end ());
			return result;
		}
		else {
			/* could not open directory */
		  	Printer::error (dir_path, "Could not open directory", {{"Directory", dir_path}});
		  	throw FileException ("Could not open directory");
		}
	}
	static string pure_name (string path) {
		if (path == "/")
			return path;
		if (path.back () == '/') {
			path.pop_back ();
		}
		size_t last_pos = path.find_last_of ('/');
		return path.substr (last_pos + 1);
	}
};



string Client::writeDirContent(string dir_path) const {
	// HTML Header
	string dir_content = "<!DOCTYPE html>\n";
	dir_content += "<html lang=\"en\">\n";
	dir_content += HtmlHelpers::header (string("Directory listing ")+dir_path);
	// HTML Body
	dir_content += "<body>\n";
	dir_content += "	<div class=\"container\">\n";
	// Table header
	vector <string> header = {"Filename"};
	// Table entries
	vector <vector <string> > entries;
	// File names in directory
	vector <string> list = Directory::dirls (dir_path);
	// first file name is parent directory
	// if at top level
	if (dir_path == "/" || dir_path == "./" || dir_path == ".") {
		;
	}
	else {
		entries.push_back ({HtmlHelpers::link (list.front (), "Up")});
	}
	list.erase (list.begin ());
	// push all other file names into the table
	for (auto &i: list) {
  		entries.push_back ({HtmlHelpers::link (string("/")+dir_path+(dir_path.back () == '/' ? "" : "/") + UrlEncoder::url_encode(i), i)});
	}
	// create Bootstrap table
  	dir_content += HtmlHelpers::table (header, entries);
	// close HTML body
	dir_content += "	</div>\n";
  	dir_content += "</body>\n";
  	dir_content += "</html>\n";
	return dir_content;
}

vector<string> Client::fill_cgi_env (Hash &request) const {
	Hash cgi_env = {
		{"CONTENT-TYPE", "text/plain"},
		{"GATEWAY-INTERFACE", Config::section("internal")["gateway_interface"]}, 
		{"REMOTE_ADDR", _address.getIp()},
		{"REMOTE_PORT", std::to_string (_address.getPort())},
		{"QUERY_STRING", request["Params"]},
		{"SERVER_ADDR", Config::section("network")["ip_address"]},
		{"SERVER_NAME", Config::section("network")["server_name"]},
		{"SERVER_PORT", Config::section("network")["server_port"]},
		{"SERVER_PROTOCOL", Config::section("internal")["server_protocol"]},
		{"SERVER_SOFTWARE", Config::section("internal")["server_software"]},
		{"SCRIPT_NAME", request["Path"]},
		{"SCRIPT_FILENAME", FileStat::getFullPath(request["Path"])},
		{"DOCUMENT_ROOT", FileStat::getCwd()},
		{"HTTP_USER_AGENT", request["User-Agent"]},
		{"HTTP_REFERER", request["Referer"]}
	};
	vector <string> env;
	for (const auto &pair: cgi_env) {
		env.push_back (pair.first + std::string ("=") + pair.second);
	}
	return env;
}

Client::Client(int sd, SocketAddress &address): _clientSocket(sd), _address(address)
{
	_buffer = new char [_BUF_MAX];
	_clientNo = _clients_num++;
	Printer::debug (std::string ("получает номер ") + std::to_string (_clientNo),"Новый клиент", {
		{"IP", _address.getIp()},
		{"Port", std::to_string (_address.getPort())}
	});
}

Client::Client(Client &&that): _clientSocket(move(that._clientSocket)), _address(that._address)
{
	_http_request = move(that._http_request);
	_buffer = that._buffer;
	that._buffer = nullptr;
	_clientNo = that._clientNo;
}

Client::~Client()
{
	if(_buffer != nullptr)
	{
		delete [] _buffer;
		Printer::debug ("успешно отключен!", std::string ("Client[") + std::to_string (_clientNo) + "]");
		_clients_num --;
	}
}

void Client::appendRequest()
{
	int n = _clientSocket.receive(_buffer,_BUF_MAX - 1);
	_buffer[n] = 0;
	_http_request.append(_buffer);
}

Hash Client::parse_request (string request) {
	Hash result;
	string method, path, params, http_ver;
	stringstream request_stream;
	if (request.find ('?') != string::npos) {
		request[request.find ('?')] = ' ';
		request_stream.str (request);
		request_stream >> method >> path >> params >> http_ver;
	}
	else {
		request_stream.str (request);
		request_stream >> method >> path >> http_ver;
	}
	path = UrlEncoder::url_decode (path);
	Printer::note (path, "Decoded path");
	result["Method"] = method;
	result["Path"] = path;
	result["Params"] = params;
	result["HTTP version"] = http_ver;
	size_t begin_pos = request.find ('\n');
	size_t fin = request.find ("\r\n\r\n");
	Printer::debug (std::to_string (fin+1), "End position");
	if (begin_pos != fin+1)
		begin_pos++;
	while (begin_pos < fin+1) {
		string name, value;
		size_t delim_pos = request.find (':', begin_pos);
		size_t end_pos = request.find ('\r', begin_pos);
		name = request.substr (begin_pos, delim_pos - begin_pos);
		value = request.substr (delim_pos + 2, end_pos - (delim_pos + 2));
		result[name] = value;
		begin_pos = request.find ('\n', begin_pos);
		if (begin_pos != string::npos)
			begin_pos++;
		Printer::debug (std::to_string (begin_pos), "Begin pos");
	}
	return result;
}

bool Client::respond () {
	// принять новые данные от клиента
	appendRequest ();
	// если прием закончился
	if(isReceiveEnded ()) {
		Hash request = parse_request (_http_request);
		Printer::debug (_address.to_s (), "Запрос клиента", request);
		if (request["Method"] != "GET" && request["Method"] != "HEAD") {
			// не поддерживается
			_clientSocket.sendString (Response::response_4xx_5xx (501, request["Method"]));
			throw ClientException (Response::response_body (501));
		}
		string path = request["Path"];
		if (path == "/") {
			path = Config::section ("network")["default_page"];
		}
		else {
			// потому что иначе нельзя в linux, не поймут
			path.erase (path.begin ());
		}
		// path = Config::section("internal")["root_dir"] + path;
		Printer::note (path, "Path changed to");
		try {
			FileStat file_attrib (path);
			if(!file_attrib.isReadable ())
				throw FileException(path, EACCES);
			time_t modif_date = file_attrib.getModifDate ();
			string resp_head, resp_body;
			if (request["Method"] == "GET") {
				if (file_attrib.isDirectory ()) {
					// delete slash at the end of the path
					if (path.back () == '/')
						path.erase (path.size () - 1);
					// Directory listing
					resp_body = writeDirContent (path);
					resp_head = Response::response_200 ("GET", modif_date, "text/html", resp_body.size());
					_clientSocket.sendString (resp_head);
					_clientSocket.sendString (resp_body);
				}
				else if(path.find("cgi-bin/") != string::npos) {
					// Launch script
					vector <string> cgi_argv = {path};
					vector <string> cgi_env = fill_cgi_env (request);
					_client_progs.push_back(CgiProg(cgi_argv[0], cgi_env, cgi_argv));
				}
				else {
					// Send file
					_clientSocket.sendString (Response::response_200 ("GET", modif_date, file_attrib.getFileType (), file_attrib.getSize ()));
					_clientSocket.sendFile (path);
					
				}
			}
			else if (request["Method"] == "HEAD") {
				resp_head = Response::response_200 ("HEAD", modif_date);
				_clientSocket.sendString (resp_head);
			}
			_http_request.erase(0, _http_request.find("\r\n\r\n") + 4);
			if(_http_request.size() > 0)
				return true;
			else
				return false;
		}
		catch(FileException &a) {
			switch (a.getErrno ())
			{
				case EACCES: 
					_clientSocket.sendString (Response::response_4xx_5xx (403, request["Method"]));
					throw ClientException (Response::response_body (403));
				case ENOENT: 
					_clientSocket.sendString (Response::response_4xx_5xx (404, request["Method"]));
					throw ClientException (Response::response_body (404));
				default: 
					_clientSocket.sendString (Response::response_4xx_5xx (400, request["Method"]));
					throw ClientException (Response::response_body (400));
			}
		}
	}
	else
		return false;
}

void Client::checkCgiProgs()
{
	auto proc = _client_progs.begin ();
	while (proc != _client_progs.end ()) {
		if(proc -> isProcessEnded () && proc -> isExitedCorrectly ()) {
			// send header
			FileStat file_stat (proc -> getOutputFile ());
			_clientSocket.sendString (Response::response_200 ("GET", file_stat.getModifDate (), "text/plain", file_stat.getSize ()));
			// send file
			_clientSocket.sendFile (proc -> getOutputFile ());
			auto tmp = proc;
			proc++;
			_client_progs.erase (tmp);
		}
		else if(proc -> isProcessEnded () && !proc -> isExitedCorrectly ()) {
			// send header 500
			_clientSocket.sendString (Response::response_4xx_5xx (500, "GET"));
			auto tmp = proc;
			proc++;
			_client_progs.erase (tmp);
		}
		else
			proc++;
	}
}

