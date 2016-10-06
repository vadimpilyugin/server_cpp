#include <iostream>
#include <list>
#include <iterator>
using namespace std;

#include "serv_sock.h"
#include "client.h"
#include "excp.h"
#include "cgiprog.h"


int main()
{
	try
	{
		ServerSocket ls("127.0.0.1", 16000);
		cout << "HTTP Model Server v0.4" << endl;
		cout << "IP: 127.0.0.1" << endl;
		cout << "Port: 16000" << endl;
		cout << "----------------------" << endl;
		list<Client> clients_list;
		SelectionSet set;
		for(;;)
		{
			set.Select(ls, clients_list, CgiProg::getProcNum());
			auto it = clients_list.begin();
			while(it != clients_list.end())
			{
				it -> checkCgiProgs();
				if(set.isReady(it -> getSock()))
				{
					//выполнить обмен с клиентом, удалить отключившихся
					try
					{
						while(it -> Respond() == true);
						++it;
					}
					catch(ClientException &a)
					{
						cerr << "Client["<<it->getNo() <<"]: " << a.what() << endl;
						clients_list.erase(it++);
					}
				}
				else
				{
					++it;
				}
			}
			if(set.isReady(ls))
			{
				//принять запрос на соединение, добавить в список
				try
				{
					if(clients_list.size() >= ServerSocket::MAX_CLNTS)
						ls.Accept().SendHeader(503); //Service Unavailable
					else
						clients_list.push_back(ls.Accept());
				}
				catch(ClientException &a)
				{
					cerr << "Client[" << clients_list.back().getNo() << "]: "
							<< a.what() << endl;
					clients_list.pop_back();
				}
			}
		}
	}
	catch(AddressException &a)
	{
		cerr << "Address Exception: " << a.what() << "(" << a.strerr() << ")" << endl
			<< "Port: "<< a.getPort() << endl;
		return 1;
	}
	catch(SocketException &a)
	{
		cerr << "Socket Exception: " << a.what() << '(' << a.strerr() << ')'<<endl;
		return 1;
	}
	catch(ServerException &a)
	{
		cerr << "Server Exception: " << a.what() << '(' << a.strerr() << ')'<<endl;
		return 1;
	}
}
