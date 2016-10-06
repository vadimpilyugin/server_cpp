#include <string>
#include <unistd.h>
using namespace std;

#include "filestat.h"

string FileStat::getFileType(const string &path)
{
	if(path.find("cgi-bin") != string::npos)
		return "text/plain";
	int dot = path.find_last_of('.');
	string file_format;
	if(dot != string::npos)
		file_format = path.substr(dot, string::npos);
	else 
		return "text/plain";
	if(file_format == ".jpg")
		return "image/jpg";
	else if(file_format == ".html")
		return "text/html";
	else
		return "text/plain";
}

string FileStat::getCwd()
{
	char *cwd = new char[256];
	getcwd(cwd, 256);
	string tmp = string(cwd);
	delete [] cwd;
	return tmp;
}
string FileStat::getFullPath(string &relative_path)
{
	string curr_dir = getCwd();
	bool slash1 = (curr_dir.back() == '/'), slash2 = (relative_path.front() == '/');
	if(slash1 || slash2)
		return curr_dir + relative_path;
	else if(slash1 && slash2)
	{
		curr_dir.pop_back();
		return curr_dir + relative_path;
	}
	else if(!slash1 && !slash2)
		return curr_dir + string("/") + relative_path;
}
