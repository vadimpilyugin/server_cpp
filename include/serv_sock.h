#pragma once

#include <list>
using namespace std;

#include "sock.h"
#include "client.h"

class ServerSocket: public Socket
{
public:
	ServerSocket(const string &hostname, const int port)
	{
		bind(hostname, port);
		listen();
	}
	Client accept();
private:
	void bind(const string &hostname, const int port);
	void listen();
};

class SelectionSet
{
	fd_set readfds;
public:
	void select(const ServerSocket &ls, const list<Client> &clients_list, int proc_num);
	bool isReady(const Socket &socket) {return FD_ISSET(socket._sd, &readfds);}
};