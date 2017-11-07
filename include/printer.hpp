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
  extern bool prev_in_place;

  extern const char black[];
  extern const char red[];
  extern const char green[];
  extern const char yellow[];
  extern const char blue[];
  extern const char magenta[];
  extern const char cyan[];
  extern const char white[];

  namespace detail {
    /*
    * Сообщения по умолчанию, выводящиеся перед двоеточием
    */
    extern const std::string debug_msg;
    extern const std::string assert_msg;
    extern const std::string error_msg;
    extern const std::string fatal_msg;
    extern const std::string note_msg;
    extern const std::string empty_msg;
    /*
    * Символы для разделения сообщения и отправителя
    */
    extern const std::string delim;
    /*
    * Символы для перевода строки
    */
    extern const std::string cr;
    extern const std::string lf;
    extern const std::string tab;
    extern const std::string space;
    /*
    * Функция вывода сообщения с именем отправителя. Занимается форматированием, цветом, выводом
    */
    void generic(  const std::string &msg, const std::string &who, const std::string &who_color,
                          const bool in_place = false, const Hash &params = {}, 
                          const std::string &msg_color = white,
                          const std::string &delim = detail::delim, const bool newline = true);
  }

  /*
  * Описания функций вывода
  * msg - сообщение, которое нужно вывести
  * who - то, что стоит перед :
  * in_place - \r вместо \n в конце строки. Нужно для счетчиков
  * expr - условие, проверяемое в assert
  */

  void debug(const std::string msg = detail::empty_msg, const std::string who = detail::debug_msg, const Hash &params = {}, const bool in_place = false);
  void assert(const bool expr, std::string msg = detail::empty_msg, const std::string who = detail::assert_msg, const Hash &params = {});
  void note(const std::string msg = detail::empty_msg, const std::string who = detail::note_msg, const Hash &params = {}, const bool in_place = false);
  void error(const std::string msg = detail::empty_msg, const std::string who = detail::error_msg, const Hash &params = {});
  void fatal(const std::string msg = detail::empty_msg, const std::string who = detail::fatal_msg, const Hash &params = {});
  void prompt(const std::string prompt_msg = detail::empty_msg);
};
