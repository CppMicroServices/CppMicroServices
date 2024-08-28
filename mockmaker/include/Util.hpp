/*****************************************************************************
 *                                                                           *
 * Util.hpp:                                                                 *
 *                                                                           *
 * Miscellaneous utility classes, functions, and constants.                  *
 *                                                                           *
 *****************************************************************************/

#ifndef __UTIL_HPP
#define __UTIL_HPP

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

/**
 * Environment variable for checking whether to pretty-print parse info.
 */
#define INFO_ENV_VAR "MM_INFO"
#define SHOW_PRETTY_INFO (getenv(INFO_ENV_VAR) != nullptr)

/**
 * Terminal colors for pretty-printing.
 */
#define RESET "\x1b[0m"
#define RED "\x1b[31m"
#define YELLOW "\x1b[33m"
#define CYAN "\x1b[36m"

#define ERR RED "[ERR!] " RESET
#define WARN YELLOW "[WARN] " RESET
#define INFO CYAN "[INFO] " RESET

/**
 * Helper class for printing either to stdout or to a file.
 */
struct TeeDevice {
  bool use_stdout = true;
  ofstream of;

  TeeDevice() {}
  TeeDevice(const char *file) : use_stdout(false) {
    of = ofstream(file, ios::out);
  }

  template<typename T>
  TeeDevice &operator<<(const T &rhs) {
    if (use_stdout) cout << rhs;
    else of << rhs;
    return *this;
  }

  typedef ostream &(*stream_function)(ostream&);
  TeeDevice &operator<<(stream_function func) {
    if (use_stdout) func(cout);
    else func(of);
    return *this;
  }

  void close() {
    if (!use_stdout) of.close();
  }
};

/**
 * Helper function for joining a vector of strings into a
 * single string.
 */
template<typename T>
string vec_join(const vector<T> &strs, string sep) {
  if (strs.empty()) return "";

  string out;
  for (size_t i = 0; i < strs.size() - 1; ++i) {
    out.append(static_cast<string>(strs[i]));
    out.append(sep);
  }
  out += static_cast<string>(strs[strs.size() - 1]);
  return out;
}

/**
 * Helper function for converting a clang parse string to
 * a native C++ string.
 */
string cxstring_to_string(const CXString &s) {
  string result = clang_getCString(s);
  clang_disposeString(s);
  return result;
}

#endif /* __UTIL_HPP */
