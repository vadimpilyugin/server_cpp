#include "datetime.h"

string DateTime::getGmtDate (time_t time_in_sec) {
	return toString (time_in_sec == -1 ? time(0) : time_in_sec, "%a, %d %b %G %H:%M:%S GMT");
}

string DateTime::toString (time_t time_in_sec, const char *format) {
	char *output = new char [DATE_MAXSIZE];
	strftime(output, DATE_MAXSIZE, format, gmtime(&time_in_sec));
	string tmp(output);
	delete [] output;
	return tmp;
}