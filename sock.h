#ifndef SOCK_H_INCLUDED
#define SOCK_H_INCLUDED

class Socket
{
	friend class SelectionSet;
protected:
	int _sd;
public:
	Socket(int sd = -1);
	~Socket();
	Socket(Socket &&sock) {_sd = sock._sd; sock._sd = -1;}
	Socket(Socket &sock) = delete;
};

#endif // SOCK_H_INCLUDED
