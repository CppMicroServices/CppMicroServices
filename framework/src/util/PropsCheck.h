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

#ifndef PROPSCHECK_H
#define PROPSCHECK_H

#ifdef US_PLATFORM_WINDOWS
#  include <string.h>
#  define ci_compare strnicmp
#else
#  include <strings.h>
#  define ci_compare strncasecmp
#endif

#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "cppmicroservices/LDAPFilter.h"

namespace cppmicroservices::props_check {
inline std::pair<bool, std::string_view> IsInvalid(
  const cppmicroservices::AnyMap& am)
{
  std::vector<std::string_view> keys(am.size());
  uint32_t currIndex = 0;
  for (auto& [key, _] : am) {
    keys[currIndex++] = key;
  }

  if (am.size() > 1) {
    for (uint32_t i = 0; i < keys.size() - 1; ++i) {
      for (uint32_t j = i + 1; j < keys.size(); ++j) {
        if (keys[i].size() == keys[j].size() &&
            ci_compare(keys[i].data(), keys[j].data(), keys[i].size()) == 0) {
          return std::make_pair(true, keys[i]);
        }
      }
    }
  }

  return std::make_pair(false, "");
}
}

#endif
