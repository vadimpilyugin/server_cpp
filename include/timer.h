#include <chrono>
#include <string>
#include <thread>  // std::this_thread::sleep_for
#pragma once

#include <iomanip> // setprecision
#include <sstream> // stringstream
using namespace std;

struct Timer {
  std::chrono::time_point<std::chrono::system_clock> start_p;
  std::chrono::time_point<std::chrono::system_clock> finish_p;
  double accumulator;
  
  Timer (): accumulator(0.0) {}
  
  void start () {
    start_p = chrono::system_clock::now();
  }
  
  void pause () {
    finish_p = chrono::system_clock::now();
    std::chrono::duration<double> diff = finish_p-start_p;
    accumulator += diff.count ();
  }
  
  double stop () {
    pause ();
    double tmp = accumulator;
    accumulator = 0;
    return tmp;
  }
  
  double get () {
    pause ();
    double current_time = accumulator;
    start ();
    return current_time;
  }

  static string hr_speed (size_t size_in_bytes, double elapsed_time) {
    const size_t B_IN_KB = 1024;
    const size_t B_IN_MB = 1024*1024;
    const double eps = 1e-3;

    if (elapsed_time <= eps)
      return "n/a B/s";
      
    string s;
    
      
    // вычисляем скорость в байтах в секунду
    double speed = size_in_bytes / elapsed_time;
    
    // переводим байты в мегабайты, если нужно
    if (speed >= B_IN_MB) {
      speed = speed / B_IN_MB;
      stringstream stream;
      stream << fixed << setprecision(2) << speed;
      s = stream.str() + " МБ/с";
    }
    else if (speed >= B_IN_KB) {
      speed = speed / B_IN_KB;
      stringstream stream;
      stream << fixed << setprecision(2) << speed;
      s = stream.str() + " КБ/с";
    }
    else {
      stringstream stream;
      stream << fixed << setprecision(0) << speed;
      s = stream.str() + " Б/с";
    }
    return s;
  }
};