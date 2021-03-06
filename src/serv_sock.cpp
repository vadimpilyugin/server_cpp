#include "serv_sock.h"
#include "excp.h"
#include "config.h"

#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

using namespace std;


void ServerSocket::bind(const string &hostname, const int port)
{
	int opt = 1;
	setsockopt(_sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	SocketAddress addr(hostname, port);
	if(::bind(_sd, addr.getAddress(), addr.getSize()) != 0)
		throw AddressException(addr.getPort(), "Не удалось назначить адрес сокету");
}

void ServerSocket::listen()
{
	if(::listen(_sd, atoi (Config::section ("params")["max_clients"].c_str ())) == -1)
		throw SocketException("Сокет не переведен в слушающий режим");
}

Client *ServerSocket::accept()
{
	SocketAddress adr;
	socklen_t n = adr.getSize();
	int cl_sock = ::accept(_sd, adr.getAddress(), &n);
	if(cl_sock < 0)
		throw ClientException("Не удалось принять запрос на соединение");
	return new Client(cl_sock, adr);
}

void SelectionSet::select(const ServerSocket &ls, const list<Client *> &clients_list, int proc_num)
{
	int max_d = ls._sd;
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_SET(ls._sd, &readfds);
	bool isClientsToRespond = false;
	for(auto &client:clients_list)
	{
		if (client -> isSendingFile ())
			FD_SET(client -> getSock()._sd, &writefds);
		else
			FD_SET(client -> getSock()._sd, &readfds);
		if (client -> isResponseToClientNeeded ())
			isClientsToRespond = true;
		if(client -> getSock()._sd > max_d)
			max_d = client -> getSock()._sd;
	}
	int res;
	struct timeval timeval;
	timeval.tv_sec = 0;
	timeval.tv_usec = 50;
	// если нет CGI-процессов и нет клиентов, которым надо ответить
	if(proc_num == 0 && !isClientsToRespond)
		res = ::select(max_d + 1, &readfds, &writefds, 0, 0);
	// есть либо клиенты, либо программы
	else
		res = ::select(max_d + 1, &readfds, &writefds, 0, &timeval);
	if(res == -1) {
		Printer::error (strerror (errno), "[SelectionSet::select]");
		throw ServerException("Не удалось выполнить select");
	}
}
