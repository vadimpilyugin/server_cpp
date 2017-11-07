#pragma once
#define DEBUG 1

#include <string>
#include <unordered_map>
#include <iostream>
#include <exception>

using Hash = std::unordered_map<std::string,std::string>;

namespace Printer {

  /*
  * Исключение, бросаемое в fatal или assert
  */ 

  class Exception: public std::exception
  {
    std::string exc_msg;
  public:
    Exception(std::string _msg = std::string()): exc_msg(_msg) {}
    virtual const char *what() const noexcept {
      return exc_msg.c_str();
    }
  };

  class AssertException: public Exception
  {
    using Exception::Exception;
  };
  class FatalException: public Exception
  {
    using Exception::Exception;
  };

  // равен true, когда предыдущая запись была in place
  static bool prev_in_place = false;

  static const char black[]   = { 0x1b, '[', '1', ';', '3', '0', 'm', 0 };
  static const char red[]     = { 0x1b, '[', '1', ';', '3', '1', 'm', 0 };
  static const char green[]   = { 0x1b, '[', '1', ';', '3', '2', 'm', 0 };
  static const char yellow[]  = { 0x1b, '[', '1', ';', '3', '3', 'm', 0 };
  static const char blue[]    = { 0x1b, '[', '1', ';', '3', '4', 'm', 0 };
  static const char magenta[] = { 0x1b, '[', '1', ';', '3', '5', 'm', 0 };
  static const char cyan[]    = { 0x1b, '[', '1', ';', '3', '6', 'm', 0 };
  static const char white[]   = { 0x1b, '[', '1', ';', '3', '7', 'm', 0 };

  namespace detail {
    /*
    * Сообщения по умолчанию, выводящиеся перед двоеточием
    */
    static const std::string debug_msg  = std::string("Debug");
    static const std::string assert_msg = std::string("Assertion failed");
    static const std::string error_msg  = std::string("Error");
    static const std::string fatal_msg  = std::string("Fatal error");
    static const std::string note_msg   = std::string("Note");
    static const std::string empty_msg  = std::string("");
    /*
    * Символы для разделения сообщения и отправителя
    */
    static const std::string delim = std::string(": ");
    /*
    * Символы для перевода строки
    */
    static const std::string cr = std::string("\r");
    static const std::string lf = std::string("\n");
    // static const std::string debug_msg_color  = std::string(green);
    // static const std::string assert_msg_color = std::string(red);
    // static const std::string error_msg_color  = std::string(red);
    // static const std::string fatal_msg_color  = std::string(red);
    // static const std::string note_msg_color   = std::string(yellow);
    // static const std::string msg_color        = std::string(white);
    /*
    * Функция вывода сообщения с именем отправителя. Занимается форматированием, цветом, выводом
    */
    static void generic(  const std::string &msg, const std::string &who, const std::string &who_color,
                          const bool in_place = false, const Hash &params = {}, 
                          const std::string &msg_color = white,
                          const std::string &delim = detail::delim, const bool newline = true) {

      // если предыдущая запись была in place, а текущая - нет
      if (prev_in_place && !in_place) {
        // очищаем строку
        std::cerr << detail::lf << detail::lf;
        prev_in_place = false;
      }
      std::cerr << who_color << who << delim << msg_color << msg;
      if(newline) {
        if(in_place) {
          prev_in_place = true;
          std::cerr << detail::cr;
        }
        else {
          std::cerr << detail::lf;
          for (const auto &pair: params) {
            std::cerr << "\t" << green << pair.first << delim << white << pair.second << detail::lf;
        }
        }
      }
    }
  }
  /*
  * Описания функций вывода
  * msg - сообщение, которое нужно вывести
  * who - то, что стоит перед :
  * in_place - \r вместо \n в конце строки. Нужно для счетчиков
  * expr - условие, проверяемое в assert
  */

  static void debug(const std::string msg = detail::empty_msg, const std::string who = detail::debug_msg, const Hash &params = {}, const bool in_place = false) {
    #if DEBUG
    detail::generic(msg, who, green, in_place, params);
    #endif
  }
  static void assert(const bool expr, std::string msg = detail::empty_msg, const std::string who = detail::assert_msg, const Hash &params = {}) {
    if(!expr) {
      detail::generic(msg, who, red, false, params);
      throw AssertException(msg);
    }
  }
  static void note(const std::string msg = detail::empty_msg, const std::string who = detail::note_msg, const Hash &params = {}, const bool in_place = false) {
    detail::generic(msg, who, yellow, in_place, params);
  }
  static void error(const std::string msg = detail::empty_msg, const std::string who = detail::error_msg, const Hash &params = {}) {
    detail::generic(msg, who, red, false, params);
  }
  static void fatal(const std::string msg = detail::empty_msg, const std::string who = detail::fatal_msg, const Hash &params = {})
  {
    detail::generic(msg, who, red, false, params);
    throw FatalException(msg);
  }
  static void prompt(const std::string prompt_msg = detail::empty_msg) {
    detail::generic(detail::empty_msg, prompt_msg, white, false, {}, white, "> ", false);
  }
};
