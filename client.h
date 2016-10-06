#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include <vector>
#include <list>
#include <arpa/inet.h>
#include <sstream>
using namespace std;

#include "sock.h"
#include "cgiprog.h"

class ClientSocket: public Socket
{
public:
	using Socket::Socket;
	ClientSocket(ClientSocket &&cl_sock): Socket(move(cl_sock)) {}
	void Send(const void *buffer, size_t size);
	int Receive(void *buffer, size_t size);
	void SendFile(const string &file_name);
private:
	bool isClosed();
};

class SocketAddress
{
	struct sockaddr_in _addr;
public:
	SocketAddress(const string &hostname = "127.0.0.1", short port = 17000);
	struct sockaddr *getAddress() const {return (struct sockaddr *) &_addr;}
	int getSize() const {return sizeof(_addr);}
	int getPort() const {return ntohs(_addr.sin_port);}
	string getIp() const {return inet_ntoa(_addr.sin_addr);}
};

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
	void AppendRequest();
	bool ifReceiveEnded() const {return (_http_request.rfind("\r\n\r\n") != string::npos);}
	string SendHeader(int errcode, const string method = "GET", int content_len = -1,
					const string content_type = "text/html", time_t modif_date = 0);
	bool Respond();
	int getNo() const {return _clientNo;};
	const ClientSocket &getSock() const {return _clientSocket;}
	void checkCgiProgs();
	void SendCgiResponse();
private:
	static string getGmtDate(time_t time_in_sec = -1);
	void ParseRequest(string &method, string &path, vector<string> &cgi_param) const;
	void writeDirContent(const string &dir_path, const string &file_name);
};

#endif // CLIENT_H_INCLUDED
