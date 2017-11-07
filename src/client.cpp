#include <sstream>

#include "client.h"
#include "helpers.h"
#include "config.h"
#include "auth.h"
#include "range.h"
#include "request.h"
#include "api.h"


int Client::_clients_num = 0;
const string Client::END_OF_REQUEST = "\r\n\r\n";

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
	if (isRequestReceived ())
		isResponseNeeded = true;
}

void Client::continue_file_sending () {
	if (file_to_send -> is_finished ()) {
		isSending = false;

		// останавливаем таймер
		_clientSocket.t.stop ();

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

void Client::init_file_sending (Hash &request) {
	Printer::assert (!isSending, "[init_file_sending]", "еще не послали предыдущий файл!");

	// устанавливаем флаг посылки
	isSending = true;

	// запускаем таймер в клиентском сокете
	_clientSocket.t.start ();
	_clientSocket.bytes_sent = 0;

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

string Client::cut_request () { 
	size_t eor_pos = _http_request.find (END_OF_REQUEST) + END_OF_REQUEST.size ();
	if (eor_pos == string::npos) {
		Printer::error ("cut_request был вызван при неполном принятом запросе");
	}
	string request = _http_request.substr (0, eor_pos);
	_http_request.erase(0, eor_pos);
	if (isRequestReceived ())
		isResponseNeeded = true;
	else
		isResponseNeeded = false;
	return request;
}

void Client::respond () {
	// принимаем новые данные от клиента
	// appendRequest ();
	// если прием закончился
	if(isRequestReceived ()) {
		// вырезаем первый запрос из буфера
		string request = cut_request ();
		// парсим заголовки запроса
		Hash parsed_request = Request::parse_request (request);
		Printer::debug ("",string("Запрос клиента [")+to_string(_clientNo)+"]", parsed_request);
		// проверяем запрос на корректность
		int code = Request::process_request (parsed_request);
		// производим действия в соответствии с кодом
		make_actions (parsed_request, code);
	}
}

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

