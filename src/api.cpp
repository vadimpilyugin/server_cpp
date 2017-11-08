#include "api.h"
#include "filestat.h"
#include "embedded.h"
#include "helpers.h"
#include "json.hpp"

using json = nlohmann::json;

const string Api::API_HEADER = "X-Custom";
const string Api::API_VALUE = "vuejs";

string elem_pic (bool isDirectory) {
  if (isDirectory)
    return folder_base64;
  else
    return file_base64;
}

json jsonify_element (FileStat &fst) {
  return json::object ({
    {"name", fst.getName ()},
    {"size", fst.getSize ()},
    {"path", fst.getPath ()},
    {"modif_date", fst.getModifDate ()},
    {"isDirectory", fst.isDirectory ()},
    {"mime_type", fst.getMimeType ()},
    {"hr_modif_date", fst.hrModifDate ()},
    {"hr_size", FileStat::pp_size (fst.getSize ())}, 
    {"pic", elem_pic (fst.isDirectory ())},
    {"url", UrlEncoder::url_encode (fst.getPath (), false)}
  });
}

string Api::jsonDirList (string path) {
  json ls_json = json::array ();
  Printer::debug (path, "Listing directory");
  auto dir_list = Directory::ls (path, true);
  // эта директория
  path = path.substr(1);
  ls_json.push_back (json::object ({{"current_name", path}}));
  // ls_json.push_back (jsonify_element (dir_list.front ()));
  for (int i = 0; i < dir_list.size (); i++) {
    auto fst = dir_list[i];
    // Printer::debug ("", "Found new file", {
    //   {"Name", fst.getName ()},
    //   {"Size", to_string (fst.getSize ())}
    // });
    ls_json.push_back (jsonify_element (fst));
  }
  // Printer::debug (ls_json.dump ());
  return ls_json.dump ();
  // return "{\"foo\": \"bar\"}";
}