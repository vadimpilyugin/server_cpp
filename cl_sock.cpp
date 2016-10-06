#include <string>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <arpa/inet.h>
#include <poll.h>
#include <assert.h>

using namespace std;

#include "client.h"
#include "excp.h"

void ClientSocket::Send(const void *buffer, size_t size)
{
	if(!isClosed())
	{
		if(write(_sd, buffer, size) < 0)
			throw SocketException("Запись в сокет неуспешна");
	}
	else 
		throw ClientException("Клиент закрыл соединение");
}

int ClientSocket::Receive(void *buffer, size_t size)
{
	int n = read(_sd, buffer, size);
	if(n < 0)
		throw SocketException("Чтение из сокета неуспешно");
	else if(n == 0)
		throw ClientException("Клиент закрыл соединение");
	else
		return n;
}

bool ClientSocket::isClosed()
{
	struct pollfd poll_struct;
	poll_struct.fd = _sd;
	poll_struct.events = POLLRDHUP | POLLOUT;
	switch(poll(&poll_struct, 1, 50))
	{
		case -1: throw ServerException("Poll не сработал");
				 break;
		case 0:	 return true;
		default:
		{
			if(poll_struct.revents & POLLRDHUP)
				return true;
			else if(poll_struct.revents & POLLOUT)
				return false;
			else
				assert(false);
		}
	}
}

void ClientSocket::SendFile(const string &path)
{
	FILE *content = fopen(path.c_str(), "r");
	int n;
	const int BUF_LEN = 1024;
	char *buf = new char[BUF_LEN];
	while(!feof(content))
	{
		n = fread(buf, sizeof(char), BUF_LEN, content);
		Send(buf, n);
	}
	delete [] buf;
	fclose(content);
}

SocketAddress::SocketAddress(const string &hostname, short port){	_addr.sin_family = AF_INET;	_addr.sin_port = htons(port);	inet_aton(hostname.c_str(),&_addr.sin_addr);}
