#include <sys/types.h>#include <sys/socket.h>
#include <unistd.h>using namespace std;

#include "sock.h"
#include "excp.h"
Socket::Socket(int sd){    if(sd >= 0)        _sd = sd;    else if((_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)        throw SocketException("Не удалось создать сокет");}Socket::~Socket(){    if(_sd >= 0)    {        shutdown(_sd, 2);        close(_sd);    }}