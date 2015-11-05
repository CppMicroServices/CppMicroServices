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
  : d(ref.d.load())
{
  ++d.load()->ref;
}

ServiceReferenceBase::ServiceReferenceBase(ServiceRegistrationBasePrivate* reg)
  : d(new ServiceReferenceBasePrivate(reg))
{
}

void ServiceReferenceBase::SetInterfaceId(const std::string& interfaceId)
{
  if (d.load()->ref > 1)
  {
    // detach
    --d.load()->ref;
    d = new ServiceReferenceBasePrivate(d.load()->registration);
  }
  d.load()->interfaceId = interfaceId;
}

ServiceReferenceBase::operator bool() const
{
  return GetBundle() != nullptr;
}

ServiceReferenceBase& ServiceReferenceBase::operator=(std::nullptr_t)
{
  if (!--d.load()->ref)
    delete d;
  d = new ServiceReferenceBasePrivate(nullptr);
  return *this;
}

ServiceReferenceBase::~ServiceReferenceBase()
{
  if (!--d.load()->ref)
    delete d;
}

Any ServiceReferenceBase::GetProperty(const std::string& key) const
{
  auto l = d.load()->registration->properties.Lock();
  return d.load()->registration->properties.Value_unlocked(key);
}

void ServiceReferenceBase::GetPropertyKeys(std::vector<std::string>& keys) const
{
  auto l = d.load()->registration->properties.Lock();
  keys = d.load()->registration->properties.Keys_unlocked();
}

Bundle* ServiceReferenceBase::GetBundle() const
{
  auto p = d.load();
  if (p->registration == nullptr) return nullptr;

  auto l = p->registration->Lock();
  if (p->registration->bundle == nullptr) return nullptr;
  return p->registration->bundle->q;
}

void ServiceReferenceBase::GetUsingBundles(std::vector<Bundle*>& bundles) const
{
  auto l = d.load()->registration->Lock();
  for (auto& iter : d.load()->registration->dependents)
  {
    bundles.push_back(iter.first);
  }
}

bool ServiceReferenceBase::operator<(const ServiceReferenceBase& reference) const
{
  if (d.load() == reference.d.load()) return false;

  if (!(*this))
  {
    return true;
  }

  if (!reference)
  {
    return false;
  }

  if (d.load()->registration == reference.d.load()->registration)
  {
    return false;
  }


  Any anyR1;
  Any anyId1;
  {
    auto l = d.load()->registration->properties.Lock();
    anyR1 = d.load()->registration->properties.Value_unlocked(ServiceConstants::SERVICE_RANKING());
    assert(anyR1.Empty() || anyR1.Type() == typeid(int));
    anyId1 = d.load()->registration->properties.Value_unlocked(ServiceConstants::SERVICE_ID());
    assert(anyId1.Type() == typeid(long int));
  }

  Any anyR2;
  Any anyId2;
  {
    auto l = reference.d.load()->registration->properties.Lock();
    anyR2 = reference.d.load()->registration->properties.Value_unlocked(ServiceConstants::SERVICE_RANKING());
    assert(anyR2.Empty() || anyR2.Type() == typeid(int));
    anyId2 = reference.d.load()->registration->properties.Value_unlocked(ServiceConstants::SERVICE_ID());
    assert(anyId2.Type() == typeid(long int));
  }

  const int r1 = anyR1.Empty() ? 0 : *any_cast<int>(&anyR1);
  const int r2 = anyR2.Empty() ? 0 : *any_cast<int>(&anyR2);

  if (r1 != r2)
  {
    // use ranking if ranking differs
    return r1 < r2;
  }
  else
  {
    const long int id1 = *any_cast<long int>(&anyId1);
    const long int id2 = *any_cast<long int>(&anyId2);

    // otherwise compare using IDs,
    // is less than if it has a higher ID.
    return id2 < id1;
  }
}

bool ServiceReferenceBase::operator==(const ServiceReferenceBase& reference) const
{
  return d.load()->registration == reference.d.load()->registration;
}

ServiceReferenceBase& ServiceReferenceBase::operator=(const ServiceReferenceBase& reference)
{
  if (d == reference.d.load()) return *this;

  ServiceReferenceBasePrivate* old_d = d;
  ServiceReferenceBasePrivate* new_d = reference.d;
  ++new_d->ref;
  d = new_d;

  if (!--old_d->ref)
    delete old_d;

  return *this;
}

bool ServiceReferenceBase::IsConvertibleTo(const std::string& interfaceId) const
{
  return d.load()->IsConvertibleTo(interfaceId);
}

std::string ServiceReferenceBase::GetInterfaceId() const
{
  return d.load()->interfaceId;
}

std::size_t ServiceReferenceBase::Hash() const
{
  using namespace std;
  return hash<ServiceRegistrationBasePrivate*>()(this->d.load()->registration);
}

std::ostream& operator<<(std::ostream& os, const ServiceReferenceBase& serviceRef)
{
  if (serviceRef)
  {
    assert(serviceRef.GetBundle() != nullptr);

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

}
