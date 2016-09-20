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

#include "cppmicroservices/AnyMap.h"

#include <stdexcept>

namespace cppmicroservices {

namespace detail {

const Any& AtCompoundKey(const std::vector<Any>& v, const AnyMap::key_type& key);

const Any& AtCompoundKey(const AnyMap& m, const AnyMap::key_type& key)
{
  auto pos = key.find(".");
  if (pos != AnyMap::key_type::npos)
  {
    auto head = key.substr(0, pos);
    auto tail = key.substr(pos + 1);

    auto& h = m.at(head);
    if (h.Type() == typeid(AnyMap))
    {
      return AtCompoundKey(ref_any_cast<AnyMap>(h), tail);
    }
    else if (h.Type() == typeid(std::vector<Any>))
    {
      return AtCompoundKey(ref_any_cast<std::vector<Any>>(h), tail);
    }
    throw std::invalid_argument("Unsupported Any type at '" + head + "' for dotted get");
  }
  else
  {
    return m.at(key);
  }
}

const Any& AtCompoundKey(const std::vector<Any>& v, const AnyMap::key_type& key)
{
  auto pos = key.find(".");
  if (pos != AnyMap::key_type::npos)
  {
    auto head = key.substr(0, pos);
    auto tail = key.substr(pos + 1);

    const int index = std::stoi(head);
    auto& h = v.at(index < 0 ? v.size() + index : index);

    if (h.Type() == typeid(AnyMap))
    {
      return AtCompoundKey(ref_any_cast<AnyMap>(h), tail);
    }
    else if (h.Type() == typeid(std::vector<Any>))
    {
      return AtCompoundKey(ref_any_cast<std::vector<Any>>(h), tail);
    }
    throw std::invalid_argument("Unsupported Any type at '" + head + "' for dotted get");
  }
  else
  {
    const int index = std::stoi(key);
    return v.at(index < 0 ? v.size() + index : index);
  }
}

}

AnyMap::mapped_type& AnyMap::AtCompoundKey(const key_type& key)
{
  return const_cast<mapped_type&>(static_cast<const AnyMap*>(this)->AtCompoundKey(key));
}

const AnyMap::mapped_type& AnyMap::AtCompoundKey(const key_type& key) const
{
  return detail::AtCompoundKey(*this, key);
}


template<>
std::ostream& any_value_to_string(std::ostream& os, const AnyMap& m)
{
  os << "{";
  typedef any_map::const_iterator Iterator;
  Iterator i1 = m.begin();
  const Iterator begin = i1;
  const Iterator end = m.end();
  for ( ; i1 != end; ++i1)
  {
    if (i1 == begin) os << i1->first << " : " << i1->second.ToString();
    else os << ", " << i1->first << " : " << i1->second.ToString();
  }
  os << "}";
  return os;
}

}
