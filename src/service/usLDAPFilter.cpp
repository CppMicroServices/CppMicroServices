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

#include "usLDAPFilter.h"
#include "usLDAPExpr_p.h"
#include "usServiceReferencePrivate.h"

#include <stdexcept>

US_BEGIN_NAMESPACE

class LDAPFilterData : public SharedData
{
public:

  LDAPFilterData() : ldapExpr()
  {}

  LDAPFilterData(const std::string& filter)
    : ldapExpr(filter)
  {}

  LDAPFilterData(const LDAPFilterData& other)
    : SharedData(other), ldapExpr(other.ldapExpr)
  {}

  LDAPExpr ldapExpr;
};

LDAPFilter::LDAPFilter()
  : d(0)
{
}

LDAPFilter::LDAPFilter(const std::string& filter)
  : d(0)
{
  try
  {
    d = new LDAPFilterData(filter);
  }
  catch (const std::exception& e)
  {
    throw std::invalid_argument(e.what());
  }
}

LDAPFilter::LDAPFilter(const LDAPFilter& other)
  : d(other.d)
{
}

LDAPFilter::~LDAPFilter()
{
}

LDAPFilter::operator bool() const
{
  return d.ConstData() != 0;
}

bool LDAPFilter::Match(const ServiceReference& reference) const
{
  return d->ldapExpr.Evaluate(reference.d->GetProperties(), true);
}

bool LDAPFilter::Match(const ServiceProperties& dictionary) const
{
  return d->ldapExpr.Evaluate(dictionary, false);
}

bool LDAPFilter::MatchCase(const ServiceProperties& dictionary) const
{
  return d->ldapExpr.Evaluate(dictionary, true);
}

std::string LDAPFilter::ToString() const
{
  return d->ldapExpr.ToString();
}

bool LDAPFilter::operator==(const LDAPFilter& other) const
{
  return d->ldapExpr.ToString() == other.d->ldapExpr.ToString();
}

LDAPFilter& LDAPFilter::operator=(const LDAPFilter& filter)
{
  d = filter.d;

  return *this;
}

US_END_NAMESPACE

US_USE_NAMESPACE

std::ostream& operator<<(std::ostream& os, const LDAPFilter& filter)
{
  return os << filter.ToString();
}
