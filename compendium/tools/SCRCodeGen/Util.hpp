/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  =============================================================================*/
#ifndef UTIL_HPP
#define UTIL_HPP

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

#include "json/json.h"

namespace codegen {
namespace util {

class JsonValueValidator
{
public:
  template<size_t S>
  using ValidChoices = std::array<std::string, S>;

  // Throw if the Json::Value
  //  - data[name] doesn't exist
  //  - data[name] is empty
  //  - data[name] is not of the type specified by type.
  JsonValueValidator(const Json::Value& data,
                     std::string name,
                     Json::ValueType type)
    : jsonName(std::move(name))
    , msg("Invalid value for the name '" + jsonName + "'. Expected ")
  {
    jsonVal = data[jsonName.c_str()];
    if (Json::Value::nullRef == jsonVal) {
      std::string msg =
        "Mandatory name '" + jsonName + "' missing from the manifest";
      throw std::runtime_error(msg);
    }

    Validate(type);
  }

  // If data[name] exists, check if it is
  //  - a non-empty string, and
  //  - present in the choices array.
  // Otherwise, throw.
  // If data[name] doesn't exist, the first item in the choices array (default value)
  // will be returned by the operator() member function.
  template<std::size_t S>
  JsonValueValidator(const Json::Value& data,
                     std::string name,
                     const ValidChoices<S>& choices)
    : jsonName(std::move(name))
    , msg("Invalid value for the name '" + jsonName + "'. Expected ")
  {
    static_assert(S > 0, "Choices cannot be empty!");

    jsonVal = data[jsonName.c_str()];
    if (Json::Value::nullRef == jsonVal) {
      jsonVal = choices.front();
      return;
    }

    Validate(Json::ValueType::stringValue);
    auto value = jsonVal.asString();

    // The cast is to help the compiler resolve the correct overload.
    // Refer: https://stackoverflow.com/questions/7131858/stdtransform-and-toupper-no-matching-function/7131881
    std::transform(
      value.begin(), value.end(), value.begin(), [](unsigned char c) {
      return std::tolower(c); });

    // Return true if string value == string b in a case-insensitive way.
    auto isCaseInsensitiveEqual = [&value](const std::string& b) {
      auto isCaseInsensitiveEqualChar = [](auto a, auto b) {
        return std::tolower(a) == std::tolower(b);
      };

      if (value.length() == b.length()) {
        return std::equal(
          b.begin(), b.end(), value.begin(), isCaseInsensitiveEqualChar);
      } else {
        return false;
      }
    };

    if (std::none_of(choices.begin(), choices.end(), isCaseInsensitiveEqual)) {
      std::ostringstream stream;
      stream << "Invalid value '" + value + "' for the name '" + jsonName +
                  "'. ";
      stream << "The valid choices are : [";
      std::copy(std::begin(choices),
                std::end(choices) - 1,
                std::ostream_iterator<std::string>(stream, ", "));
      stream << choices.back() << "]";
      throw std::runtime_error(stream.str());
    }
  }

  // Return the Json value data[name]
  // (data and name are the Json data and name specified in the constructors)
  Json::Value operator()() const { return jsonVal; }

  // Return the string representation of the Json value data[name]
  // Throw if data[name] is not of Json String type
  // (data and name are the Json data and name specified in the constructors)
  std::string GetString() const
  {
    if (!jsonVal.isString()) {
      throw std::runtime_error("The JSON value for the name '" + jsonName +
                               "' must be of type string");
    }
    return jsonVal.asString();
  }

private:
  void Validate(Json::ValueType type)
  {
    switch (type) {
      case Json::ValueType::stringValue:
        if (!jsonVal.isString() || jsonVal.asString() == "") {
          msg.append("non-empty string");
          throw std::runtime_error(msg);
        }
        break;
      case Json::ValueType::arrayValue:
        if (!jsonVal.isArray() || !jsonVal.size()) {
          msg.append("non-empty array");
          throw std::runtime_error(msg);
        }
        break;
      case Json::ValueType::objectValue:
        if (!jsonVal.isObject() || !jsonVal.size()) {
          msg.append(
            "non-empty JSON object i.e. collection of name/value pairs");
          throw std::runtime_error(msg);
        }
        break;
      case Json::ValueType::intValue:
        if (!jsonVal.isInt()) {
          msg.append("int");
          throw std::runtime_error(msg);
        }
        break;
      case Json::ValueType::booleanValue:
        if (!jsonVal.isBool()) {
          msg.append("boolean");
          throw std::runtime_error(msg);
        }
        break;
      default:
        throw std::runtime_error("Unsupported type!");
    }
  }

  std::string jsonName;
  std::string msg;
  Json::Value jsonVal;
};

// Parse the manifest specified in the istream jsonStream.
// Return the root JSON value.
// Throw if the parsing isn't successful (such as presence of duplicate keys)
Json::Value ParseManifestOrThrow(std::istream& jsonStream);

// Write content to a file specified by the path filePath.
// Throw if the file can't be opened.
void WriteToFile(const std::string& filePath, const std::string& content);

namespace detail {
inline void replace(std::string& fmtstr, size_t index, const std::string& s)
{
  size_t pos = 0;
  std::string anchor = "{" + std::to_string(index) + "}";

  while (std::string::npos != pos) {
    pos = fmtstr.find(anchor, pos);
    if (std::string::npos != pos) {
      fmtstr.replace(pos, anchor.size(), s);
      pos += s.size();
    }
  }
}

inline void substitute(std::string&, size_t) {}

template<typename T, typename... Args>
void substitute(std::string& src, size_t index, T& t, Args&... args)
{
  replace(src, index, t);
  ++index;
  substitute(src, index, args...);
}
} // namespace detail

// Given a string "src" with placeholders of form "{0}", "{1}", "{2}"
// etc., substitute the arguments "args" into the string where the first
// argument of "args" is substituted in place of "{0}", the second
// argument of "args" is substituted in place of "{1}", and so on.
// For example, substitute("foo {0} {1}", "bar", "baz"); will return
// the string "foo bar baz"
template<typename... Args>
std::string Substitute(std::string src, Args... args)
{
  detail::substitute(src, 0, args...);
  return src;
}
} // namespace util
} // namespace codegen

#endif // UTIL_HPP
