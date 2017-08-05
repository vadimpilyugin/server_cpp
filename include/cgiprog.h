#pragma once

#include <vector>
#include <string>
using namespace std;

class CgiProg
{
	static int _proc_num;
	int _pid = -1;
	bool _isEnded = false;
	int _status = 0;
	string _path;
	string _tmpfile;
public:
	CgiProg(string path, vector<string> &env_vect, vector<string> &argv_vect);
	~CgiProg();
	CgiProg(CgiProg &&p);
	CgiProg(CgiProg &p) = delete;
	bool isProcessEnded();
	bool isExitedCorrectly();
	static int getProcNum() {return _proc_num;}
	string getPath() {return _path;}
	string getOutputFile() {return _tmpfile;}
private:
	char **vector_to_argv(vector<string> &vect);
	void free(char **argv);
};
