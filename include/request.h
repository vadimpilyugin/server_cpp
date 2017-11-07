#pragma once

#include <string>
#include <vector>
#include "printer.hpp"
#include "cl_sock.h"

using namespace std;

class Request {
  static const string ROOT_DIR;
  static const string USER_AGENT;
  static const string REFERER;
  static const string PARAMS;
  static const string HTTP_VER;
public:
  static const string PATH_HEADER;
  static const string RANGE_HEADER;
  static const string METHOD_HEADER;
  static const string GET_METHOD;
  static const string HEAD_METHOD;
  static int process_request (Hash &request);
  static Hash parse_request (string request);
  static vector<string> fill_cgi_env (Hash &request, SocketAddress &addr);
};