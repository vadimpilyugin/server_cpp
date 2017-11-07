#include "request.h"
#include "helpers.h"
#include "auth.h"
#include "api.h"
#include "config.h"
#include "filestat.h"
#include "range.h"

#include <sstream>

const string Request::PATH_HEADER = "Path";
const string Request::METHOD_HEADER = "Method";
const string Request::HTTP_VER = "Http-ver";
const string Request::PARAMS = "Params";
const string Request::GET_METHOD = "GET";
const string Request::HEAD_METHOD = "HEAD";
const string Request::ROOT_DIR = "/";
const string Request::USER_AGENT = "User-Agent";
const string Request::REFERER = "Referer";
const string Request::RANGE_HEADER = "Range";

int Request::process_request (Hash &request) {

  // если метод не GET и не HEAD
  if (
    request[METHOD_HEADER] != GET_METHOD && 
    request[METHOD_HEADER] != HEAD_METHOD) {

    Printer::error (Response::response_code (ResponseCode::NOT_IMPLEMENTED));
    return ResponseCode::NOT_IMPLEMENTED;
  }
  // метод GET или HEAD
  // проверяем, не запрос ли это от фронтэнда
  bool isApi = Api::is_api_request (request);

  // если это запрос рутовой папки и не от фронтэнда
  if (request[PATH_HEADER] == ROOT_DIR && !isApi) {
    // меняем ее на дефолтный путь
    request[PATH_HEADER] = Config::section ("network")["default_page"];
  }
  // делаем путь относительным, добавляя точку перед первым слэшом
  Directory::remove_first_slash (request[PATH_HEADER]);

  // если путь не существует
  if (!FileStat::isExists (request[PATH_HEADER])) {
    Printer::error (Response::response_code (ResponseCode::NOT_FOUND));
    return ResponseCode::NOT_FOUND;
  }
  // путь существует
  // если путь недоступен для чтения
  if (!FileStat::isReadable (request[PATH_HEADER])) {
    Printer::error (Response::response_code (ResponseCode::FORBIDDEN));
    return ResponseCode::FORBIDDEN;
  }
  // путь доступен для чтения
  // проверяем на правильность указания Range
  int code;
  if ((code = Range::check_range (request)) != ResponseCode::OK) {
    Printer::error (Response::response_code (code));
    return code;
  }
  // проверяем авторизацию, если запрашивают содержимое папки
  //if (!Auth::authorized (request) && FileStat::isDirectory (request[PATH_HEADER])) {
  //  Printer::error (Response::response_code (ResponseCode::NOT_AUTHORIZED));
  //  return ResponseCode::NOT_AUTHORIZED;
  //}
  // клиент авторизован
  // возвращаем успех
  Printer::debug("Запрос корректный");
  return ResponseCode::OK;
}

Hash Request::parse_request (string request) {
  const char QUERY_STRING_DELIM = '?';
  const char HEADER_DELIM = ':';
  const char CR = '\r';
  const char EOL = '\n';
  const string HTTP_REQUEST_END = "\r\n\r\n";


  Hash result;
  string method, path, params, http_ver;
  stringstream request_stream;
  if (request.find (QUERY_STRING_DELIM) != string::npos) {
    request[request.find (QUERY_STRING_DELIM)] = ' ';
    request_stream.str (request);
    request_stream >> method >> path >> params >> http_ver;
  }
  else {
    request_stream.str (request);
    request_stream >> method >> path >> http_ver;
  }
  path = UrlEncoder::url_decode (path);
  Printer::note (path, "Decoded path");
  result[METHOD_HEADER] = method;
  result[PATH_HEADER] = path;
  result[PARAMS] = params;
  result[HTTP_VER] = http_ver;
  size_t begin_pos = request.find (EOL);
  size_t fin = request.find (HTTP_REQUEST_END);
  // Printer::debug (std::to_string (fin+1), "End position");
  if (begin_pos != fin+1)
    begin_pos++;
  while (begin_pos < fin+1) {
    string name, value;
    size_t delim_pos = request.find (HEADER_DELIM, begin_pos);
    size_t end_pos = request.find (CR, begin_pos);
    name = request.substr (begin_pos, delim_pos - begin_pos);
    value = request.substr (delim_pos + 2, end_pos - (delim_pos + 2));
    result[name] = value;
    begin_pos = request.find (EOL, begin_pos);
    if (begin_pos != string::npos)
      begin_pos++;
    // Printer::debug (std::to_string (begin_pos), "Begin pos");
  }
  return result;
}

vector<string> Request::fill_cgi_env (Hash &request, SocketAddress &addr) {
  Hash cgi_env = {
    {"CONTENT-TYPE", "text/plain"},
    {"GATEWAY-INTERFACE", Config::section("internal")["gateway_interface"]}, 
    {"REMOTE_ADDR", addr.getIp()},
    {"REMOTE_PORT", std::to_string (addr.getPort())},
    {"QUERY_STRING", request[PARAMS]},
    {"SERVER_ADDR", Config::section("network")["server_ip"]},
    {"SERVER_NAME", Config::section("network")["server_name"]},
    {"SERVER_PORT", Config::section("network")["server_port"]},
    {"SERVER_PROTOCOL", Config::section("internal")["server_protocol"]},
    {"SERVER_SOFTWARE", Config::section("internal")["server_software"]},
    {"SCRIPT_NAME", request[PATH_HEADER]},
    {"SCRIPT_FILENAME", request[PATH_HEADER]},
    {"DOCUMENT_ROOT", Directory::ROOT},
    {"HTTP_USER_AGENT", request[USER_AGENT]},
    {"HTTP_REFERER", request[REFERER]}
  };
  vector <string> env;
  for (const auto &pair: cgi_env) {
    env.push_back (pair.first + std::string ("=") + pair.second);
  }
  return env;
}
