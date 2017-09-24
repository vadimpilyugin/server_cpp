#include "cl_sock.h"
#include "excp.h"
#include "printer.hpp"
#include "file_to_send.h"

#include <poll.h>
#include <errno.h>
#include <unistd.h>
#include <cstdio>

int ClientSocket::send(const void *buffer, size_t size)
{
	if(!isClosed())
	{
		int n_written = write(_sd, buffer, size);
		if(n_written < 0) {
			// ошибка при записи
			Printer::error ("ошибка при записи в сокет", string("[ClientSocket::send]"));
			throw SocketException("Запись в сокет неуспешна");
		}
		// если записали меньше, чем хотели
		else if (n_written < size) {
			Printer::error ("послали меньше, чем было в буфере", string("[ClientSocket::send]"));
			return n_written;
		}
	}
	else {
		Printer::debug ("сокет закрыт для записи", "[ClientSocket::send]");
		throw ClientException("Клиент закрыл соединение");
	}
}

int ClientSocket::receive(void *buffer, size_t size)
{
	int n = read(_sd, buffer, size);
	if(n < 0) {
		Printer::error ("чтение из сокета неуспешно", "[ClientSocket::receive]");
		throw SocketException("Чтение из сокета неуспешно");
	}
	else if(n == 0) {
		Printer::debug ("прочитали 0 байт из сокета", "[ClientSocket::receive]");
		throw ClientException("Клиент закрыл соединение");
	}
	else
		return n;
}

bool ClientSocket::isClosed()
{
	struct pollfd poll_struct;
	poll_struct.fd = _sd;
	poll_struct.events = POLLHUP | POLLOUT;
	switch(poll(&poll_struct, 1, 500))
	{
		case -1: throw ServerException("Poll не сработал");
				 break;
		case 0:	 return false;
		default:
		{
			if(poll_struct.revents & POLLHUP)
				return true;
			else if(poll_struct.revents & POLLOUT)
				return false;
			else
				throw ServerException ("Unknown poll event");
		}
	}
}

void ClientSocket::sendFile(FileToSend *file_to_send)
{
	if (file_to_send == nullptr) {
		Printer::error ("переданный указатель на FileToSend нулевой", "[ClientSocket::sendFile]");
		throw ServerException ("Переданный указатель на FileToSend нулевой!");
	}
	// указатель не нулевой
	size_t n_read = file_to_send -> fread (BUF_LEN, buf);
	Printer::debug (
		string ("посылаем ")+to_string (n_read)+" байт",
		string ("sendFile ")+to_string (file_to_send -> current_pos-n_read)+
		"-"+to_string ((file_to_send -> current_pos)-1)+"/"+to_string(file_to_send -> file_size)
	);
	int n_written = send (buf, n_read);
	if (n_written < n_read) {
		file_to_send -> rewind_back (n_read - n_written);
	}
	// FILE *content = fopen(path.c_str(), "r");
	// if (content == NULL) {
	// 	Printer::error ("Cannot open file");
	// 	throw ClientException (strerror (errno));
	// }
	// else
	// 	Printer::debug ("Файл открыт");
	// int i = 0;
	// while(!feof(content))
	// {
	// 	n = fread(buf, sizeof(char), BUF_LEN, content);
	// 	if (n < BUF_LEN && !feof(content)) {
	// 		Printer::error ("Reading error");
	// 		throw ClientException (strerror (errno));
	// 	}
	// 	else
	// 		Printer::debug (string("[")+to_string(i++)+"]"+"Посылаем " +to_string(n)+" байт");
	// 	send(buf, n);
	// }
	// 
	// fclose(content);
	// Printer::debug ("Посылка закончена");
}
void ClientSocket::sendString(const string &body)
{
	int n_written;
	if ((n_written = send (body.c_str(), body.size())) < body.size ()) {
		Printer::error (
			"послали строку длиной "+to_string(body.size())+", но прошло только "+to_string(n_written),
			"[ClientSocket::sendString]"	
		);
		throw SocketException ("Послали строку не полностью");
	}
}
