#include <string>
#include <sys/time.h>
#include <sstream>
#include <iostream>
using namespace std;

#include "client.h"
#include "filestat.h"


#include <assert.h>


int Client::_clients_num = 0;

string Client::getGmtDate(time_t time_in_sec)
{
	const int DATE_MAXSIZE = 80;
	const char *format = "%a, %d %b %G %H:%M:%S GMT";
	char *output = new char [DATE_MAXSIZE];
	time_t meantime;
	if(time_in_sec == -1)
		meantime = time(0);
	else
		meantime = time_in_sec;
	struct tm *t = gmtime(&meantime);
	strftime(output, DATE_MAXSIZE, format, t);
	string tmp(output);
	delete [] output;
	return tmp;
}

void Client::writeDirContent(const string &dir_path, const string &file_name)
{
	string shell_command = ">" + file_name + ";"
		+ "ls -A -c --group-directories-first --indicator-style=slash "
		+ dir_path + ">>" + file_name;
	system(shell_command.c_str());
}

void Client::ParseRequest(string &method, string &path, vector<string> &env) const
{
	string header = _http_request.substr(0, _http_request.find('\r')); //первая строка запроса
	string params;
	if(header.find("cgi-bin/") != string::npos)
	{
		if(header.find('?') != string::npos)
		{
			header[header.find('?')] = ' ';
			stringstream tmp(move(header));
			tmp >> method >> path >> params;
		}
		else
		{
			stringstream tmp(move(header));
			tmp >> method >> path;
		}
		string user_agent, port;
		{
			stringstream tmp;
			tmp << _address.getPort();
			tmp >> port;
		}
		size_t pos;
		if((pos = _http_request.find("User-Agent:")) != string::npos)
		{
			size_t space = _http_request.find(' ', pos);
			user_agent = _http_request.substr(space + 1, _http_request.find('\r', pos) - space - 1);
		}
		vector<string> cgi_env
		{
			"CONTENT-TYPE=text/plain",
			"GATEWAY-INTERFACE=CGI/1.1",
			"REMOTE_ADDR=" + _address.getIp(),
			"REMOTE_PORT=" + port,
			"QUERY_STRING=" + params,
			"SERVER_ADDR=127.0.0.1",
			"SERVER_NAME=www.void.info",
			"SERVER_PORT=16000",
			"SERVER_PROTOCOL=HTTP/1.1",
			"SERVER_SOFTWARE=HTTP Model Server v0.4",
			"SCRIPT_NAME=" + path,
			"SCRIPT_FILENAME=" + FileStat::getFullPath(path),
			"DOCUMENT_ROOT=" + FileStat::getCwd(),
			"HTTP_USER_AGENT=" + user_agent,
			"HTTP_REFERER=index.html"
		};
		env = move(cgi_env);
	}
	else
	{
		stringstream tmp(move(header));
		tmp >> method >> path;
	}
	assert(header[0] != '\n');
	cout << "Запрос клиента["<<_clientNo<<"]: " << header << endl;
	cout << "Метод: " << method << endl;
	cout << "Путь: " << path << endl;
	cout << "Параметры запроса: ";
	if(params != "")
		cout << params << endl;
	else
		cout << "-" << endl;
	cout << endl;
}


Client::Client(int sd, SocketAddress &address): _clientSocket(sd), _address(address)
{
	_buffer = new char [_BUF_MAX];
	_clientNo = _clients_num++;
	cout << endl << "Новый клиент!(получает номер " << _clientNo << ")" << endl;
	cout << "IP: " << _address.getIp() << endl;
	cout << "Port: " << _address.getPort() << endl;
	cout << endl;
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
		cout << "Client["<<_clientNo<<"]: успешно отключен!"<<endl;
		_clients_num --;
	}
}

void Client::AppendRequest()
{
	int n = _clientSocket.Receive(_buffer,_BUF_MAX - 1);
	_buffer[n] = 0;
	_http_request.append(_buffer);
}

