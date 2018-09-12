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

#include "cppmicroservices/ServiceReferenceBase.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/Constants.h"

#include "BundlePrivate.h"
#include "ServiceReferenceBasePrivate.h"
#include "ServiceRegistrationBasePrivate.h"

#include <cassert>

namespace cppmicroservices {

ServiceReferenceBase::ServiceReferenceBase()
  : impl_ptr(std::make_shared<ServiceReferenceBasePrivate>(nullptr))
{}

ServiceReferenceBase::ServiceReferenceBase(const ServiceReferenceBase& ref)
  : impl_ptr(ref.impl())
{
}

ServiceReferenceBase::ServiceReferenceBase(const std::shared_ptr<ServiceRegistrationBasePrivate>& reg)
    : impl_ptr(std::make_shared<ServiceReferenceBasePrivate>(reg))
{}

void ServiceReferenceBase::SetInterfaceId(const std::string& interfaceId)
{
  impl_ptr = std::make_shared<ServiceReferenceBasePrivate>(impl()->registration);
  impl()->interfaceId = interfaceId;
}

ServiceReferenceBase::operator bool() const
{
  return static_cast<bool>(GetBundle());
}

ServiceReferenceBase& ServiceReferenceBase::operator=(std::nullptr_t)
{
  set_impl_ptr(std::make_shared<ServiceReferenceBasePrivate>(nullptr));
  return *this;
}

ServiceReferenceBase::~ServiceReferenceBase()
{
}

Any ServiceReferenceBase::GetProperty(const std::string& key) const
{
  auto l = impl()->registration->properties.Lock();
  US_UNUSED(l);
  return impl()->registration->properties.Value_unlocked(key);
}

void ServiceReferenceBase::GetPropertyKeys(std::vector<std::string>& keys) const
{
  keys = GetPropertyKeys();
}

std::vector<std::string> ServiceReferenceBase::GetPropertyKeys() const
{
  auto l = impl()->registration->properties.Lock();
  US_UNUSED(l);
  return impl()->registration->properties.Keys_unlocked();
}

Bundle ServiceReferenceBase::GetBundle() const
{
  auto p = impl();
  if (p->registration == nullptr)
    return Bundle();

  auto l = p->registration->Lock();
  US_UNUSED(l);
  if (p->registration->bundle == nullptr)
    return Bundle();
  return MakeBundle(p->registration->bundle->shared_from_this());
}

std::vector<Bundle> ServiceReferenceBase::GetUsingBundles() const
{
  std::vector<Bundle> bundles;
  auto l = impl()->registration->Lock();
  US_UNUSED(l);
  for (auto& iter : impl()->registration->dependents) {
    bundles.push_back(MakeBundle(iter.first->shared_from_this()));
  }
  return bundles;
}

bool ServiceReferenceBase::operator<(
  const ServiceReferenceBase& reference) const
{
  if (impl() == reference.impl())
    return false;

  if (!(*this)) {
    return true;
  }

  if (!reference) {
    return false;
  }

  if (impl()->registration == reference.impl()->registration) {
    return false;
  }

  Any anyR1;
  Any anyId1;
  {
    auto l = impl()->registration->properties.Lock();
    US_UNUSED(l);
    anyR1 = impl()->registration->properties.Value_unlocked(
      Constants::SERVICE_RANKING);
    assert(anyR1.Empty() || anyR1.Type() == typeid(int));
    anyId1 =
      impl()->registration->properties.Value_unlocked(Constants::SERVICE_ID);
    assert(anyId1.Type() == typeid(long int));
  }

  Any anyR2;
  Any anyId2;
  {
    auto l = reference.impl()->registration->properties.Lock();
    US_UNUSED(l);
    anyR2 = reference.impl()->registration->properties.Value_unlocked(
      Constants::SERVICE_RANKING);
    assert(anyR2.Empty() || anyR2.Type() == typeid(int));
    anyId2 = reference.impl()->registration->properties.Value_unlocked(
      Constants::SERVICE_ID);
    assert(anyId2.Type() == typeid(long int));
  }

  const int r1 = anyR1.Empty() ? 0 : *any_cast<int>(&anyR1);
  const int r2 = anyR2.Empty() ? 0 : *any_cast<int>(&anyR2);

  if (r1 != r2) {
    // use ranking if ranking differs
    return r1 < r2;
  } else {
    const long int id1 = *any_cast<long int>(&anyId1);
    const long int id2 = *any_cast<long int>(&anyId2);

    // otherwise compare using IDs,
    // is less than if it has a higher ID.
    return id2 < id1;
  }
}

bool ServiceReferenceBase::operator==(const ServiceReferenceBase& reference) const
{
  return impl()->registration == reference.impl()->registration;
}

ServiceReferenceBase& ServiceReferenceBase::operator=(const ServiceReferenceBase& reference)
{
  if (impl_ptr == reference.impl())
    return *this;

  set_impl_ptr(reference.impl_ptr);
  return *this;
}

bool ServiceReferenceBase::IsConvertibleTo(const std::string& interfaceId) const
{
  return impl()->IsConvertibleTo(interfaceId);
}

std::string ServiceReferenceBase::GetInterfaceId() const
{
  return impl()->interfaceId;
}

std::size_t ServiceReferenceBase::Hash() const
{
  using namespace std;
  return hash<ServiceRegistrationBasePrivate*>()(this->impl()->registration.get());
}

std::ostream& operator<<(std::ostream& os,
                         const ServiceReferenceBase& serviceRef)
{
  if (serviceRef) {
    assert(serviceRef.GetBundle());

    os << "Reference for service object registered from "
       << serviceRef.GetBundle().GetSymbolicName() << " "
       << serviceRef.GetBundle().GetVersion() << " (";
    std::vector<std::string> keys = serviceRef.GetPropertyKeys();
    size_t keySize = keys.size();
    for (size_t i = 0; i < keySize; ++i) {
      os << keys[i] << "=" << serviceRef.GetProperty(keys[i]).ToString();
      if (i < keySize - 1)
        os << ",";
    }
    os << ")";
  } else {
    os << "Invalid service reference";
  }

  return os;
}
}
