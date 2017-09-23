#include <iostream>
#include <list>
#include <iterator>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "serv_sock.h"
#include "config.h"
#include "client.h"
#include "helpers.h"
#include "excp.h"
#include "cgiprog.h"

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
		Printer::debug (
			"", 
			Config::section("internal")["server_software"], {
				{"IP", Config::section("network")["server_ip"]},
				{"Port", Config::section("network")["server_port"]}
			}
		);
		Printer::debug ("", "----------------------");
		list <Client *> clients_list;
		SelectionSet set;
		for (;;) {
			set.select(ls, clients_list, CgiProg::getProcNum());
			auto it = clients_list.begin();
			while(it != clients_list.end()) {
				try {
					(*it) -> checkCgiProgs();
					if ((*it) -> isSendingFile ()) {
						Printer::debug ("Клиент ["+to_string((*it)->getNo())+"] принимает файл!");
					}
					else {
						Printer::error ("Клиент ["+to_string((*it)->getNo())+"] не принимает файл!");
					}
					if (set.isReadyToWrite ((*it) -> getSock ())) {
						Printer::debug ("Клиент ["+to_string((*it)->getNo())+"] готов к записи!");
					}
					else {
						Printer::error ("Клиент ["+to_string((*it)->getNo())+"] не готов к записи!");
					}
					// если посылаем файл и готов к записи
					if ((*it) -> isSendingFile () && set.isReadyToWrite ((*it) -> getSock ()))
						(*it) -> respond_or_send ();
					// если пришел запрос по сети либо нужен был ответ ранее
					else if (set.isReadyToRead ((*it) -> getSock ()) || (*it) -> isResponseToClientNeeded ())
						(*it) -> respond_or_send ();
					++it;
				}
				catch(ClientException &a) {
					cerr << "Client[" << (*it) -> getNo() << "]: " << a.what() << endl;
					clients_list.erase(it++);
				}
				catch (SocketException &exc) {
					Printer::error (exc.what (), "[main loop]");
					clients_list.erase (it++);
				}
			}
			if (set.isReadyToRead (ls))
			{
				//принять запрос на соединение, добавить в список
				try {
					Client *next_client = ls.accept ();
					if (clients_list.size () >= atoi (Config::section ("params")["max_clients"].c_str ())) {
						next_client -> getSock ().sendString (Response::response_4xx_5xx (503, "GET"));
						delete next_client;
					}
					else {
						clients_list.push_back (next_client);
					}
				}
				catch(ClientException &a)
				{
					cerr << "Client[" << clients_list.back () -> getNo () << "]: "
						<< a.what() << endl;
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
