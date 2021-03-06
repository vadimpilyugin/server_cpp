#pragma once
#include "sock.h"
#include "timer.h"

#include <arpa/inet.h>
#include "file_to_send.h"
#include <string>

using namespace std;

class ClientSocket: public Socket
{
public:
	ClientSocket (int _sd = -1): Socket (_sd), bytes_sent(0) { buf = new char [BUF_LEN]; }
	~ClientSocket () { delete [] buf; }
	ClientSocket (ClientSocket &&cl_sock) = delete;
	int send (const void *buffer, size_t size);
	int receive (void *buffer, size_t size);
	void sendFile (FileToSend *file_to_send);
	void sendString (const string &body);
	Timer t;
	size_t bytes_sent;
private:
	bool isClosed ();
	char *buf;
	const int BUF_LEN = 8192;
};
