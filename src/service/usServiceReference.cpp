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

#include "usServiceReference.h"
#include "usServiceReferencePrivate.h"
#include "usServiceRegistrationPrivate.h"

#include "usModule.h"
#include "usModulePrivate.h"


US_BEGIN_NAMESPACE

typedef ServiceRegistrationPrivate::MutexType MutexType;
typedef MutexLock<MutexType> MutexLocker;

ServiceReference::ServiceReference()
  : d(new ServiceReferencePrivate(0))
{

}

ServiceReference::ServiceReference(const ServiceReference& ref)
  : d(ref.d)
{
  d->ref.Ref();
}

ServiceReference::ServiceReference(ServiceRegistrationPrivate* reg)
  : d(new ServiceReferencePrivate(reg))
{

}

ServiceReference::operator bool() const
{
  return GetModule() != 0;
}

ServiceReference& ServiceReference::operator=(int null)
{
  if (null == 0)
  {
    if (!d->ref.Deref())
      delete d;
    d = new ServiceReferencePrivate(0);
  }
  return *this;
}

ServiceReference::~ServiceReference()
{
  if (!d->ref.Deref())
    delete d;
}

Any ServiceReference::GetProperty(const std::string& key) const
{
  MutexLocker lock(d->registration->propsLock);

  ServiceProperties::const_iterator iter = d->registration->properties.find(key);
  if (iter != d->registration->properties.end())
    return iter->second;
  return Any();
}

void ServiceReference::GetPropertyKeys(std::vector<std::string>& keys) const
{
  MutexLocker lock(d->registration->propsLock);

  ServiceProperties::const_iterator iterEnd = d->registration->properties.end();
  for (ServiceProperties::const_iterator iter = d->registration->properties.begin();
       iter != iterEnd; ++iter)
  {
    keys.push_back(iter->first);
  }
}

Module* ServiceReference::GetModule() const
{
  if (d->registration == 0 || d->registration->module == 0)
  {
    return 0;
  }

  return d->registration->module->q;
}

void ServiceReference::GetUsingModules(std::vector<Module*>& modules) const
{
  MutexLocker lock(d->registration->propsLock);

  ServiceRegistrationPrivate::ModuleToRefsMap::const_iterator end = d->registration->dependents.end();
  for (ServiceRegistrationPrivate::ModuleToRefsMap::const_iterator iter = d->registration->dependents.begin();
       iter != end; ++iter)
  {
    modules.push_back(iter->first);
  }
}

bool ServiceReference::operator<(const ServiceReference& reference) const
{
  int r1 = 0;
  int r2 = 0;

  Any anyR1 = GetProperty(ServiceConstants::SERVICE_RANKING());
  Any anyR2 = reference.GetProperty(ServiceConstants::SERVICE_RANKING());
  if (anyR1.Type() == typeid(int)) r1 = any_cast<int>(anyR1);
  if (anyR2.Type() == typeid(int)) r2 = any_cast<int>(anyR2);

  if (r1 != r2)
  {
    // use ranking if ranking differs
    return r1 < r2;
  }
  else
  {
    long int id1 = any_cast<long int>(GetProperty(ServiceConstants::SERVICE_ID()));
    long int id2 = any_cast<long int>(reference.GetProperty(ServiceConstants::SERVICE_ID()));

    // otherwise compare using IDs,
    // is less than if it has a higher ID.
    return id2 < id1;
  }
}

bool ServiceReference::operator==(const ServiceReference& reference) const
{
  return d->registration == reference.d->registration;
}

ServiceReference& ServiceReference::operator=(const ServiceReference& reference)
{
  ServiceReferencePrivate* curr_d = d;
  d = reference.d;
  d->ref.Ref();

  if (!curr_d->ref.Deref())
    delete curr_d;

  return *this;
}

std::size_t ServiceReference::Hash() const
{
  using namespace US_HASH_FUNCTION_NAMESPACE;
  return US_HASH_FUNCTION(ServiceRegistrationPrivate*, this->d->registration);
}

US_END_NAMESPACE

US_USE_NAMESPACE

std::ostream& operator<<(std::ostream& os, const ServiceReference& serviceRef)
{
  os << "Reference for service object registered from "
     << serviceRef.GetModule()->GetName() << " " << serviceRef.GetModule()->GetVersion()
     << " (";
  std::vector<std::string> keys;
  serviceRef.GetPropertyKeys(keys);
  int keySize = keys.size();
  for(int i = 0; i < keySize; ++i)
  {
    os << keys[i] << "=" << serviceRef.GetProperty(keys[i]);
    if (i < keySize-1) os << ",";
  }
  os << ")";

  return os;
}

