#ifndef FILESTAT_H_INCLUDED
#define FILESTAT_H_INCLUDED

#include <string>
#include <sys/stat.h>
#include <iostream>
using namespace std;

#include "excp.h"


class FileStat
{
	struct stat _file_attrib;
	string _path;
public:
	FileStat(const string &path): _path(path)
	{
		if(stat(path.c_str(), &_file_attrib) == -1)
			throw FileException(path);
	}
	bool isDirectory() {return S_ISDIR(_file_attrib.st_mode);}
	size_t getSize() {return _file_attrib.st_size;}
	time_t getModifDate() {return _file_attrib.st_mtime;}
	string getFileType() const;
	static string getCwd();
	bool isReadable() {return _file_attrib.st_mode & (S_IRGRP | S_IRUSR | S_IROTH);}
	static string getFullPath(string &relative_path);
};

#endif // FILESTAT_H_INCLUDED
