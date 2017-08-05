#include <sstream>
#include "config.h"

#include "helpers.h"

// const string DELIM = "\r\n";
const string HeaderField::DELIM = "\r\n";

string DateTime::getGmtDate(time_t time_in_sec) {
	const int DATE_MAXSIZE = 500;
	const char *format = "%a, %d %b %G %H:%M:%S GMT";
	char *output = new char [DATE_MAXSIZE];
	time_t meantime;
	if(time_in_sec == -1)
		meantime = time(0);
	else
		meantime = time_in_sec;
	struct tm *t = gmtime(&meantime);
	strftime(output, DATE_MAXSIZE, format, t);
	string tmp(output);
	delete [] output;
	return tmp;
}

string Response::response_code (int errcode) {
	string code;
	switch(errcode)
	{
		case 200:
		{
			code = "200 OK";
			break;
		}
		case 400:
		{
			code = "400 Bad Request";
			break;
		}
		case 403:
		{
			code = "403 Forbidden";
			break;
		}
		case 404:
		{
			code = "404 Not Found";
			break;
		}
		case 500:
		{
			code = "500 Internal Server Error";
			break;
		}
		case 501:
		{
			code = "501 Not Implemented";
			break;
		}
		case 503:
		{
			code = "503 Service Unavailable";
			break;
		}
	}
	return code;
}

string Response::response_body (int errcode) {
	string info;
	switch(errcode)
	{
		case 400:
		{
			info = "This is a Bad Request\n";
			break;
		}
		case 403:
		{
			info = "You have no permission to access this directory or file\n";
			break;
		}
		case 404:
		{
			info = "The specified location is not found\n";
			break;
		}
		case 500:
		{
			info = "Server error. Service unavailable\n";
			break;
		}
		case 501:
		{
			info = "This method is not supported.\n";
			info += "The supported methods are: "+Config::section("internal")["supported_methods"]+"\n";
			break;
		}
		case 503:
		{
			info = "Too many clients. Service is Unavailable\n";
			break;
		}
	}
	return info;
}

string Response::response_200 (string method, time_t modif_date, string content_type, int content_length) {
	/*
	Date, Server,
	Content-type-для GET, 
	Content-length – для GET,
	Last-modified,
	тело – для GET
	*/
	stringstream resp;
	resp << Config::section("internal")["server_protocol"] << " " << response_code (200) << HeaderField::DELIM;
	resp << HeaderField::date_field ();
	resp << HeaderField::server_field ();
	if (method == "GET") {
		resp << HeaderField::content_type_field (content_type);
		resp << HeaderField::content_length_field (content_length);
	}
	resp << HeaderField::last_modified_field (modif_date);
	resp << HeaderField::DELIM;
	return resp.str();
}
string Response::response_4xx_5xx (int errcode, string method) {
	/*
	Date, Server,
	Content-type,
	Content-length,
	тело
	*/
	string body = response_body (errcode);
	int content_length = body.size ();
	string content_type = "text/plain";

	stringstream resp;
	resp << Config::section("internal")["server_protocol"] << " " << response_code (errcode) << HeaderField::DELIM;
	resp << HeaderField::date_field ();
	resp << HeaderField::server_field ();
	if (errcode == 501) {
		resp << HeaderField::allow_field ();
	}
	if (method == "GET") {
		resp << HeaderField::content_type_field (content_type);
		resp << HeaderField::content_length_field (content_length);
	}
	resp << HeaderField::DELIM;
	if (method == "GET") {
		resp << body;
	}
	return resp.str();
}
