#include "range.h"
#include "helpers.h"
#include "filestat.h"
#include <sstream>

const string Range::RANGE_HEADER = "Range";
const string Range::PATH_HEADER = "Path";


long int Range::long_stoi (std::string s_num) {
  std::stringstream tmp;
  tmp << s_num;
  long int i_num;
  tmp >> i_num;
  return i_num;
}

void Range::parse_bytes_pos (long int &first_n, long int &last_n, string range) {
  // range := "bytes=(\d*)-(\d*)", но не оба нулевые
  //           012345
  range.erase (0, 6);
  //                "(\d*)-(\d*)"
  if (range[0] == '-') {
    //                   "-(\d+)"
    range.erase (0, 1);
    last_n = long_stoi (range);
    first_n = -1;
  }
  else {
    //              "(\d+)-(\d*)"
    first_n = long_stoi (range.substr (0, range.find ("-")));
    range.erase (0, range.find ("-")+1);
    //                    "(\d*)"
    if (range.empty ())
      last_n = -1;
    else
      last_n = long_stoi (range);
  }
}

void Range::get_file_pos (size_t &first_byte_pos, size_t &last_byte_pos,
    long int first_n, long int last_n, size_t file_size) {

  // если начальная позиция не указана
  if (first_n == -1) {
    // выборка последних last_n байт
    // если отступ от конца больше самого файла
    if (last_n > file_size)
      // выравниваем его
      last_n = file_size;
    // первая позиция это размер файла минус величина отступа
    first_byte_pos = file_size - last_n;
    // последняя позиция это конец файла
    last_byte_pos = file_size - 1;
  }
  // начальная позиция указана
  else {
    // если конечная позиция не указана
    if (last_n == -1)
      // она равна концу файла
      last_n = file_size - 1;
    // конечная позиция указана или равна концу файла
    // выборка байт с first_n по last_n
    first_byte_pos = first_n;
    last_byte_pos = last_n;
  }
  // если конечная позиция больше длины файла
  if (last_byte_pos > file_size)
    // выравниваем ее
    last_byte_pos = file_size - 1;
}

int Range::check_range (Hash &request) {

  // если в запросе не содержится интервал
  if (request[RANGE_HEADER].empty()) {
    return ResponseCode::OK;
  }
  
  // присутствует Range
  // выделяем начальный и конечный байты
  long int first_n, last_n;

  // FIXME: FileStat::getSize не определена
  size_t file_size = FileStat::getSize (request[PATH_HEADER]);

  // парсим заголовок
  parse_bytes_pos(first_n, last_n, request[RANGE_HEADER]);
  Printer::debug (
    string("<")+to_string(first_n)+">-<"+to_string(last_n)+">",
   "Распарсили позиции"
  );

  // получаем позиции байт в файле на основе распарсенных данных
  size_t first_byte_pos, last_byte_pos;
  get_file_pos (first_byte_pos, last_byte_pos, first_n, last_n, file_size);

  // если начальная позиция больше конечной
  if (first_byte_pos > last_byte_pos) {
    Printer::error ("Последняя позиция меньше первой", "[check_range]");
    // посылаем 416 Range Not Satisfiable
    return ResponseCode::RANGE_NOT_SATISFIABLE;
  }

  // если дошли до этой точки, то все ок
  return ResponseCode::OK;
}

void Range::get_file_pos (size_t &first_byte_pos, size_t &last_byte_pos, 
  size_t file_size, string range_header) {

  // парсим заголовок
  long int first_n, last_n;
  parse_bytes_pos (first_n, last_n, range_header);
  // возвращаем позиции байтов
  return get_file_pos (first_byte_pos, last_byte_pos, first_n, last_n, file_size);
}