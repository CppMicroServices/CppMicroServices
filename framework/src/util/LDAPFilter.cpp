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

#include "cppmicroservices/LDAPFilter.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/ServiceReference.h"

#include "LDAPExpr.h"
#include "Properties.h"
#include "ServiceReferenceBasePrivate.h"

#include <stdexcept>

#ifdef US_PLATFORM_WINDOWS
#  include <string.h>
#  define ci_compare strnicmp
#else
#  include <strings.h>
#  define ci_compare strncasecmp
#endif

namespace cppmicroservices {

class LDAPFilterData
{
public:
  LDAPFilterData()
    : ldapExpr()
  {}

  LDAPFilterData(const std::string& filter)
    : ldapExpr(filter)
  {}

  LDAPFilterData(const LDAPFilterData&) = default;

  LDAPExpr ldapExpr;
};

LDAPFilter::LDAPFilter()
  : d(nullptr)
{}

LDAPFilter::LDAPFilter(const std::string& filter)
  : d(nullptr)
{
  try {
    d = std::make_shared<LDAPFilterData>(filter);
  } catch (const std::exception& e) {
    throw std::invalid_argument(e.what());
  }
}

LDAPFilter::LDAPFilter(const LDAPFilter&) = default;

LDAPFilter::~LDAPFilter() = default;

LDAPFilter::operator bool() const
{
  return d != nullptr;
}

bool LDAPFilter::Match(const ServiceReferenceBase& reference) const
{
  return ((d) ? d->ldapExpr.Evaluate(reference.d.load()->GetProperties(), false)
              : false);
}

bool LDAPFilter::Match(const Bundle& bundle) const
{
  auto& headers = bundle.GetHeaders();
  std::vector<std::string> keys;
  for (auto& [key, _] : headers) {
    keys.emplace_back(key);
  }

  if (headers.size() > 0) {
    for (uint32_t i = 0; i < keys.size() - 1; ++i) {
      for (uint32_t j = i + 1; j < keys.size(); ++j) {
        if (keys[i].size() == keys[j].size() &&
            ci_compare(keys[i].c_str(), keys[j].c_str(), keys[i].size()) == 0) {
          std::string msg("Properties contain case variants of the key: ");
          msg += keys[i];
          throw std::runtime_error(msg.c_str());
        }
      }
    }
  }

  return ((d) ? d->ldapExpr.Evaluate(bundle.GetHeaders(), false) : false);
}

bool LDAPFilter::Match(const AnyMap& dictionary) const
{
  auto& headers = dictionary;
  std::vector<std::string> keys;
  for (auto& [key, _] : headers) {
    keys.emplace_back(key);
  }

  if (headers.size() > 0) {
    for (uint32_t i = 0; i < keys.size() - 1; ++i) {
      for (uint32_t j = i + 1; j < keys.size(); ++j) {
        if (keys[i].size() == keys[j].size() &&
            ci_compare(keys[i].c_str(), keys[j].c_str(), keys[i].size()) == 0) {
          std::string msg("Properties contain case variants of the key: ");
          msg += keys[i];
          throw std::runtime_error(msg.c_str());
        }
      }
    }
  }

  return ((d) ? d->ldapExpr.Evaluate(dictionary, false) : false);
}

bool LDAPFilter::MatchCase(const AnyMap& dictionary) const
{
  auto& headers = dictionary;
  std::vector<std::string> keys;
  for (auto& [key, _] : headers) {
    keys.emplace_back(key);
  }

  if (headers.size() > 0) {
    for (uint32_t i = 0; i < keys.size() - 1; ++i) {
      for (uint32_t j = i + 1; j < keys.size(); ++j) {
        if (keys[i].size() == keys[j].size() &&
            ci_compare(keys[i].c_str(), keys[j].c_str(), keys[i].size()) == 0) {
          std::string msg("Properties contain case variants of the key: ");
          msg += keys[i];
          throw std::runtime_error(msg.c_str());
        }
      }
    }
  }

  return ((d) ? d->ldapExpr.Evaluate(dictionary, true) : false);
}

std::string LDAPFilter::ToString() const
{
  return ((d) ? d->ldapExpr.ToString() : std::string());
}

bool LDAPFilter::operator==(const LDAPFilter& other) const
{
  return (this->ToString() == other.ToString());
}

LDAPFilter& LDAPFilter::operator=(const LDAPFilter& filter) = default;

std::ostream& operator<<(std::ostream& os, const LDAPFilter& filter)
{
  return os << filter.ToString();
}
}
