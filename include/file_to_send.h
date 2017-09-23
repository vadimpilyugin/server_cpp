#pragma once
#include <stdio.h>
#include <errno.h>
#include <string>

using namespace std;

struct FileToSend {

	string path;
	size_t first_byte;
	size_t last_byte;
	size_t file_size;
	size_t portion_size;
	size_t current_pos;
	size_t bytes_left;

	FileToSend (string _path, size_t first_byte_pos, size_t last_byte_pos);
	~FileToSend () { fclose (fp); }
	size_t fread (size_t n_bytes, char *buf);
	string to_s () { return path; };
	bool is_anything_left () { return bytes_left > 0; }
	bool is_finished () { return !is_anything_left (); }
private:
	FILE *fp;
};
