#include <sstream>

#include "client.h"
#include "helpers.h"
#include "config.h"
#include "auth.h"
#include "range.h"
#include "request.h"
#include "api.h"


int Client::_clients_num = 0;

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

Client::~Client()
{
	if(_buf != nullptr)
	{
		delete [] _buf;
		if (file_to_send != nullptr)
			delete file_to_send;
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

void Client::respond_or_send () {
	// если не посылаем файл
	if (!isSending) {
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


// void Client::sendFile (string path, FileStat &file_attrib, Hash &request) {
// 	isSending = true;
// 	time_t modif_date = file_attrib.getModifDate ();
// 	// если в запросе не содержится интервал
// 	if (request["Range"].empty()) {
// 		// посылаем обычный 200 OK
// 		_clientSocket.sendString (
// 			Response::response_200 (
// 				"GET", 
// 				modif_date, 
// 				file_attrib.getMimeType (), 
// 				file_attrib.getSize ()
// 			)
// 		);
// 		// подготавливаем структуру файла
// 		file_to_send = new FileToSend (path, 0, file_attrib.getSize () - 1);
// 		// отсылаем файл (возможно, не полностью)
// 		_clientSocket.sendFile (file_to_send);
// 	}
// 	else {
// 		// присутствует Range
// 		// выделяем начальный и конечный байты
// 		size_t first_byte_pos, last_byte_pos;
// 		long int first_n, last_n;
// 		size_t file_size = file_attrib.getSize ();
// 		parse_bytes_pos(first_n, last_n, request["Range"]);
// 		Printer::debug (
// 			string("<")+to_string(first_n)+">-<"+to_string(last_n)+">",
// 		 "Распарсили позиции"
// 		);
// 		// если начальная позиция не указана
// 		if (first_n == -1) {
// 			// выборка последних last_n байт
// 			// если отступ от конца больше самого файла
// 			if (last_n > file_size)
// 				// выравниваем его
// 				last_n = file_size;
// 			// первая позиция это размер файла минус величина отступа
// 			first_byte_pos = file_size - last_n;
// 			// последняя позиция это конец файла
// 			last_byte_pos = file_size - 1;
// 		}
// 		// начальная позиция указана
// 		else {
// 			// если конечная позиция не указана
// 			if (last_n == -1)
// 				// она равна концу файла
// 				last_n = file_size - 1;
// 			// конечная позиция указана или равна концу файла
// 			// выборка байт с first_n по last_n
// 			first_byte_pos = first_n;
// 			last_byte_pos = last_n;
// 		}
// 		// если конечная позиция больше длины файла
// 		if (last_byte_pos > file_size)
// 			// выравниваем ее
// 			last_byte_pos = file_size - 1;
// 		// если начальная позиция больше конечной
// 		if (first_byte_pos > last_byte_pos) {
// 			Printer::error ("Последняя позиция меньше первой", "[Client::sendFile]");
// 			// посылаем 416 Range Not Satisfiable
// 			_clientSocket.sendString (Response::response_4xx_5xx (416, request["Method"], file_size));
// 			throw ClientException ("Последняя позиция меньше первой");
// 		}
// 		// подготавливаем структуру файла
// 		file_to_send = new FileToSend (path, first_byte_pos, last_byte_pos);
// 		_clientSocket.sendString (
// 			Response::response_206 (
// 				modif_date, 
// 				file_attrib.getMimeType (), 
// 				file_to_send -> portion_size,
// 				file_size,
// 				first_byte_pos,
// 				last_byte_pos
// 			)
// 		);
// 	}
// }

void Client::init_file_sending (Hash &request) {
	Printer::assert (!isSending, "[init_file_sending]", "еще не послали предыдущий файл!");

	// устанавливаем флаг посылки
	isSending = true;
	// получаем информацию о файле
	FileStat file_attrib (request[Request::PATH_HEADER]);

	Printer::debug ("Запрос от клиента на файл "+request[Request::PATH_HEADER]);

	// если не присутствует заголовок Range 
	if (!Range::has_range (request)) {

		Printer::debug ("Запрос без Range");

		// посылаем обычный 200 OK
		_clientSocket.sendString (
			Response::response_200 (
				Methods::GET_METHOD, 
				file_attrib.getModifDate (), 
				file_attrib.getMimeType (), 
				file_attrib.getSize ()
			)
		);

		// подготавливаем структуру файла
		file_to_send = new FileToSend (
			request[Request::PATH_HEADER], 
			0, 
			file_attrib.getSize () - 1
		);
	}
	else {
		// посылка с учетом Range

		// получаем first и last byte pos
		size_t first_byte_pos, last_byte_pos;
		Range::get_file_pos (first_byte_pos, last_byte_pos, 
			file_attrib.getSize (), request[Request::RANGE_HEADER]);

		Printer::debug ("Запрос с учетом Range <"+
			to_string(first_byte_pos)+">-<"+to_string(last_byte_pos)+">"
		);
		// подготавливаем структуру файла
		file_to_send = new FileToSend (request[Request::PATH_HEADER], first_byte_pos, last_byte_pos);
		_clientSocket.sendString (
			Response::response_206 (
				file_attrib.getModifDate (), 
				file_attrib.getMimeType (), 
				file_to_send -> portion_size,
				file_attrib.getSize (),
				first_byte_pos,
				last_byte_pos
			)
		);
	}
	// посылаем первую часть файла
	_clientSocket.sendFile (file_to_send);
}

void Client::make_actions (Hash &request, int code) {
	const string METHOD_HEADER = "Method";

	switch (code) {
		case 	ResponseCode::NOT_IMPLEMENTED:
		case	ResponseCode::NOT_AUTHORIZED:
		case	ResponseCode::NOT_FOUND:
		case	ResponseCode::FORBIDDEN:
		case	ResponseCode::RANGE_NOT_SATISFIABLE:

			_clientSocket.sendString (
				Response::response_4xx_5xx (code, request[METHOD_HEADER])
			);
			throw ClientException (Response::response_body (code));
			break;
		case ResponseCode::OK:
			// если метод HEAD
			if (request[METHOD_HEADER] == Methods::HEAD_METHOD) {
				Printer::debug ("Метод Head, посылаем информацию о пути "+request[Request::PATH_HEADER]);
				string resp_head = Response::response_200 (
					Methods::HEAD_METHOD, 
					FileStat::getModifDate (request[Request::PATH_HEADER])
				);
				_clientSocket.sendString (resp_head);
			}
			else if (request[Request::PATH_HEADER].find(Config::section ("internal")["script_folder"]) != 
				string::npos && !FileStat::isDirectory (request[Request::PATH_HEADER])) {
				Printer::debug("Запускаем скрипт по пути "+request[Request::PATH_HEADER]);
				// Launch script
				vector <string> cgi_argv = {request[Request::PATH_HEADER]};
				vector <string> cgi_env = Request::fill_cgi_env (request, _address);
				_client_progs.push_back(CgiProg(cgi_argv[0], cgi_env, cgi_argv));
			}
			else {
				// метод GET, не CGI
				// отсылаем данные
				do_sending (request);
			}
			break;
	}
}

void Client::do_sending (Hash &request) {

	// если нужно послать директорию
	if (FileStat::isDirectory (request[Request::PATH_HEADER])) {
		// тело и заголовок ответа
		string resp_body;
		string resp_head;

		// если это запрос с фронтэнда
		if (Api::is_api_request (request)) {

			// посылаем JSON с содержимым директории
			resp_body = Api::jsonDirList (request[Request::PATH_HEADER]);
			resp_head = Response::response_200 (
				Methods::GET_METHOD, 
				FileStat::getModifDate (request[Request::PATH_HEADER]), 
				FileStat::JSON_TYPE, 
				resp_body.size(), 
				true,
				true
			);
			Printer::debug ("Запрос от js кода, отправляем JSON");
			// Printer::debug (resp_body, "JSON");
		}
		else {
			// это запрос от клиента
			// телом будет таблица с файлами внутри директории
			resp_body = HtmlHelpers::htmlDirList (request[Request::PATH_HEADER]);
			resp_head = Response::response_200 (
				Methods::GET_METHOD, 
				FileStat::getModifDate (request[Request::PATH_HEADER]), 
				FileStat::HTML_TYPE, 
				resp_body.size(), 
				true
			);
			Printer::debug ("Запрос от клиента на листинг директории "+request[Request::PATH_HEADER]);
		}
		_clientSocket.sendString (resp_head);
		_clientSocket.sendString (resp_body);
	}
	else {
		// нужно послать файл
		init_file_sending (request);
	}
}

bool Client::respond () {
	// принять новые данные от клиента
	appendRequest ();
	// если прием закончился
	if(isReceiveEnded ()) {
		// парсим заголовки запроса
		Hash request = Request::parse_request (_http_request);
		Printer::debug ("",string("Запрос клиента [")+to_string(_clientNo)+"]", request);
		// проверяем запрос на корректность
		int code = Request::process_request (request);
		// производим действия в соответствии с кодом
		make_actions (request, code);
		// вырезаем обработанный запрос из строки
		_http_request.erase(0, _http_request.find("\r\n\r\n") + 4);
		if(_http_request.size() > 0)
			return true;
		else
			return false;
	}
}

// bool Client::respond () {
// 	// принять новые данные от клиента
// 	appendRequest ();
// 	// если прием закончился
// 	if(isReceiveEnded ()) {
// 		// парсим заголовки запроса
// 		Hash request = parse_request (_http_request);
// 		Printer::debug ("",string("Запрос клиента [")+to_string(_clientNo)+"]", request);
// 		if (request["Method"] != "GET" && request["Method"] != "HEAD") {
// 			// не поддерживается
// 			_clientSocket.sendString (Response::response_4xx_5xx (501, request["Method"]));
// 			throw ClientException (Response::response_body (501));
// 		}
// 		else if (Auth::authorized (request)) {
// 			// разветвление: либо это запрос от vue.js, либо обычный запрос браузера
// 			// определяется путем проверки заголовка X-Custom
// 			if (Api::is_api_request (request))

// 			// получаем путь
// 			string path = request["Path"];
// 			// заменяем / на default page
// 			if (path == "/") {
// 				path = Config::section ("network")["default_page"];
// 			}
// 			Directory::remove_first_slash (path);
// 			Printer::note (path, "Path changed to");
// 			try {
// 				FileStat file_attrib (path);
// 				if(!file_attrib.isReadable ())
// 					throw FileException(path, EACCES);
// 				time_t modif_date = file_attrib.getModifDate ();
// 				string resp_head, resp_body;
// 				if (request["Method"] == "GET") {
// 					if (file_attrib.isDirectory ()) {
// 						// телом будет таблица с файлами внутри директории
// 						resp_body = HtmlHelpers::htmlDirList (path);
// 						resp_head = Response::response_200 ("GET", modif_date, "text/html", resp_body.size(), true);
// 						_clientSocket.sendString (resp_head);
// 						_clientSocket.sendString (resp_body);
// 					}
// 					else if(path.find(Config::section ("internal")["script_folder"]) != string::npos) {
// 						// Launch script
// 						vector <string> cgi_argv = {path};
// 						vector <string> cgi_env = fill_cgi_env (request);
// 						_client_progs.push_back(CgiProg(cgi_argv[0], cgi_env, cgi_argv));
// 					}
// 					else {
// 						// дополнительный парсинг на проверку Content-Range
// 						sendFile (path, file_attrib, request);
// 					}
// 				}
// 				else if (request["Method"] == "HEAD") {
// 					resp_head = Response::response_200 ("HEAD", modif_date);
// 					_clientSocket.sendString (resp_head);
// 				}
// 				_http_request.erase(0, _http_request.find("\r\n\r\n") + 4);
// 				if(_http_request.size() > 0)
// 					return true;
// 				else
// 					return false;
// 			}
// 			catch(FileException &a) {
// 				switch (a.getErrno ())
// 				{
// 					case EACCES: 
// 						_clientSocket.sendString (Response::response_4xx_5xx (403, request["Method"]));
// 						throw ClientException (Response::response_body (403));
// 					case ENOENT: 
// 						_clientSocket.sendString (Response::response_4xx_5xx (404, request["Method"]));
// 						throw ClientException (Response::response_body (404));
// 					default: 
// 						_clientSocket.sendString (Response::response_4xx_5xx (400, request["Method"]));
// 						throw ClientException (Response::response_body (400));
// 				}
// 			}
// 		}
// 		else {
// 			// not authorized
// 			_clientSocket.sendString (Response::response_4xx_5xx (401, request["Method"]));
// 			_http_request.erase(0, _http_request.find("\r\n\r\n") + 4);
// 			if(_http_request.size() > 0)
// 				return true;
// 			else
// 				return false;
// 		}
// 	}
// 	else
// 		return false;
// }

void Client::checkCgiProgs()
{
	auto proc = _client_progs.begin ();
	while (proc != _client_progs.end ()) {
		if(proc -> isProcessEnded () && proc -> isExitedCorrectly ()) {

			// send header
			FileStat file_stat (proc -> getOutputFile ());
			_clientSocket.sendString (
				Response::response_200 (
					Methods::GET_METHOD, 
					file_stat.getModifDate (), 
					FileStat::HTML_TYPE, 
					file_stat.getSize ()
				)
			);

			// подготавливаем структуру
			FileToSend *output_file = new FileToSend (
				proc -> getOutputFile (), 
				0, 
				file_stat.getSize () - 1
			);
			_clientSocket.sendFile (output_file);

			// удаляем структуру
			delete output_file;
			auto tmp = proc;
			proc++;
			_client_progs.erase (tmp);
		}
		else if(proc -> isProcessEnded () && !proc -> isExitedCorrectly ()) {

			// send header 500
			_clientSocket.sendString (
				Response::response_4xx_5xx (
					ResponseCode::INTERNAL_ERROR, 
					Methods::GET_METHOD
				)
			);
			auto tmp = proc;
			proc++;
			_client_progs.erase (tmp);
		}
		else
			proc++;
	}
}

