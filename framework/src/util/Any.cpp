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

#include <stdexcept>

namespace cppmicroservices {

std::string any_value_to_string(const Any& any)
{
  return any.ToString();
}

std::ostream& any_value_to_string(std::ostream& os, const Any& any)
{
  os << any.ToString();
  return os;
}

std::string any_value_to_json(const Any& val)
{
  return val.ToJSON();
}

std::ostream& any_value_to_json(std::ostream& os, const Any& val)
{
  os << val.ToJSON();
  return os;
}

std::ostream& any_value_to_json(std::ostream& os, const std::string& val)
{
  return os << '"' << val << '"';
}

std::ostream& any_value_to_json(std::ostream& os, bool val)
{
  return os << (val ? "true" : "false");
}

std::string Any::ToString() const
{
  if (Empty())
  {
    throw std::logic_error("empty any");
  }
  return _content->ToString();
}

std::string Any::ToStringNoExcept() const
{
  return Empty() ? std::string() : _content->ToString();
}

}
