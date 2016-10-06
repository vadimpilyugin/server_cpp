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
using namespace std;

#include "cgiprog.h"
#include "excp.h"
#include "filestat.h"


#include <assert.h>


int CgiProg::_proc_num = 0;

char **CgiProg::vector_to_argv(vector<string> &vect)
{
	size_t size;
	if((size = vect.size()) == 0)
		return nullptr;
	char **argv = new char *[size + 1];
	for(size_t i = 0; i < size; i++){
		argv[i] = new char[vect[i].size() + 1];
		strcpy(argv[i], vect[i].c_str());
	}
	argv[size] = nullptr;
	return argv;
}

void CgiProg::free(char **argv){
	if(argv == nullptr)
		return;
	int i = 0;
	while(argv[i] != nullptr)
		delete [] argv[i++];
	delete [] argv;
}

CgiProg::CgiProg(string path, vector<string> &env_vect, vector<string> &argv_vect)
{
	char **argv = vector_to_argv(argv_vect);
	char **env = vector_to_argv(env_vect);
	{
		stringstream tmp;
		tmp << "tmpfile" << time(NULL) / (_proc_num + 56);
		tmp >> _tmpfile;
	}
	_path = path;
	if((_pid = fork()) < 0)
		throw ServerException("Fork не сработал");
	else if(_pid == 0)
	{
		int fd = creat(_tmpfile.c_str(), 0666);
		if(fd == -1)
			exit(1);
		dup2(fd, 1);
		execve(argv[0], argv, env);
		free(argv);
		free(env);
		close(1);
		remove(_tmpfile.c_str());
		cerr << "Не удалось выполнить exec: " << strerror(errno) << endl;
		exit(2);
	}
	else
	{
		cout << "---Created process #" << _proc_num << endl;
		cout << "---Name = " << _path << endl;
		cout << "---PID = " << _pid << endl;
		cout << endl;
		_proc_num++;
		argv = env = nullptr;
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
		cout << "---Killed by signal? " << boolalpha << WIFSIGNALED(_status) << endl;
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
	else return _isEnded = (waitpid(_pid, &_status, WNOHANG) > 0);
}

bool CgiProg::isExitedCorrect()
{
	return WIFEXITED(_status) && !WEXITSTATUS(_status);
}
