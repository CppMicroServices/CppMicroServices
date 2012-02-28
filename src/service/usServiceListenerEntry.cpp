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


#include "usServiceListenerEntry_p.h"

US_BEGIN_NAMESPACE

class ServiceListenerEntryData : public SharedData
{
public:

  ServiceListenerEntryData(Module* mc, const ServiceListenerEntry::ServiceListener& l, const std::string& filter)
    : mc(mc), listener(l), bRemoved(false), ldap()
  {
    if (!filter.empty())
    {
      ldap = LDAPExpr(filter);
    }
  }

  ~ServiceListenerEntryData()
  {

  }

  Module* const mc;
  ServiceListenerEntry::ServiceListener listener;
  bool bRemoved;
  LDAPExpr ldap;

  /**
   * The elements of "simple" filters are cached, for easy lookup.
   *
   * The grammar for simple filters is as follows:
   *
   * <pre>
   * Simple = '(' attr '=' value ')'
   *        | '(' '|' Simple+ ')'
   * </pre>
   * where <code>attr</code> is one of {@link Constants#OBJECTCLASS},
   * {@link Constants#SERVICE_ID} or {@link Constants#SERVICE_PID}, and
   * <code>value</code> must not contain a wildcard character.
   * <p>
   * The index of the vector determines which key the cache is for
   * (see {@link ServiceListenerState#hashedKeys}). For each key, there is
   * a vector pointing out the values which are accepted by this
   * ServiceListenerEntry's filter. This cache is maintained to make
   * it easy to remove this service listener.
   */
  LDAPExpr::LocalCache local_cache;

private:

  // purposely not implemented
  ServiceListenerEntryData(const ServiceListenerEntryData&);
  ServiceListenerEntryData& operator=(const ServiceListenerEntryData&);
};

ServiceListenerEntry::ServiceListenerEntry(const ServiceListenerEntry& other)
  : d(other.d)
{

}

ServiceListenerEntry::~ServiceListenerEntry()
{

}

ServiceListenerEntry& ServiceListenerEntry::operator=(const ServiceListenerEntry& other)
{
  d = other.d;
  return *this;
}

bool ServiceListenerEntry::operator==(const ServiceListenerEntry& other) const
{
  return ((d->mc == 0 || other.d->mc == 0) || d->mc == other.d->mc) &&
      ServiceListenerCompare()(d->listener, other.d->listener);
}

bool ServiceListenerEntry::operator<(const ServiceListenerEntry& other) const
{
  return d->mc < other.d->mc;
}

void ServiceListenerEntry::SetRemoved(bool removed) const
{
  d->bRemoved = removed;
}

bool ServiceListenerEntry::IsRemoved() const
{
  return d->bRemoved;
}

ServiceListenerEntry::ServiceListenerEntry(Module* mc, const ServiceListener& l, const std::string& filter)
  : d(new ServiceListenerEntryData(mc, l, filter))
{

}

Module* ServiceListenerEntry::GetModule() const
{
  return d->mc;
}

std::string ServiceListenerEntry::GetFilter() const
{
  return d->ldap.IsNull() ? std::string() : d->ldap.ToString();
}

LDAPExpr ServiceListenerEntry::GetLDAPExpr() const
{
  return d->ldap;
}

LDAPExpr::LocalCache& ServiceListenerEntry::GetLocalCache() const
{
  return d->local_cache;
}

void ServiceListenerEntry::CallDelegate(const ServiceEvent& event) const
{
  d->listener(event);
}

US_END_NAMESPACE
