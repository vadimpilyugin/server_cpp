#pragma once

#include <string>
#include "printer.hpp"
using namespace std;

class Range {
  static const string RANGE_HEADER;
  static const string PATH_HEADER;

  static void get_file_pos (size_t &first_byte_pos, size_t &last_byte_pos,
    long int first_n, long int last_n, size_t file_size);
  static void parse_bytes_pos (long int &, long int &, string range);
  static long int long_stoi (std::string s_num);
public:
  static bool has_range (Hash &request) { return !request[RANGE_HEADER].empty (); }
  static int check_range (Hash &request);
  static void get_file_pos (size_t &first_byte_pos, size_t &last_byte_pos, 
    size_t file_size, string range_header);
};