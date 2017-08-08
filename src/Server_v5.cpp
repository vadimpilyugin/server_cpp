#include "serv_sock.h"
#include "config.h"
#include "client.h"
#include "helpers.h"
#include "excp.h"
#include "cgiprog.h"

#include <iostream>
#include <list>
#include <iterator>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

using namespace std;



int main()
{
	try
	{
		Config::load ("./config.cfg");
		// chdir ("/");
		chdir (Config::section("internal")["root_dir"].c_str ());
		srand (time (NULL));
		ServerSocket ls(Config::section("network")["server_ip"], stoi (Config::section("network")["server_port"].c_str ()));
		Printer::debug ("", Config::section("internal")["server_software"], {
			{"IP", Config::section("network")["server_ip"]},
			{"Port", Config::section("network")["server_port"]}
		});
		Printer::debug ("", "----------------------");
		list <Client> clients_list;
		SelectionSet set;
		while (true)
		{
			set.select(ls, clients_list, CgiProg::getProcNum());
			auto it = clients_list.begin();
			while(it != clients_list.end())
			{
				it -> checkCgiProgs();
				if(set.isReady(it -> getSock()))
				{
					//выполнить обмен с клиентом, удалить отключившихся
					try
					{
						while(it -> respond() == true);
						++it;
					}
					catch(ClientException &a)
					{
						cerr << "Client["<<it->getNo() <<"]: " << a.what() << endl;
						clients_list.erase(it++);
					}
					catch (SocketException &exc) {
						Printer::error (exc.what (), "SocketException");
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
					if(clients_list.size() >= atoi (Config::section ("params")["max_clients"].c_str ())) {
						ls.accept().getSock().sendString (Response::response_4xx_5xx (503, "GET"));
					}
					else
						clients_list.push_back(ls.accept());
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
