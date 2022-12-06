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

#include "cppmicroservices/GlobalConfig.h"

US_MSVC_PUSH_DISABLE_WARNING(
  4180) // qualifier applied to function type has no meaning; ignored

#include "cppmicroservices/FilteringStrategy.h"
#include "ServiceListenerEntry.h"
#include "ServiceListenerHookPrivate.h"
#include <cassert>

namespace cppmicroservices {

class ServiceListenerEntryData : public ServiceListenerHook::ListenerInfoData
{
public:
  ServiceListenerEntryData(const ServiceListenerEntryData&) = delete;
  ServiceListenerEntryData& operator=(const ServiceListenerEntryData&) = delete;

  ServiceListenerEntryData(const std::shared_ptr<BundleContextPrivate>& context,
                           const ServiceListener& l,
                           void* data,
                           ListenerTokenId tokenId,
                           const std::string& filter_string)
    : ServiceListenerHook::ListenerInfoData(context, l, data, tokenId, filter_string)
    , filter(filter_string)
    , local_cache()
    , hashValue(0)
  {
  }

  ~ServiceListenerEntryData() override = default;

  FilteringStrategy filter;
  
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
};

ServiceListenerEntry::ServiceListenerEntry() = default;

ServiceListenerEntry::ServiceListenerEntry(const ServiceListenerEntry&) =
  default;

ServiceListenerEntry::ServiceListenerEntry(
  const ServiceListenerHook::ListenerInfo& info)
  : ServiceListenerHook::ListenerInfo(info)
{
  assert(info.d);
}

ServiceListenerEntry::~ServiceListenerEntry() = default;

ServiceListenerEntry& ServiceListenerEntry::operator=(
  const ServiceListenerEntry& other)
{
  d = other.d;
  return *this;
}

void ServiceListenerEntry::SetRemoved(bool removed) const
{
  d->bRemoved = removed;
}

ServiceListenerEntry::ServiceListenerEntry(
  const std::shared_ptr<BundleContextPrivate>& context,
  const ServiceListener& l,
  void* data,
  ListenerTokenId tokenId,
  const std::string& filter_string)
  : ServiceListenerHook::ListenerInfo(
      new ServiceListenerEntryData(context, l, data, tokenId, filter_string))
{}

LDAPExpr::LocalCache& ServiceListenerEntry::GetLocalCache() const
{
  return static_cast<ServiceListenerEntryData*>(d.get())->local_cache;
}

void ServiceListenerEntry::CallDelegate(const ServiceEvent& event) const
{
  d->listener(event);
}

bool ServiceListenerEntry::operator==(const ServiceListenerEntry& other) const
{
  return (d->tokenId == other.d->tokenId) &&
         ((d->context == nullptr || other.d->context == nullptr) ||
          d->context == other.d->context);
}

bool ServiceListenerEntry::operator<(const ServiceListenerEntry& other) const
{
  return d->tokenId < other.d->tokenId;
}

bool ServiceListenerEntry::Contains(
  const std::shared_ptr<BundleContextPrivate>& context,
  const ServiceListener& listener,
  void* data) const
{
  return ((d->context == context)
          && (d->data == data)
          && (d->listener.target<void(const ServiceEvent&)>() ==
              listener.target<void(const ServiceEvent&)>()));
}

ListenerTokenId ServiceListenerEntry::Id() const
{
  return d->tokenId;
}

std::size_t ServiceListenerEntry::Hash() const
{
  using std::hash;

  if (static_cast<ServiceListenerEntryData*>(d.get())->hashValue == 0) {
    static_cast<ServiceListenerEntryData*>(d.get())->hashValue =
      (hash<BundleContextPrivate*>()(d->context.get()) >> 1) ^
      ((hash<ListenerTokenId>()(d->tokenId) << 1) << 1);
  }

  return static_cast<ServiceListenerEntryData*>(d.get())->hashValue;
}

bool ServiceListenerEntry::MatchFilter(const AnyMap& props) const
{
  return static_cast<ServiceListenerEntryData*>(d.get())->filter.Match(props);
}

bool ServiceListenerEntry::IsComplicatedFilter() const
{
  return static_cast<ServiceListenerEntryData*>(d.get())->filter.IsComplicated();
}

bool ServiceListenerEntry::AddToSimpleCache(const StringList& keywords, LocalCache& cache) const
{
  return static_cast<ServiceListenerEntryData*>(d.get())->filter.AddToSimpleCache(keywords, cache);
}

}

US_MSVC_POP_WARNING
