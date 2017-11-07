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
	list <Client *> clients_list;
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
		SelectionSet set;
		for (;;) {
			set.select(ls, clients_list, CgiProg::getProcNum());
			auto it = clients_list.begin();
			while(it != clients_list.end()) {
				try {
					(*it) -> checkCgiProgs();
					// если клиент прислал очередную порцию данных
					if (set.isReadyToRead ((*it) -> getSock ())) {
						Printer::debug ("Клиент ["+to_string((*it)->getNo())+"] готов к чтению!");
						// (*it) -> respond_or_send ();
						// добавляем данные в буфер
						(*it) -> appendRequest ();
					}
					// если посылаем файл и готов к записи
					if ((*it) -> isSendingFile () && set.isReadyToWrite ((*it) -> getSock ())) {
						// Printer::debug ("Клиент ["+to_string((*it)->getNo())+"] принимает файл и готов к записи!");
						(*it) -> continue_file_sending ();
					}
					// если в буфере есть готовый запрос и мы закончили отвечать на предыдущий запрос
					else if (!(*it) -> isSendingFile () && (*it) -> isResponseToClientNeeded ()) {
						// отвечаем клиенту
						(*it) -> respond ();
					}
					++it;
				}
				catch(ClientException &a) {
					cerr << "Client[" << (*it) -> getNo() << "]: " << a.what() << endl;
					delete (*it);
					clients_list.erase(it++);
				}
				catch (SocketException &exc) {
					Printer::error (exc.what (), "[main loop]");
					delete (*it);
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
		for (auto it: clients_list) {
			delete it;
		}
		return 1;
	}
	catch(SocketException &a)
	{
		cerr << "Socket Exception: " << a.what() << '(' << a.strerr() << ')'<<endl;
		for (auto it: clients_list) {
			delete it;
		}
		return 1;
	}
	catch(ServerException &a)
	{
		cerr << "Server Exception: " << a.what() << '(' << a.strerr() << ')'<<endl;
		for (auto it: clients_list) {
			delete it;
		}
		return 1;
	}
}
