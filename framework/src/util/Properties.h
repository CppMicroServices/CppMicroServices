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

#ifndef CPPMICROSERVICES_PROPERTIES_H
#define CPPMICROSERVICES_PROPERTIES_H

#include "cppmicroservices/Any.h"
#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/detail/Threads.h"

#include <string>
#include <vector>

namespace cppmicroservices {

class Properties : public detail::MultiThreaded<>
{

public:
  explicit Properties(const AnyMap& props);
  explicit Properties(AnyMap&& props);

  Properties(Properties&& o) noexcept;
  Properties& operator=(Properties&& o) noexcept;

  std::pair<Any, bool> Value_unlocked(const std::string& key,
                                      bool matchCase = false) const;

  std::vector<std::string> Keys_unlocked() const;

  void Clear_unlocked();

private:
  AnyMap props;
  std::unordered_map<std::string,
                     std::string,
                     detail::any_map_cihash,
                     detail::any_map_ciequal>
    caseInsensitiveLookup;

  static const Any emptyAny;
};

class PropertiesHandle
{
public:
  PropertiesHandle(const Properties& props, bool lock)
    : props(props)
    , l(lock ? props.Lock() : Properties::UniqueLock())
  {}

  PropertiesHandle(PropertiesHandle&& o) noexcept
    : props(o.props)
    , l(std::move(o.l))
  {}

  const Properties* operator->() const { return &props; }

private:
  const Properties& props;
  Properties::UniqueLock l;
};
}

#endif // CPPMICROSERVICES_PROPERTIES_H
