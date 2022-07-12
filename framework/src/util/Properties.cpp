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

#include "PropsCheck.h"

US_MSVC_PUSH_DISABLE_WARNING(4996)

namespace cppmicroservices {

const Any Properties::emptyAny;

Properties::Properties(const AnyMap& p)
  : props(p)
{
  props_check::ValidateAnyMap(props);
}

Properties::Properties(AnyMap&& p)
  : props(std::move(p))
{
  props_check::ValidateAnyMap(props);
}

Properties::Properties(Properties&& o) noexcept
  : props(std::move(o.props))
{}

Properties& Properties::operator=(Properties&& o) noexcept
{
  props = std::move(o.props);
  return *this;
}

std::pair<Any, bool> Properties::Value_unlocked(const std::string& key,
                                                bool matchCase) const
{
  // Case-sensitive search first
  auto& uo = props.uo_m();
  auto csItr = uo.find(key);
  if (csItr != uo.end()) {
    return std::make_pair(csItr->second, true);
  }

  if (!matchCase) {
    // Case-insensitive search after
    auto& uoci = props.uoci_m();
    auto ciItr = uoci.find(key);
    if (ciItr != uoci.end()) {
      return std::make_pair(ciItr->second, true);
    }
  }

  return std::make_pair(emptyAny, false);
}

std::vector<std::string> Properties::Keys_unlocked() const
{
  std::vector<std::string> result{};
  for (auto& [key, _] : props) {
    US_UNUSED(_);
    result.push_back(key);
  }
  return result;
}

void Properties::Clear_unlocked()
{
  props.clear();
}

}

US_MSVC_POP_WARNING
