/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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

US_BEGIN_NAMESPACE

Any ServicePropertiesImpl::emptyAny;

ServicePropertiesImpl::ServicePropertiesImpl(const ServiceProperties& p)
{
  if (p.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
  {
    throw std::runtime_error("ServiceProperties object contains too many keys");
  }

  keys.reserve(p.size());
  values.reserve(p.size());

  for (ServiceProperties::const_iterator iter = p.begin();
       iter != p.end(); ++iter)
  {
    if (Find(iter->first) > -1)
    {
      std::string msg = "ServiceProperties object contains case variants of the key: ";
      msg += iter->first;
      throw std::runtime_error(msg.c_str());
    }
    keys.push_back(iter->first);
    values.push_back(iter->second);
  }
}

const Any& ServicePropertiesImpl::Value(const std::string& key) const
{
  int i = Find(key);
  if (i < 0) return emptyAny;
  return values[i];
}

const Any& ServicePropertiesImpl::Value(int index) const
{
  if (index < 0 || static_cast<std::size_t>(index) >= values.size())
    return emptyAny;
  return values[static_cast<std::size_t>(index)];
}

int ServicePropertiesImpl::Find(const std::string& key) const
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

int ServicePropertiesImpl::FindCaseSensitive(const std::string& key) const
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

const std::vector<std::string>& ServicePropertiesImpl::Keys() const
{
  return keys;
}

US_END_NAMESPACE
