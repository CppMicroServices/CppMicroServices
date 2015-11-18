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

#include "usServiceReferenceBase.h"
#include "usServiceReferenceBasePrivate.h"
#include "usServiceRegistrationBasePrivate.h"

#include "usBundle.h"
#include "usBundlePrivate.h"

#include <cassert>

namespace us {

ServiceReferenceBase::ServiceReferenceBase()
  : d(new ServiceReferenceBasePrivate(0))
{

}

ServiceReferenceBase::ServiceReferenceBase(const ServiceReferenceBase& ref)
  : d(ref.d)
{
  ++d->ref;
}

ServiceReferenceBase::ServiceReferenceBase(ServiceRegistrationBasePrivate* reg)
  : d(new ServiceReferenceBasePrivate(reg))
{
}

void ServiceReferenceBase::SetInterfaceId(const std::string& interfaceId)
{
  if (d->ref > 1)
  {
    // detach
    --d->ref;
    d = new ServiceReferenceBasePrivate(d->registration);
  }
  d->interfaceId = interfaceId;
}

ServiceReferenceBase::operator bool_type() const
{
  return GetBundle() != nullptr ? &ServiceReferenceBase::d : NULL;
}

ServiceReferenceBase& ServiceReferenceBase::operator=(std::nullptr_t)
{
  if (!--d->ref)
    delete d;
  d = new ServiceReferenceBasePrivate(0);
  return *this;
}

ServiceReferenceBase::~ServiceReferenceBase()
{
  if (!--d->ref)
    delete d;
}

Any ServiceReferenceBase::GetProperty(const std::string& key) const
{
  typedef decltype(d->registration->propsLock) T; // gcc 4.6 workaround
  T::Lock l(d->registration->propsLock);

  return d->registration->properties.Value(key);
}

void ServiceReferenceBase::GetPropertyKeys(std::vector<std::string>& keys) const
{
  typedef decltype(d->registration->propsLock) T; // gcc 4.6 workaround
  T::Lock l(d->registration->propsLock);

  const std::vector<std::string>& ks = d->registration->properties.Keys();
  keys.assign(ks.begin(), ks.end());
}

std::shared_ptr<Bundle> ServiceReferenceBase::GetBundle() const
{
  if (d->registration == nullptr || d->registration->bundle == nullptr)
  {
    return nullptr;
  }

  return d->registration->bundle->q;
}

void ServiceReferenceBase::GetUsingBundles(std::vector<std::shared_ptr<Bundle>>& bundles) const
{
  typedef decltype(d->registration->propsLock) T; // gcc 4.6 workaround
  T::Lock l(d->registration->propsLock);

  ServiceRegistrationBasePrivate::BundleToRefsMap::const_iterator end = d->registration->dependents.end();
  for (ServiceRegistrationBasePrivate::BundleToRefsMap::const_iterator iter = d->registration->dependents.begin();
       iter != end; ++iter)
  {
    bundles.push_back(iter->first);
  }
}

bool ServiceReferenceBase::operator<(const ServiceReferenceBase& reference) const
{
  if (!(*this))
  {
    return true;
  }

  if (!reference)
  {
    return false;
  }

  const Any anyR1 = GetProperty(ServiceConstants::SERVICE_RANKING());
  const Any anyR2 = reference.GetProperty(ServiceConstants::SERVICE_RANKING());
  assert(anyR1.Empty() || anyR1.Type() == typeid(int));
  assert(anyR2.Empty() || anyR2.Type() == typeid(int));
  const int r1 = anyR1.Empty() ? 0 : *any_cast<int>(&anyR1);
  const int r2 = anyR2.Empty() ? 0 : *any_cast<int>(&anyR2);

  if (r1 != r2)
  {
    // use ranking if ranking differs
    return r1 < r2;
  }
  else
  {
    const Any anyId1 = GetProperty(ServiceConstants::SERVICE_ID());
    const Any anyId2 = reference.GetProperty(ServiceConstants::SERVICE_ID());
    assert(anyId1.Type() == typeid(long int));
    assert(anyId2.Type() == typeid(long int));
    const long int id1 = *any_cast<long int>(&anyId1);
    const long int id2 = *any_cast<long int>(&anyId2);

    // otherwise compare using IDs,
    // is less than if it has a higher ID.
    return id2 < id1;
  }
}

bool ServiceReferenceBase::operator==(const ServiceReferenceBase& reference) const
{
  return d->registration == reference.d->registration;
}

ServiceReferenceBase& ServiceReferenceBase::operator=(const ServiceReferenceBase& reference)
{
  ServiceReferenceBasePrivate* curr_d = d;
  d = reference.d;
  ++d->ref;

  if (!--curr_d->ref)
    delete curr_d;

  return *this;
}

bool ServiceReferenceBase::IsConvertibleTo(const std::string& interfaceId) const
{
  return d->IsConvertibleTo(interfaceId);
}

std::string ServiceReferenceBase::GetInterfaceId() const
{
  return d->interfaceId;
}

std::size_t ServiceReferenceBase::Hash() const
{
  using namespace std;
  return hash<ServiceRegistrationBasePrivate*>()(this->d->registration);
}

}

using namespace us;

std::ostream& operator<<(std::ostream& os, const ServiceReferenceBase& serviceRef)
{
  if (serviceRef)
  {
    assert(serviceRef.GetBundle() != NULL);

    os << "Reference for service object registered from "
       << serviceRef.GetBundle()->GetName() << " " << serviceRef.GetBundle()->GetVersion()
       << " (";
    std::vector<std::string> keys;
    serviceRef.GetPropertyKeys(keys);
    size_t keySize = keys.size();
    for(size_t i = 0; i < keySize; ++i)
    {
      os << keys[i] << "=" << serviceRef.GetProperty(keys[i]).ToString();
      if (i < keySize-1) os << ",";
    }
    os << ")";
  }
  else
  {
    os << "Invalid service reference";
  }

  return os;
}
