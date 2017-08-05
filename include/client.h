#pragma once
#include <string>
#include <vector>
#include <list>
using namespace std;

#include "cl_sock.h"
#include "cgiprog.h"
#include "printer.hpp"

class Client
{
	string _http_request;
	const int _BUF_MAX = 1024;
	char *_buffer;
	int _clientNo;
	static int _clients_num;
	ClientSocket _clientSocket;
	SocketAddress _address;
	list <CgiProg> _client_progs;
public:
	
	Client(int sd, SocketAddress &address);
	Client(Client &&cl);
	~Client();
	void appendRequest();
	bool isReceiveEnded() const {return (_http_request.rfind("\r\n\r\n") != string::npos);}
	// string sendHeader(int errcode, const string method = "GET", int content_len = -1,
	// 				const string content_type = "text/html", time_t modif_date = 0);
	bool respond();
	int getNo() const {return _clientNo;};
	const ClientSocket &getSock() const {return _clientSocket;}
	void checkCgiProgs();
	// void sendCgiResponse();
	ClientSocket &getSock () { return _clientSocket; }
private:
	Hash parse_request (string request);
	vector<string> fill_cgi_env (Hash &request) const;
	string writeDirContent(string dir_path) const;
};
