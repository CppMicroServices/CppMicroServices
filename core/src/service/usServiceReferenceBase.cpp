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

#include "usServiceReferenceBase.h"
#include "usServiceReferenceBasePrivate.h"
#include "usServiceRegistrationBasePrivate.h"

#include "usModule.h"
#include "usModulePrivate.h"

#include <cassert>

US_BEGIN_NAMESPACE

ServiceReferenceBase::ServiceReferenceBase()
  : d(new ServiceReferenceBasePrivate(0))
{

}

ServiceReferenceBase::ServiceReferenceBase(const ServiceReferenceBase& ref)
  : d(ref.d)
{
  d->ref.Ref();
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
    d->ref.Deref();
    d = new ServiceReferenceBasePrivate(d->registration);
  }
  d->interfaceId = interfaceId;
}

ServiceReferenceBase::operator bool_type() const
{
  return GetModule() != 0 ? &ServiceReferenceBase::d : NULL;
}

ServiceReferenceBase& ServiceReferenceBase::operator=(int null)
{
  if (null == 0)
  {
    if (!d->ref.Deref())
      delete d;
    d = new ServiceReferenceBasePrivate(0);
  }
  return *this;
}

ServiceReferenceBase::~ServiceReferenceBase()
{
  if (!d->ref.Deref())
    delete d;
}

Any ServiceReferenceBase::GetProperty(const std::string& key) const
{
  MutexLock lock(d->registration->propsLock);

  return d->registration->properties.Value(key);
}

void ServiceReferenceBase::GetPropertyKeys(std::vector<std::string>& keys) const
{
  MutexLock lock(d->registration->propsLock);

  const std::vector<std::string>& ks = d->registration->properties.Keys();
  keys.assign(ks.begin(), ks.end());
}

Module* ServiceReferenceBase::GetModule() const
{
  if (d->registration == 0 || d->registration->module == 0)
  {
    return 0;
  }

  return d->registration->module->q;
}

void ServiceReferenceBase::GetUsingModules(std::vector<Module*>& modules) const
{
  MutexLock lock(d->registration->propsLock);

  ServiceRegistrationBasePrivate::ModuleToRefsMap::const_iterator end = d->registration->dependents.end();
  for (ServiceRegistrationBasePrivate::ModuleToRefsMap::const_iterator iter = d->registration->dependents.begin();
       iter != end; ++iter)
  {
    modules.push_back(iter->first);
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
  d->ref.Ref();

  if (!curr_d->ref.Deref())
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
  using namespace US_HASH_FUNCTION_NAMESPACE;
  return US_HASH_FUNCTION(ServiceRegistrationBasePrivate*, this->d->registration);
}

US_END_NAMESPACE

US_USE_NAMESPACE

std::ostream& operator<<(std::ostream& os, const ServiceReferenceBase& serviceRef)
{
  if (serviceRef)
  {
    assert(serviceRef.GetModule() != NULL);

    os << "Reference for service object registered from "
       << serviceRef.GetModule()->GetName() << " " << serviceRef.GetModule()->GetVersion()
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
