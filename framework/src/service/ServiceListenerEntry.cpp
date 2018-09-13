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

#include "ServiceListenerEntry.h"

#include "ServiceListenerHookPrivate.h"

#include <cassert>

namespace cppmicroservices {

struct ServiceListenerCompare
  : std::binary_function<ServiceListener, ServiceListener, bool>
{
  bool operator()(const ServiceListener& f1, const ServiceListener& f2) const
  {
    return f1.target<void(const ServiceEvent&)>() ==
           f2.target<void(const ServiceEvent&)>();
  }
};

class ServiceListenerEntryData : public ServiceListenerHook::ListenerInfoData
{
public:
  ServiceListenerEntryData(const ServiceListenerEntryData&) = delete;
  ServiceListenerEntryData& operator=(const ServiceListenerEntryData&) = delete;

  ServiceListenerEntryData(const std::shared_ptr<BundleContextPrivate>& context,
                           const ServiceListener& l,
                           void* data,
                           ListenerTokenId tokenId,
                           const std::string& filter)
    : ServiceListenerHook::ListenerInfoData(context, l, data, tokenId, filter)
    , ldap()
    , hashValue(0)
  {
    if (!filter.empty()) {
      ldap = LDAPExpr(filter);
    }
  }

  ~ServiceListenerEntryData() {}

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
};

ServiceListenerEntry::ServiceListenerEntry() {}

ServiceListenerEntry::ServiceListenerEntry(const ServiceListenerEntry& other)
  : ServiceListenerHook::ListenerInfo(other)
{}

ServiceListenerEntry::ServiceListenerEntry(
  const ServiceListenerHook::ListenerInfo& info)
  : ServiceListenerHook::ListenerInfo(info)
{
  assert(info.data_ptr);
}

ServiceListenerEntry::~ServiceListenerEntry() {}

ServiceListenerEntry& ServiceListenerEntry::operator=(
  const ServiceListenerEntry& other)
{
  data_ptr = other.data_ptr;
  return *this;
}

void ServiceListenerEntry::SetRemoved(bool removed) const
{
  data_ptr->bRemoved = removed;
}

ServiceListenerEntry::ServiceListenerEntry(
  const std::shared_ptr<BundleContextPrivate>& context,
  const ServiceListener& l,
  void* data,
  ListenerTokenId tokenId,
  const std::string& filter)
  : ServiceListenerHook::ListenerInfo(
      new ServiceListenerEntryData(context, l, data, tokenId, filter))
{}

const LDAPExpr& ServiceListenerEntry::GetLDAPExpr() const
{
  return static_cast<ServiceListenerEntryData*>(data_ptr.get())->ldap;
}

LDAPExpr::LocalCache& ServiceListenerEntry::GetLocalCache() const
{
  return static_cast<ServiceListenerEntryData*>(data_ptr.get())->local_cache;
}

void ServiceListenerEntry::CallDelegate(const ServiceEvent& event) const
{
  data_ptr->listener(event);
}

bool ServiceListenerEntry::operator==(const ServiceListenerEntry& other) const
{
  return ((data_ptr->context == nullptr || other.data_ptr->context == nullptr) ||
          data_ptr->context == other.data_ptr->context) &&
         (data_ptr->data == other.data_ptr->data) && (data_ptr->tokenId == other.data_ptr->tokenId) &&
         ServiceListenerCompare()(data_ptr->listener, other.data_ptr->listener);
}

bool ServiceListenerEntry::Contains(
  const std::shared_ptr<BundleContextPrivate>& context,
  ListenerTokenId tokenId) const
{
  return (data_ptr->context == context) && (data_ptr->tokenId == tokenId);
}

bool ServiceListenerEntry::Contains(
  const std::shared_ptr<BundleContextPrivate>& context,
  const ServiceListener& listener,
  void* data) const
{
  return (data_ptr->context == context) && (data_ptr->data == data) &&
         ServiceListenerCompare()(data_ptr->listener, listener);
}

ListenerTokenId ServiceListenerEntry::Id() const
{
  return data_ptr->tokenId;
}

std::size_t ServiceListenerEntry::Hash() const
{
  using namespace std;

  if (static_cast<ServiceListenerEntryData*>(data_ptr.get())->hashValue == 0) {
    static_cast<ServiceListenerEntryData*>(data_ptr.get())->hashValue =
      ((hash<BundleContextPrivate*>()(data_ptr->context.get()) ^
        (hash<void*>()(data_ptr->data) << 1)) >>
       1) ^
      ((hash<ServiceListener>()(data_ptr->listener)) ^
       (hash<ListenerTokenId>()(data_ptr->tokenId) << 1) << 1);
  }
  return static_cast<ServiceListenerEntryData*>(data_ptr.get())->hashValue;
}
}

US_MSVC_POP_WARNING
