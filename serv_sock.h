#ifndef SERV_SOCK_H_INCLUDED
#define SERV_SOCK_H_INCLUDED

#include <list>
using namespace std;

#include "sock.h"
#include "client.h"

class ServerSocket: public Socket
{
public:
	static const int MAX_CLNTS = 3;
	ServerSocket(const string &hostname, const int port)
	{
		Bind(hostname, port);
		Listen();
	}
	Client Accept();
private:
	void Bind(const string &hostname, const int port);
	void Listen();
};

class SelectionSet
{
	fd_set readfds;
public:
	void Select(const ServerSocket &ls, const list<Client> &clients_list, int proc_num);
	bool isReady(const Socket &socket) {return FD_ISSET(socket._sd, &readfds);}
};

#endif // SERV_SOCK_H_INCLUDED
