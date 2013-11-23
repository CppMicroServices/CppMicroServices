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
#include "usServiceListenerHook_p.h"

#include <cassert>

US_BEGIN_NAMESPACE

class ServiceListenerEntryData : public ServiceListenerHook::ListenerInfoData
{
public:

  ServiceListenerEntryData(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& l,
                           void* data, const std::string& filter)
    : ServiceListenerHook::ListenerInfoData(mc, l, data, filter)
    , ldap()
    , hashValue(0)
  {
    if (!filter.empty())
    {
      ldap = LDAPExpr(filter);
    }
  }

  ~ServiceListenerEntryData()
  {
  }

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
   * where <code>attr</code> is one of Constants#OBJECTCLASS,
   * Constants#SERVICE_ID or Constants#SERVICE_PID, and
   * <code>value</code> must not contain a wildcard character.
   * <p>
   * The index of the vector determines which key the cache is for
   * (see ServiceListenerState#hashedKeys). For each key, there is
   * a vector pointing out the values which are accepted by this
   * ServiceListenerEntry's filter. This cache is maintained to make
   * it easy to remove this service listener.
   */
  LDAPExpr::LocalCache local_cache;

  std::size_t hashValue;

private:

  // purposely not implemented
  ServiceListenerEntryData(const ServiceListenerEntryData&);
  ServiceListenerEntryData& operator=(const ServiceListenerEntryData&);
};

ServiceListenerEntry::ServiceListenerEntry(const ServiceListenerEntry& other)
  : ServiceListenerHook::ListenerInfo(other)
{
}

ServiceListenerEntry::ServiceListenerEntry(const ServiceListenerHook::ListenerInfo& info)
  : ServiceListenerHook::ListenerInfo(info)
{
  assert(info.d);
}

ServiceListenerEntry::~ServiceListenerEntry()
{
}

ServiceListenerEntry& ServiceListenerEntry::operator=(const ServiceListenerEntry& other)
{
  d = other.d;
  return *this;
}

void ServiceListenerEntry::SetRemoved(bool removed) const
{
  d->bRemoved = removed;
}

ServiceListenerEntry::ServiceListenerEntry(ModuleContext* mc, const ServiceListener& l,
                                           void* data, const std::string& filter)
  : ServiceListenerHook::ListenerInfo(new ServiceListenerEntryData(mc, l, data, filter))
{
}

const LDAPExpr& ServiceListenerEntry::GetLDAPExpr() const
{
  return static_cast<ServiceListenerEntryData*>(d.Data())->ldap;
}

LDAPExpr::LocalCache& ServiceListenerEntry::GetLocalCache() const
{
  return static_cast<ServiceListenerEntryData*>(d.Data())->local_cache;
}

void ServiceListenerEntry::CallDelegate(const ServiceEvent& event) const
{
  d->listener(event);
}

bool ServiceListenerEntry::operator==(const ServiceListenerEntry& other) const
{
  return ((d->mc == NULL || other.d->mc == NULL) || d->mc == other.d->mc) &&
      (d->data == other.d->data) && ServiceListenerCompare()(d->listener, other.d->listener);
}

std::size_t ServiceListenerEntry::Hash() const
{
  using namespace US_HASH_FUNCTION_NAMESPACE;

  if (static_cast<ServiceListenerEntryData*>(d.Data())->hashValue == 0)
  {
    static_cast<ServiceListenerEntryData*>(d.Data())->hashValue =
        ((US_HASH_FUNCTION(ModuleContext*, d->mc) ^ (US_HASH_FUNCTION(void*, d->data) << 1)) >> 1) ^
        (US_HASH_FUNCTION(US_SERVICE_LISTENER_FUNCTOR, d->listener) << 1);
  }
  return static_cast<ServiceListenerEntryData*>(d.Data())->hashValue;
}

US_END_NAMESPACE
