#include <sstream>

#include "client.h"
#include "helpers.h"
#include "config.h"
#include "filestat.h"


int Client::_clients_num = 0;

vector<string> Client::fill_cgi_env (Hash &request) const {
	Hash cgi_env = {
		{"CONTENT-TYPE", "text/plain"},
		{"GATEWAY-INTERFACE", Config::section("internal")["gateway_interface"]}, 
		{"REMOTE_ADDR", _address.getIp()},
		{"REMOTE_PORT", std::to_string (_address.getPort())},
		{"QUERY_STRING", request["Params"]},
		{"SERVER_ADDR", Config::section("network")["server_ip"]},
		{"SERVER_NAME", Config::section("network")["server_name"]},
		{"SERVER_PORT", Config::section("network")["server_port"]},
		{"SERVER_PROTOCOL", Config::section("internal")["server_protocol"]},
		{"SERVER_SOFTWARE", Config::section("internal")["server_software"]},
		{"SCRIPT_NAME", request["Path"]},
		{"SCRIPT_FILENAME", request["Path"]},
		{"DOCUMENT_ROOT", Directory::ROOT},
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
		// парсим заголовки запроса
		Hash request = parse_request (_http_request);
		Printer::debug ("",string("Запрос клиента №")+to_string(_clientNo), request);
		if (request["Method"] == "POST") {
			_http_request.erase(0, _http_request.find("\r\n\r\n") + 4);
			if(_http_request.size() > 0)
				return true;
			else
				return false;
		}
		else if (request["Method"] != "GET" && request["Method"] != "HEAD") {
			// не поддерживается
			_clientSocket.sendString (Response::response_4xx_5xx (501, request["Method"]));
			throw ClientException (Response::response_body (501));
		}
		else {
			// получаем путь
			string path = request["Path"];
			// заменяем / на default page
			if (path == "/") {
				path = Config::section ("network")["default_page"];
			}
			Directory::remove_first_slash (path);
			Printer::note (path, "Path changed to");
			try {
				FileStat file_attrib (path);
				if(!file_attrib.isReadable ())
					throw FileException(path, EACCES);
				time_t modif_date = file_attrib.getModifDate ();
				string resp_head, resp_body;
				if (request["Method"] == "GET") {
					if (file_attrib.isDirectory ()) {
						// Directory listing
						resp_body = HtmlHelpers::htmlDirList (path);
						resp_head = Response::response_200 ("GET", modif_date, "text/html", resp_body.size());
						_clientSocket.sendString (resp_head);
						_clientSocket.sendString (resp_body);
					}
					else if(path.find(Config::section ("internal")["script_folder"]) != string::npos) {
						// Launch script
						vector <string> cgi_argv = {path};
						vector <string> cgi_env = fill_cgi_env (request);
						_client_progs.push_back(CgiProg(cgi_argv[0], cgi_env, cgi_argv));
					}
					else {
						// Send file
						_clientSocket.sendString (Response::response_200 ("GET", modif_date, file_attrib.getMimeType (), file_attrib.getSize ()));
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
			_clientSocket.sendString (Response::response_200 ("GET", file_stat.getModifDate (), "text/html", file_stat.getSize ()));
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

