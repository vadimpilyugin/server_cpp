#include <sstream>

#include "client.h"
#include "helpers.h"
#include "config.h"


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
	_buf = new char [BUF_LEN];
	_clientNo = _clients_num++;
	isSending = false;
	file_to_send = nullptr;
	isResponseNeeded = false;
	Printer::debug (std::string ("получает номер ") + std::to_string (_clientNo),"Новый клиент", {
		{"IP", _address.getIp()},
		{"Port", std::to_string (_address.getPort())}
	});
}

// Client::Client(Client &&that): _clientSocket(move(that._clientSocket)), _address(that._address)
// {
// 	_http_request = move(that._http_request);
// 	_buffer = that._buffer;
// 	that._buffer = nullptr;
// 	_clientNo = that._clientNo;
// }

Client::~Client()
{
	if(_buf != nullptr)
	{
		delete [] _buf;
		Printer::debug ("успешно отключен!", std::string ("Client[") + std::to_string (_clientNo) + "]");
		_clients_num --;
	}
}

void Client::appendRequest()
{
	int n = _clientSocket.receive(_buf,BUF_LEN - 1);
	_buf[n] = 0;
	_http_request.append(_buf);
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
	// Printer::debug (std::to_string (fin+1), "End position");
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
		// Printer::debug (std::to_string (begin_pos), "Begin pos");
	}
	return result;
}

void Client::respond_or_send () {
	// если не посылаем файл
	if (!isSending) {
		// если не нужно читать запрос
		// if (!isResponseNeeded) {
		// 	// то что мы вообще здесь делаем?
		// 	Printer::error ("мы отвечаем на запрос, которого нет", "[Client::respond_or_send]");
		// 	throw ClientException ("Мы отвечаем на запрос, которого нет");
		// }
		// 
		// отвечаем на запрос
		bool data_left = respond ();
		// если считали запрос + какую-то часть
		if (data_left)
			// говорим, что нужна дополнительная обработка
			isResponseNeeded = true;
		else
			isResponseNeeded = false;
	}
	// если закончили посылать файл
	else if (file_to_send -> is_finished ()) {
		isSending = false;
		// удаляем структуру
		delete file_to_send;
		file_to_send = nullptr;
	}
	// если все еще не послали файл
	else {
		// посылаем следующюю часть файла
		_clientSocket.sendFile (file_to_send);
	}
}

void parse_bytes_pos (int &first_byte_pos, int &last_byte_pos, string range) {
	// range := "bytes=(\d*)-(\d*)", но не оба нулевые
	// 					 012345
	range.erase (0, 6);
	// 					      "(\d*)-(\d*)"
	if (range[0] == '-') {
		// 						       "-(\d+)"
		range.erase (0, 1);
		last_byte_pos = std::stoi (range);
		first_byte_pos = -1;
	}
	else {
		// 				 			"(\d+)-(\d*)"
		first_byte_pos = std::stoi (range.substr (0, range.find ("-")));
		range.erase (0, range.find ("-")+1);
		// 						        "(\d*)"
		if (range.empty ())
			last_byte_pos = -1;
		else
			last_byte_pos = std::stoi (range);
	}
}

void Client::sendFile (string path, FileStat &file_attrib, Hash &request) {
	isSending = true;
	time_t modif_date = file_attrib.getModifDate ();
	// если в запросе не содержится интервал
	if (request["Range"].empty()) {
		// посылаем обычный 200 OK
		_clientSocket.sendString (
			Response::response_200 (
				"GET", 
				modif_date, 
				file_attrib.getMimeType (), 
				file_attrib.getSize ()
			)
		);
		// подготавливаем структуру файла
		file_to_send = new FileToSend (path, 0, file_attrib.getSize () - 1);
		// отсылаем файл (возможно, не полностью)
		_clientSocket.sendFile (file_to_send);
	}
	else {
		// присутствует Range
		// выделяем начальный и конечный байты
		size_t first_byte_pos, last_byte_pos;
		int first_n, last_n;
		size_t file_size = file_attrib.getSize ();
		parse_bytes_pos(first_n, last_n, request["Range"]);
		Printer::debug (
			string("<")+to_string(first_n)+">-<"+to_string(last_n)+">",
		 "Распарсили позиции"
		);
		// если начальная позиция не указана
		if (first_n == -1) {
			// выборка последних last_n байт
			// если отступ от конца больше самого файла
			if (last_n > file_size)
				// выравниваем его
				last_n = file_size;
			// первая позиция это размер файла минус величина отступа
			first_byte_pos = file_size - last_n;
			// последняя позиция это конец файла
			last_byte_pos = file_size - 1;
		}
		// начальная позиция указана
		else {
			// если конечная позиция не указана
			if (last_n == -1)
				// она равна концу файла
				last_n = file_size - 1;
			// конечная позиция указана или равна концу файла
			// выборка байт с first_n по last_n
			first_byte_pos = first_n;
			last_byte_pos = last_n;
		}
		// если конечная позиция больше длины файла
		if (last_byte_pos > file_size)
			// выравниваем ее
			last_byte_pos = file_size - 1;
		// если начальная позиция больше конечной
		if (first_byte_pos > last_byte_pos) {
			Printer::error ("Последняя позиция меньше первой", "[Client::sendFile]");
			// посылаем 416 Range Not Satisfiable
			_clientSocket.sendString (Response::response_4xx_5xx (416, request["Method"], file_size));
			throw ClientException ("Последняя позиция меньше первой");
		}
		// подготавливаем структуру файла
		file_to_send = new FileToSend (path, first_byte_pos, last_byte_pos);
		_clientSocket.sendString (
			Response::response_206 (
				modif_date, 
				file_attrib.getMimeType (), 
				file_to_send -> portion_size,
				file_size,
				first_byte_pos,
				last_byte_pos
			)
		);
	}
}

bool Client::respond () {
	// принять новые данные от клиента
	appendRequest ();
	// если прием закончился
	if(isReceiveEnded ()) {
		// парсим заголовки запроса
		Hash request = parse_request (_http_request);
		Printer::debug ("",string("Запрос клиента [")+to_string(_clientNo)+"]", request);
		if (request["Method"] != "GET" && request["Method"] != "HEAD") {
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
						// телом будет таблица с файлами внутри директории
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
						// дополнительный парсинг на проверку Content-Range
						sendFile (path, file_attrib, request);
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
			// подготавливаем структуру
			FileToSend *output_file = new FileToSend (proc -> getOutputFile (), 0, file_stat.getSize () - 1);
			_clientSocket.sendFile (output_file);
			// удаляем структуру
			delete output_file;
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

