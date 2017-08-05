#include "cl_sock.h"
#include "excp.h"
#include "printer.hpp"

#include <poll.h>
#include <errno.h>
#include <unistd.h>
#include <cstdio>

void ClientSocket::send(const void *buffer, size_t size)
{
	if(!isClosed())
	{
		if(write(_sd, buffer, size) < 0)
			throw SocketException("Запись в сокет неуспешна");
	}
	else 
		throw ClientException("Клиент закрыл соединение");
}

int ClientSocket::receive(void *buffer, size_t size)
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
		case 0:	 return false;
		default:
		{
			if(poll_struct.revents & POLLRDHUP)
				return true;
			else if(poll_struct.revents & POLLOUT)
				return false;
			else
				throw ServerException ("Unknown poll event");
		}
	}
}

void ClientSocket::sendFile(const string &path)
{
	Printer::debug (path, "Посылаем файл");
	FILE *content = fopen(path.c_str(), "r");
	if (content == NULL) {
		Printer::error ("Cannot open file");
		throw ClientException (strerror (errno));
	}
	else
		Printer::debug ("Файл открыт");
	int n;
	const int BUF_LEN = 1024*1024;
	char *buf = new char[BUF_LEN];
	while(!feof(content))
	{
		n = fread(buf, sizeof(char), BUF_LEN, content);
		if (n < BUF_LEN && !feof(content)) {
			Printer::error ("Reading error");
			throw ClientException (strerror (errno));
		}
		else
			Printer::debug ("Посылаем " +to_string(n)+" байт");
		send(buf, n);
	}
	delete [] buf;
	fclose(content);
	Printer::debug ("Посылка закончена");
}
void ClientSocket::sendString(const string &body)
{
	Printer::debug (body, "Наш ответ клиенту на его запрос");
	send (body.c_str(), body.size());
}
