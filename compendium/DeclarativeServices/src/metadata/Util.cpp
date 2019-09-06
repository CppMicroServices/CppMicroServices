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

#include "Util.hpp"

namespace cppmicroservices { namespace scrimpl { namespace util {

// @brief Throws if the container T is of type @c Any or @c string or @c AnyMap and it's empty
template <>
void ThrowIfEmpty(const std::vector<cppmicroservices::Any>& value, const std::string& key)
{
  ThrowIfEmptyHelper(value, key);
}
  
template <>
void ThrowIfEmpty(const std::string& value, const std::string& key)
{
  ThrowIfEmptyHelper(value, key);
}
  
template <>
void ThrowIfEmpty(const cppmicroservices::AnyMap& value, const std::string& key)
{
  ThrowIfEmptyHelper(value, key);
}

template <>
void ThrowIfValueAbsentInChoices(const std::string& inValue, const std::vector<std::string>& choices)
{
  std::string value = inValue;
#if _WIN32
  std::transform(value.begin(), value.end(), value.begin(), tolower);
#else
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(c))); });
#endif
  if (!choices.empty() &&
      std::find(choices.begin(), choices.end(), value) == choices.end())
  {
    std::ostringstream stream;
    stream << "Invalid value '" + value + "'. ";
    stream << "The valid choices are : [";
    for (auto c = std::begin(choices); c < std::end(choices) - 1; ++c)
    {
      stream << *c << ", ";
    }
    stream << choices.back() << "].";
    throw std::out_of_range(stream.str());
  }
}

}}}
