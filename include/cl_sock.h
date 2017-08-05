#pragma once
#include "sock.h"

#include <arpa/inet.h>
#include <string>

using namespace std;

class ClientSocket: public Socket
{
public:
	using Socket::Socket;
	ClientSocket (ClientSocket &&cl_sock): Socket(move(cl_sock)) {}
	void send (const void *buffer, size_t size);
	int receive (void *buffer, size_t size);
	void sendFile (const string &file_name);
	void sendString (const string &body);
private:
	bool isClosed ();
};
