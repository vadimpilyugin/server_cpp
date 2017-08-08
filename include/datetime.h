#pragma once

#include <sys/time.h>
#include <string>
using namespace std;

class DateTime {
	static const int DATE_MAXSIZE = 500;
public:
	static string getGmtDate (time_t time_in_sec = -1);
	static string toString (time_t time_in_sec, const char *format);
};