string Client::SendHeader(int errcode, const string method, int content_len,
						const string content_type, time_t modif_date)
{
	stringstream http_response;
	string code, info;
	switch(errcode)
	{
		case 200:
		{
			code = "200 OK";
			info = "-";
			break;
		}
		case 400:
		{
			code = "400 Bad Request";
			info = "This is a Bad Request\n";
			break;
		}
		case 403:
		{
			code = "403 Forbidden";
			info = "You have no permission to access this directory or file\n";
			break;
		}
		case 404:
		{
			code = "404 File Not Found";
			info = "The file you are looking for is not found\n";
			break;
		}
		case 500:
		{
			code = "500 Internal Server Error";
			info = "Server error. Service unavailable\n";
			break;
		}
		case 501:
		{
			code = "501 Not Implemented";
			info = "The method "+method+" is not supported.\n";
			info += "The supported methods are GET and HEAD.\n";
			break;
		}
		case 503:
		{
			code = "503 Service Unavailable";
			info = "Too many clients. Service is Unavailable\n";
			break;
		}
	}
	http_response << "HTTP/1.1 " << code << " \r\n"
				<< "Date: " << getGmtDate() << " \r\n"
				<< "Server: HTTP Model Server 1.0 \r\n";
	if(errcode == 501)
		http_response << "Allow: GET, HEAD \r\n";
	if(method == "GET")
	{
		http_response << "Content-type: " << content_type << " \r\n"
					<< "Content-length: ";
		if(content_len != -1)
			http_response << content_len << "\r\n";
		else
			http_response << info.size()+1 << "\r\n";
	}
	else if(method == "HEAD")
		http_response << "Last-modified: " << getGmtDate(modif_date) << "\r\n";
	http_response << "\r\n";
	_clientSocket.Send(http_response.str().c_str(), http_response.str().size());
	if(errcode != 200 && method == "GET")
		_clientSocket.Send(info.c_str(), info.size()+1);
	cout << "Наш ответ клиенту["<<_clientNo<<"] на его запрос: " << code << endl
		<< "Пояснение: "<< info << endl << endl;
	return code;
}

bool Client::Respond()
{
	const string homepage = "../server_v2";
	AppendRequest();
	if(ifReceiveEnded())
	{
		string method, path;
		vector<string> cgi_params;
		ParseRequest(method, path, cgi_params);
		if(!(method == "GET" || method == "HEAD"))
			throw ClientException(SendHeader(501, method));
		path.erase(path.begin()); //удалить первый слэш
		if(path == "")
			path = homepage;
		try
		{
			FileStat file_attrib(path);
			if(!file_attrib.isReadable())
				throw FileException(path, EACCES);
			if(file_attrib.isDirectory())
			{
				writeDirContent(path, "tmpfile.txt");
				SendHeader(200, method, FileStat("tmpfile.txt").getSize(), "text/plain", FileStat(path).getModifDate());
				if(method == "GET")
					_clientSocket.SendFile("tmpfile.txt");
			}
			else if((path.find("cgi-bin/") != string::npos) && method == "GET")
			{
				vector <string> cgi_argv = {FileStat::getFullPath(path)};
				_client_progs.push_back(CgiProg(cgi_argv[0], cgi_params, cgi_argv));
			}
			else
			{
				SendHeader(200, method, file_attrib.getSize(), FileStat::getFileType(path),
							file_attrib.getModifDate());
				if(method == "GET")
					_clientSocket.SendFile(path);
			}
			_http_request.erase(0, _http_request.find("\r\n\r\n") + 4);
			if(_http_request.size() > 0)
				return true;
			else
				return false;
		}
		catch(FileException &a)
		{
			switch(a.getErrno())
			{
				case EACCES: throw(ClientException(SendHeader(403)));
				case ENOENT: throw(ClientException(SendHeader(404)));
				default: throw(ClientException(SendHeader(400)));
			}
		}
	}
}

void Client::checkCgiProgs()
{
	auto proc = _client_progs.begin();
	while(proc != _client_progs.end())
	{
		if(proc -> isProcessEnded() && proc -> isExitedCorrect())
		{
			SendHeader(200, "GET", FileStat(proc -> getOutputFile()).getSize(), "text/plain");
			_clientSocket.SendFile(proc -> getOutputFile());
			_client_progs.erase(proc++);
		}
		else if(proc -> isProcessEnded() && !proc -> isExitedCorrect())
		{
			SendHeader(500); //Internal Server Error
			_client_progs.erase(proc++);
		}
		++proc;
	}
}
