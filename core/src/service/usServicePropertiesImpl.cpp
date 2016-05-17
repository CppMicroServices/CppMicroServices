/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

#include "usServicePropertiesImpl_p.h"

#include <limits>
#include <stdexcept>
#ifdef US_PLATFORM_WINDOWS
#include <string.h>
#define ci_compare strnicmp
#else
#include <strings.h>
#define ci_compare strncasecmp
#endif

namespace us {

Any ServicePropertiesImpl::emptyAny;

ServicePropertiesImpl::ServicePropertiesImpl(const ServiceProperties& p)
{
  if (p.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
  {
    throw std::runtime_error("ServiceProperties object contains too many keys");
  }

  keys.reserve(p.size());
  values.reserve(p.size());

  for (auto& iter : p)
  {
    if (Find_unlocked(iter.first) > -1)
    {
      std::string msg = "ServiceProperties object contains case variants of the key: ";
      msg += iter.first;
      throw std::runtime_error(msg.c_str());
    }
    keys.push_back(iter.first);
    values.push_back(iter.second);
  }
}


ServicePropertiesImpl::ServicePropertiesImpl(ServicePropertiesImpl&& o)
  : keys(std::move(o.keys))
  , values(std::move(o.values))
{
}

ServicePropertiesImpl& ServicePropertiesImpl::operator=(ServicePropertiesImpl&& o)
{
  keys = std::move(o.keys);
  values = std::move(o.values);
  return *this;
}

Any ServicePropertiesImpl::Value_unlocked(const std::string& key) const
{
  int i = Find_unlocked(key);
  if (i < 0) return emptyAny;
  return values[i];
}

Any ServicePropertiesImpl::Value_unlocked(int index) const
{
  if (index < 0 || static_cast<std::size_t>(index) >= values.size())
    return emptyAny;
  return values[static_cast<std::size_t>(index)];
}

int ServicePropertiesImpl::Find_unlocked(const std::string& key) const
{
  for (std::size_t i = 0; i < keys.size(); ++i)
  {
    if (ci_compare(key.c_str(), keys[i].c_str(), key.size()) == 0)
    {
      return static_cast<int>(i);
    }
  }
  return -1;
}

int ServicePropertiesImpl::FindCaseSensitive_unlocked(const std::string& key) const
{
  for (std::size_t i = 0; i < keys.size(); ++i)
  {
    if (key == keys[i])
    {
      return static_cast<int>(i);
    }
  }
  return -1;
}

std::vector<std::string> ServicePropertiesImpl::Keys_unlocked() const
{
  return keys;
}

void ServicePropertiesImpl::Clear_unlocked()
{
  keys.clear();
  values.clear();
}

}
