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

#include "Properties.h"

#include <limits>
#include <stdexcept>
#include <utility>

#include "PropsCheck.h"

US_MSVC_PUSH_DISABLE_WARNING(4996)

namespace cppmicroservices {

const Any Properties::emptyAny;

Properties::Properties(const AnyMap& p)
  : props(p)
{
  if (p.GetType() != AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS) {
    props_check::ValidateAnyMap(p);
  }

  if (p.GetType() != AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS) {
    for (const auto& kv_pair : p) {
      caseInsensitiveLookup[props_check::ToLower(kv_pair.first)] =
        kv_pair.first;
    }
  }
}

Properties::Properties(AnyMap&& p)
  : props(std::move(p))
{
  if (props.GetType() != AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS) {
    props_check::ValidateAnyMap(props);
  }

  if (props.GetType() != AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS) {
    for (const auto& kv_pair : props) {
      caseInsensitiveLookup[props_check::ToLower(kv_pair.first)] =
        kv_pair.first;
    }
  }
}

Properties::Properties(Properties&& o) noexcept
  : props(std::move(o.props))
{
  if (props.GetType() != AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS) {
    for (const auto& kv_pair : props) {
      caseInsensitiveLookup[props_check::ToLower(kv_pair.first)] =
        kv_pair.first;
    }
  }
}

Properties& Properties::operator=(Properties&& o) noexcept
{
  props = std::move(o.props);
  if (props.GetType() != AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS) {
    for (const auto& kv_pair : props) {
      caseInsensitiveLookup[props_check::ToLower(kv_pair.first)] =
        kv_pair.first;
    }
  }
  return *this;
}

std::pair<Any, bool> Properties::Value_unlocked(const std::string& key,
                                                bool matchCase) const
{
  if (props.GetType() == AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS) {
    if (auto itr = props.find(key); itr != props.end()) {
      if (!matchCase) {
        return std::make_pair(itr->second, true);
      } else if (matchCase && itr->first == key) {
        return std::make_pair(itr->second, true);
      } else {
        return std::make_pair(emptyAny, false);
      }
    } else {
      return std::make_pair(emptyAny, false);
    }
  } else { // map is not case-insensitive (std::map or std::unordered_map (NOT CI))
    // First do case-sensitive search...
    auto itr = props.find(key);
    if (itr != props.end()) {
      return std::make_pair(itr->second, true);
    }

    // If searching insensitively...
    if (!matchCase) {
      auto ciItr = caseInsensitiveLookup.find(key);
      if (ciItr != caseInsensitiveLookup.end()) {
        return std::make_pair(props.find(ciItr->second.data())->second, true);
      } else {
        return std::make_pair(emptyAny, false);
      }
    }

    return std::make_pair(emptyAny, false);
  }
}

std::vector<std::string> Properties::Keys_unlocked() const
{
  std::vector<std::string> result{};
  for (const auto& kv_pair : props) {
    result.push_back(kv_pair.first);
  }

  return result;
}

void Properties::Clear_unlocked()
{
  props.clear();
}
}

US_MSVC_POP_WARNING
