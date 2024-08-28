/*****************************************************************************
 *                                                                           *
 * Parse.hpp:                                                                *
 *                                                                           *
 * Simple container classes for parse data, either of an in-progress parsing *
 * or the final mocks generated a completed parsing.                         *
 *                                                                           *
 *****************************************************************************/

#ifndef __PARSE_HPP
#define __PARSE_HPP

#include <set>
#include <map>

#include "Mocks.hpp"

using namespace std;

/**
 * Result of a successful parsing: Mocked classes
 * and their necessary includes (if applicable).
 */
struct ParseResult {
  bool failed = false;
  int n_methods = 0;
  set<string> includes;
  map<string, MockedClass> mocks;

  ParseResult() {}
  ParseResult(bool failed) : failed(failed) {}
};

/**
 * Variables used by an in-progress parsing, including
 * a pointer to the parse result used as output.
 */
struct ParseState {
  bool continue_descending = true;
  bool has_namespace = false;
  string class_name;
  ParseResult *out;
  vector<string> namespaces;

  ParseState() : out(nullptr) {}
  ParseState(ParseResult *out) : out(out) {}
};

#endif /* __PARSE_HPP */
