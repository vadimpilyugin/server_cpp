#pragma once

#include "printer.hpp"
#include <string>

using namespace std;

class Api {
  static const string API_HEADER; 
  static const string API_VALUE; 

public:
  static bool is_api_request (Hash &request) { return request[API_HEADER] == API_VALUE; }
  static string jsonDirList (string path);
};