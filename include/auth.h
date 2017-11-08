#pragma once

#include "printer.hpp"

class Auth {
  static const std::string cred_base64;
public:
  static bool check_credentials (std::string cred) {
    return cred == cred_base64;
  }
  static bool authorized (Hash &request) {
    return request["Authorization"] == cred_base64;
  }
};