#pragma once

#include <string>
#include <sys/stat.h>
#include <time.h>
#include <iostream>
#include <vector>

#include "excp.h"
#include "datetime.h"

using namespace std;

class FileStat
{
	string _path; // full path from root
	string _name; // short name
	size_t _fileSize; // file size
	time_t _modifDate; // last modification time
	bool _isDirectory; // is it a directory?
	bool _isReadable;  // can server read it?
	string _mimeType;  // MIME type associated with the file

	static string mimeType (string &path);
public:
	FileStat (string path);
	string getPath () const { return _path; }
	string getName () const { return _name; }
	size_t getSize() const { return _fileSize; }
	time_t getModifDate() const { return _modifDate; }
	bool isDirectory() const { return _isDirectory; }
	bool isReadable() const { return _isReadable; }
	string getMimeType() const { return _mimeType; }
	string hrModifDate () const {
		// если дата больше, чем неделю
		if (time (NULL) - _modifDate > 24*3600*7) {
			return DateTime::toString (_modifDate, "%a, %e %b %R");
		}
		// больше, чем день назад
		else if (time (NULL) - _modifDate > 24*3600)
			return DateTime::toString (_modifDate, "%a, %R");
		// сегодня
		else {
			return DateTime::toString (_modifDate, "%R");
		}
	}
};

class Directory {
	static const char SLASH = '/';
	static const char *THIS_DIR;
	static const char *PARENT_DIR;
	// removes last slash from directory path
	static void remove_slash (string &path);
	// adds a slash to directory path
	static void add_slash (string &path);
public:
	// returns parent directory for a file
	// parent_dir of ROOT is ROOT
	static string parent_dir (string path);
	// 
	static void remove_first_slash (string &path);
	static const char *ROOT;
	// returns file name embedded in a full path from ROOT
	static string short_name (string full_path);
	// returns a vector of files inside the directory
	// if include_parent is true, then it includes a parent directory
	// sorts by modification date by default
	// folders are pushed first, then files
	static vector<FileStat> ls (string dir_path, bool include_parent = true);
	static void sort_by_modif_date (vector <FileStat> &files);
private:
	// just returns all files in a directory
	static vector <string> list_directory (string &dir_path);
};