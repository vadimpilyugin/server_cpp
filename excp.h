#ifndef EXCP_H_INCLUDED
#define EXCP_H_INCLUDED

#include <string>
#include <cerrno>
#include <cstring>
using namespace std;

class Excp
{
	int _errcode;
	string _comment;
public:
	Excp(const string &comment = "", int err = errno): _errcode(err), _comment(comment) {};
	Excp(const Excp &a): _errcode(a._errcode), _comment(a._comment) {};
	string strerr() const {return strerror(_errcode);}
	int getErrno() const {return _errcode;}
	const string &what() const {return _comment;}
};

class AddressException: public Excp
{
	int _port;
public:
	AddressException(int port, string comment) : Excp(comment), _port(port) {};
	int getPort() const {return _port;};
};

class ClientException: public Excp
{
	using Excp::Excp;
};

class SocketException: public Excp
{
	using Excp::Excp;
};

class ServerException: public Excp
{
	using Excp::Excp;
};

class FileException: public Excp
{
	using Excp::Excp;
};

#endif // EXCP_H_INCLUDED
