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

#include "cppmicroservices/Any.h"
#include "Utils.h"

#include <stdexcept>
#include <iomanip>

namespace cppmicroservices {

namespace detail {

void ThrowBadAnyCastException(const std::string& funcName,
                              const std::type_info& source,
                              const std::type_info& target)
{
  std::string msg("cppmicroservices::BadAnyCastException: ");
  std::string targetTypeName(GetDemangledName(target));
  std::string sourceTypeName(GetDemangledName(source));
  msg += funcName + ": Failed to convert from cppmicroservices::Any type " +
         sourceTypeName + " to target type " + targetTypeName;
  throw BadAnyCastException(msg);
}
}

std::ostream& indent_line(std::ostream& os, const uint8_t increment, const int32_t indent)
{
  if (increment > 0) {
    os << std::endl << std::setw(std::max(0,indent)) << ' ';
  }
  return os;
}

std::ostream& any_value_to_string(std::ostream& os, const Any& any)
{
  os << any.ToString();
  return os;
}

std::ostream& any_value_to_json(std::ostream& os, const Any& val, const uint8_t increment, const int32_t indent)
{
  os << val.ToJSON(increment, indent);
  return os;
}

std::ostream& any_value_to_json(std::ostream& o, const std::string& s, const uint8_t, const int32_t)
{
  o << '"';
  for (auto c = s.cbegin(); c != s.cend(); c++) {
    switch (*c) {
      case '"' : o << "\\\""; break;
      case '\\': o << "\\\\"; break;
      case '\b': o << "\\b";  break;
      case '\f': o << "\\f";  break;
      case '\n': o << "\\n";  break;
      case '\r': o << "\\r";  break;
      case '\t': o << "\\t";  break;
      default:
        if ('\x00' <= *c && *c <= '\x1f') {
          o << "\\u"
            << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(*c);
        } else {
          o << *c;
        }
    }
  }
  return o << '"';
}

std::ostream& any_value_to_json(std::ostream& os, bool val, const uint8_t, const int32_t)
{
  return os << std::boolalpha << val;
}

// The default constructor implementation needs to be in the implementation file, not the
// header in order to avoid this error:
// "default initialization of an object of const type 'const cppmicroservices::Any' without
// a user-provided default constructor"
Any::Any() = default;
    
std::string Any::ToString() const
{
  if (Empty()) {
    throw std::logic_error("empty any");
  }
  return _content->ToString();
}

std::string Any::ToStringNoExcept() const
{
  return Empty() ? std::string() : _content->ToString();
}
}
