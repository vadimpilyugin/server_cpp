#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h> /* superset of previous */ 
#include <arpa/inet.h>
#include <string>
using namespace std;

class Socket
{
	friend class SelectionSet;
protected:
	int _sd;
public:
	Socket(int sd = -1);
	~Socket();
	Socket(Socket &&sock) = delete;
	Socket(Socket &sock) = delete;
};

class SocketAddress
{
	struct sockaddr_in _addr;
public:
	SocketAddress (const string hostname = "", short port = 0);
	struct sockaddr *getAddress () const {return (struct sockaddr *) &_addr;}
	int getSize () const {return sizeof(_addr);}
	int getPort () const {return ntohs(_addr.sin_port);}
	string getIp () const {return inet_ntoa(_addr.sin_addr);}
	string to_s () { return getIp () + string(":") + to_string(getPort ()); }
};