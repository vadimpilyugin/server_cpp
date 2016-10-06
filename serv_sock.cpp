#include <string>
#include <sys/types.h>
#include <sys/socket.h>
using namespace std;

#include "serv_sock.h"
#include "excp.h"

void ServerSocket::Bind(const string &hostname, const int port)
{
	int opt = 1;
	setsockopt(_sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	SocketAddress addr(hostname, port);
	if(bind(_sd, addr.getAddress(), addr.getSize()) != 0)
		throw AddressException(addr.getPort(), "Не удалось назначить адрес сокету");
}

void ServerSocket::Listen()
{
	if(listen(_sd, MAX_CLNTS) == -1)
		throw SocketException("Сокет не переведен в слушающий режим");
}

Client ServerSocket::Accept()
{
	SocketAddress adr;
	socklen_t n = adr.getSize();
	int cl_sock = accept(_sd, adr.getAddress(), &n);
	if(cl_sock < 0)
		throw ClientException("Не удалось принять запрос на соединение");
	return Client(cl_sock, adr);
}

void SelectionSet::Select(const ServerSocket &ls, const list<Client> &clients_list, int proc_num)
{
	int max_d = ls._sd;
	FD_ZERO(&readfds);
	FD_SET(ls._sd, &readfds);
	for(auto &client:clients_list)
	{
		FD_SET(client.getSock()._sd, &readfds);
		if(client.getSock()._sd > max_d)
			max_d = client.getSock()._sd;
	}
	int res;
	struct timeval timeval;
	timeval.tv_sec = 0;
	timeval.tv_usec = 50;
	if(proc_num == 0)
		res = select(max_d + 1, &readfds, 0, 0, 0);
	else
		res = select(max_d + 1, &readfds, 0, 0, &timeval);
	if(res == -1)
		throw ServerException("Не удалось выполнить select");
}
