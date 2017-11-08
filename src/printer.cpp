#include "printer.hpp"

bool Printer::prev_in_place = false;

const char Printer::black[]   = { 0x1b, '[', '1', ';', '3', '0', 'm', 0 };
const char Printer::red[]     = { 0x1b, '[', '1', ';', '3', '1', 'm', 0 };
const char Printer::green[]   = { 0x1b, '[', '1', ';', '3', '2', 'm', 0 };
const char Printer::yellow[]  = { 0x1b, '[', '1', ';', '3', '3', 'm', 0 };
const char Printer::blue[]    = { 0x1b, '[', '1', ';', '3', '4', 'm', 0 };
const char Printer::magenta[] = { 0x1b, '[', '1', ';', '3', '5', 'm', 0 };
const char Printer::cyan[]    = { 0x1b, '[', '1', ';', '3', '6', 'm', 0 };
const char Printer::white[]   = { 0x1b, '[', '1', ';', '3', '7', 'm', 0 };

// detail::
const std::string Printer::detail::debug_msg  = std::string("Debug");
const std::string Printer::detail::assert_msg = std::string("Assertion failed");
const std::string Printer::detail::error_msg  = std::string("Error");
const std::string Printer::detail::fatal_msg  = std::string("Fatal error");
const std::string Printer::detail::note_msg   = std::string("Note");
const std::string Printer::detail::empty_msg  = std::string("");

const std::string Printer::detail::delim = std::string(": ");
const std::string Printer::detail::cr = std::string("\r");
const std::string Printer::detail::lf = std::string("\n");
const std::string Printer::detail::tab = std::string("\t");
const std::string Printer::detail::space = std::string(" ");

/*
* Функция вывода сообщения с именем отправителя. Занимается форматированием, цветом, выводом
*/
void Printer::detail::generic(  
  const std::string &msg, const std::string &who, 
  const std::string &who_color,
  const bool in_place, const Hash &params, 
  const std::string &msg_color,
  const std::string &delim, const bool newline) {

  // если предыдущая запись была in place, а текущая - нет
  // if (prev_in_place && !in_place) {
  //   // очищаем строку
  //   std::cerr << std::endl << std::endl;
  //   prev_in_place = false;
  //   std::cerr << "Cleared in_place";
  // }
  if (in_place) {
    prev_in_place = true;
  }
  else if (prev_in_place) {
    prev_in_place = false;
    std::cerr << std::endl;
  }
  std::cerr << who_color << who << delim << msg_color << msg;
  if (in_place) {
    for (int i = 0; i < 5; i++)
      std::cerr << detail::space;
  }
  if(newline) {
    if(in_place) {
      std::cerr << detail::cr;
    }
    else {
      std::cerr << std::endl;
      for (const auto &pair: params) {
        std::cerr << detail::tab << green << pair.first << delim << white << pair.second << std::endl;
      }
    }
  }
}

void Printer::debug(const std::string msg, const std::string who, const Hash &params, const bool in_place) {
  #if DEBUG
  detail::generic(msg, who, green, in_place, params);
  #endif
}
void Printer::assert(const bool expr, std::string msg, const std::string who, const Hash &params) {
  if(!expr) {
    detail::generic(msg, who, red, false, params);
    throw AssertException(msg);
  }
}
void Printer::note(const std::string msg, const std::string who, const Hash &params, const bool in_place) {
  detail::generic(msg, who, yellow, in_place, params);
}
void Printer::error(const std::string msg, const std::string who, const Hash &params) {
  detail::generic(msg, who, red, false, params);
}
void Printer::fatal(const std::string msg, const std::string who, const Hash &params)
{
  detail::generic(msg, who, red, false, params);
  throw FatalException(msg);
}
void Printer::prompt(const std::string prompt_msg) {
  detail::generic(detail::empty_msg, prompt_msg, white, false, {}, white, "> ", false);
}