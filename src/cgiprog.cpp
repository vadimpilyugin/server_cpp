#include "cgiprog.h"
#include "excp.h"
#include "filestat.h"
#include "printer.hpp"

#include <vector>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>
#include <cerrno>
#include <assert.h>

using namespace std;

int CgiProg::_proc_num = 0;

char **CgiProg::vector_to_argv(vector<string> &vect)
{
	size_t size;
	if((size = vect.size()) == 0)
		return nullptr;
	char **argv = new char *[size + 1];
	for(size_t i = 0; i < size; i++) {
		argv[i] = new char[vect[i].size() + 1];
		strcpy(argv[i], vect[i].c_str());
	}
	argv[size] = nullptr;
	return argv;
}

void CgiProg::free(char **argv) {
	if(argv == nullptr)
		return;
	int i = 0;
	while(argv[i] != nullptr)
		delete [] argv[i++];
	delete [] argv;
}

CgiProg::CgiProg(string path, vector<string> &env_vect, vector<string> &argv_vect): _path(path)
{
	_tmpfile = std::string ("/var/tmp/tmpfile") + std::to_string (rand ());
	if((_pid = fork()) < 0) {
		Printer::error ("fork() не сработал");
		throw ServerException("fork не сработал");
	}
	else if(_pid == 0)
	{
		int fd = creat(_tmpfile.c_str(), 0666);
		if(fd == -1) {
			Printer::error (strerror (errno), "Temporary file cannot be created");
			exit (EXIT_FAILURE);
		}
		char **argv = vector_to_argv(argv_vect);
		char **env = vector_to_argv(env_vect);
		dup2(fd, 1);
		execve(argv[0], argv, env);
		// if execve didn't work
		free(argv);
		free(env);
		close(1);
		// Printer::error (strerror (errno), "Не удалось выполнить exec");
		exit (EXIT_FAILURE);
	}
	else
	{
		_proc_num++;
		cout << "---Created process #" << _proc_num << endl;
		cout << "---Name = " << _path << endl;
		cout << "---PID = " << _pid << endl;
		cout << "---Output = " << _tmpfile << endl;
		cout << endl;
	}
}

CgiProg::~CgiProg()
{
	if(_pid > 0)
	{
		if(!isProcessEnded())
			kill(_pid, SIGKILL);
		waitpid(_pid, &_status, WNOHANG);
		cout << "---Killed process:" << endl;
		cout << "---Name = " << _path << endl;
		cout << "---PID = " << _pid << endl;
		cout << "---Exit Status: " << WEXITSTATUS(_status) << endl;
		cout << "---Killed by signal? " << (WIFSIGNALED(_status)?"Yes":"No") << endl;
		cout << endl;
		remove(_tmpfile.c_str());
		_proc_num--;
	}
}

CgiProg::CgiProg(CgiProg &&p)
{
	_pid = p._pid;
	p._pid = -1;
	_isEnded = p._isEnded;
	_status = p._status;
	_path = move(p._path);
	_tmpfile = move(p._tmpfile);
}

bool CgiProg::isProcessEnded()
{
	if(_isEnded)
		return true;
	else 
		return _isEnded = (waitpid(_pid, &_status, WNOHANG) > 0);
}

bool CgiProg::isExitedCorrectly()
{
	return WIFEXITED(_status) && !WEXITSTATUS(_status);
}
