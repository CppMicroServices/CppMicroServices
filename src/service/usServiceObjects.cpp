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

#include "usServiceObjects.h"

#include "usServiceReferenceBasePrivate.h"

#include <set>

US_BEGIN_NAMESPACE

struct ServiceObjectsBasePrivate
{
  AtomicInt ref;

  ModuleContext* m_context;
  ServiceReferenceBase m_reference;

  std::set<void*> m_serviceInstances;

  ServiceObjectsBasePrivate(ModuleContext* context, const ServiceReferenceBase& reference)
    : m_context(context)
    , m_reference(reference)
  {}
};

ServiceObjectsBase::ServiceObjectsBase(ModuleContext* context, const ServiceReferenceBase& reference)
  : d(new ServiceObjectsBasePrivate(context, reference))
{
  if (!reference)
  {
    delete d;
    throw std::invalid_argument("The service reference is invalid");
  }
}

void* ServiceObjectsBase::GetService() const
{
  if (!d->m_reference)
  {
    return NULL;
  }

  void* result = NULL;

  bool isPrototypeScope = d->m_reference.GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() ==
      ServiceConstants::SCOPE_PROTOTYPE();

  if (isPrototypeScope)
  {
    result = d->m_reference.d->GetPrototypeService(d->m_context->GetModule());
  }
  else
  {
    result = d->m_context->GetService(d->m_reference);
  }

  if (result)
  {
    d->m_serviceInstances.insert(result);
  }
  return result;
}

void ServiceObjectsBase::UngetService(void* service)
{
  std::set<void*>::iterator serviceIter = d->m_serviceInstances.find(service);
  if (serviceIter == d->m_serviceInstances.end())
  {
    throw std::invalid_argument("The provided service has not been retrieved via this ServiceObjects instance");
  }

  if (!d->m_reference.d->UngetPrototypeService(d->m_context->GetModule(), service))
  {
    US_WARN << "Ungetting service unsuccessful";
  }
  else
  {
    d->m_serviceInstances.erase(serviceIter);
  }
}

ServiceReferenceBase ServiceObjectsBase::GetReference() const
{
  return d->m_reference;
}

ServiceObjectsBase::ServiceObjectsBase(const ServiceObjectsBase& other)
  : d(other.d)
{
  d->ref.Ref();
}

ServiceObjectsBase::~ServiceObjectsBase()
{
  if (!d->ref.Deref())
  {
    delete d;
  }
}

ServiceObjectsBase& ServiceObjectsBase::operator =(const ServiceObjectsBase& other)
{
  ServiceObjectsBasePrivate* curr_d = d;
  d = other.d;
  d->ref.Ref();

  if (!curr_d->ref.Deref())
    delete curr_d;

  return *this;
}

US_END_NAMESPACE
