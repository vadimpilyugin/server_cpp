#pragma once
#include <string>
#include <vector>
#include <list>
using namespace std;

#include "cl_sock.h"
#include "cgiprog.h"
#include "file_to_send.h"
#include "printer.hpp"
#include "filestat.h"

class Client
{
	string _http_request;
	const int BUF_LEN = 1024;
	char *_buf;
	int _clientNo;
	static int _clients_num;
	ClientSocket _clientSocket;
	SocketAddress _address;
	list <CgiProg> _client_progs;
	bool isSending;
	FileToSend *file_to_send;
	bool isResponseNeeded;
public:
	
	Client(int sd, SocketAddress &address);
	Client(Client &&cl) = delete;
	~Client();
	void appendRequest();
	bool isReceiveEnded() const {return (_http_request.rfind("\r\n\r\n") != string::npos);}
	// string sendHeader(int errcode, const string method = "GET", int content_len = -1,
	// 				const string content_type = "text/html", time_t modif_date = 0);
	void respond_or_send ();
	bool respond();
	int getNo() const {return _clientNo;};
	const ClientSocket &getSock() const {return _clientSocket;}
	void checkCgiProgs();
	// void sendCgiResponse();
	ClientSocket &getSock () { return _clientSocket; }
	bool isSendingFile () { return isSending; }
	bool isResponseToClientNeeded () { return isResponseNeeded; }
private:
	Hash parse_request (string request);
	vector<string> fill_cgi_env (Hash &request) const;
	string writeDirContent(string dir_path) const;
	void sendFile (string path, FileStat &file_attrib, Hash &request);
};
