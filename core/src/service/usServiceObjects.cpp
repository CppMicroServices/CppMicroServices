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

#include "usServiceObjects.h"

#include "usServiceReferenceBasePrivate.h"
#include "usLog.h"

#include <set>
#include <atomic>

namespace us {

class ServiceObjectsBasePrivate
{
public:

  std::atomic<int> ref;

  BundleContext* m_context;
  ServiceReferenceBase m_reference;

  ServiceObjectsBasePrivate(BundleContext* context, const ServiceReferenceBase& reference)
    : m_context(context)
    , m_reference(reference)
  {}

  InterfaceMapConstPtr GetServiceInterfaceMap()
  {
    InterfaceMapConstPtr result;

    bool isPrototypeScope = m_reference.GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() ==
                            ServiceConstants::SCOPE_PROTOTYPE();

    if (isPrototypeScope)
    {
      result = m_reference.d->GetPrototypeService(m_context->GetBundle());
    }
    else
    {
      result = m_reference.d->GetServiceInterfaceMap(m_context->GetBundle());
    }

    return result;
  }
};

ServiceObjectsBase::ServiceObjectsBase(BundleContext* context, const ServiceReferenceBase& reference)
  : d(new ServiceObjectsBasePrivate(context, reference))
{
  if (!reference)
  {
    delete d;
    throw std::invalid_argument("The service reference is invalid");
  }
  ++d->ref;
}

/* @brief Private helper struct used to facilitate the shared_ptr aliasing constructor
 *        in ServiceObjectsBase::GetService & ServiceObjectsBase::GetServiceInterfaceMap
 *        methods. The aliasing constructor helps automate the call to UngetService method.
 *
 *        Service consumers can simply call GetService to obtain a shared_ptr to the
 *        service object and not worry about calling UngetService when they are done.
 *        The UngetService is called when all instances of the returned shared_ptr object
 *        go out of scope.
 */
struct UngetHelper
{
  const InterfaceMapConstPtr interfaceMap;
  const ServiceReferenceBase sref;
  BundleContext* const bc;

  UngetHelper(const InterfaceMapConstPtr& im, const ServiceReferenceBase& sr, BundleContext* bc)
    : interfaceMap(im)
    , sref(sr)
    , bc(bc)
  {}
  ~UngetHelper()
  {
    try
    {
      if(sref && bc->GetBundle() != nullptr)
      {
        bool isPrototypeScope = sref.GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() ==
        ServiceConstants::SCOPE_PROTOTYPE();

        if (isPrototypeScope)
        {
          sref.d->UngetPrototypeService(bc->GetBundle(), interfaceMap);
        }
        else
        {
          sref.d->UngetService(bc->GetBundle(), true);
        }
      }
    }
    catch (const std::exception& ex)
    {
      US_INFO << "UngetService threw an exception - " << ex.what();
    }
  }
};

std::shared_ptr<void> ServiceObjectsBase::GetService() const
{
  if (!d->m_reference)
  {
    return nullptr;
  }

  std::shared_ptr<UngetHelper> h(new UngetHelper(d->GetServiceInterfaceMap(), d->m_reference, d->m_context));
  return std::shared_ptr<void>(h, (h->interfaceMap->find(d->m_reference.GetInterfaceId()))->second.get());
}

InterfaceMapConstPtr ServiceObjectsBase::GetServiceInterfaceMap() const
{
  InterfaceMapConstPtr result;
  if (!d->m_reference)
  {
    return result;
  }
  // copy construct a new map to be handed out to consumers
  result = std::make_shared<const InterfaceMap>(*(d->GetServiceInterfaceMap().get()));
  std::shared_ptr<UngetHelper> h(new UngetHelper{ result, d->m_reference, d->m_context });
  return InterfaceMapConstPtr(h, h->interfaceMap.get());
}

ServiceReferenceBase ServiceObjectsBase::GetReference() const
{
  return d->m_reference;
}

ServiceObjectsBase::ServiceObjectsBase(const ServiceObjectsBase& other)
  : d(other.d)
{
  ++d->ref;
}

ServiceObjectsBase::~ServiceObjectsBase()
{
  if (!--d->ref)
  {
    delete d;
  }
}

ServiceObjectsBase& ServiceObjectsBase::operator =(const ServiceObjectsBase& other)
{
  ServiceObjectsBasePrivate* curr_d = d;
  d = other.d;
  ++d->ref;

  if (!--curr_d->ref)
    delete curr_d;

  return *this;
}

InterfaceMapConstPtr ServiceObjects<void>::GetService() const
{
  return this->ServiceObjectsBase::GetServiceInterfaceMap();
}

ServiceReferenceU ServiceObjects<void>::GetServiceReference() const
{
  return this->ServiceObjectsBase::GetReference();
}

ServiceObjects<void>::ServiceObjects(BundleContext* context, const ServiceReferenceU& reference)
  : ServiceObjectsBase(context, reference)
{}

}
