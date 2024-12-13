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

#include "cppmicroservices/ServiceObjects.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/detail/Log.h"
#include "cppmicroservices/util/Error.h"

#include "BundleContextPrivate.h"
#include "BundlePrivate.h"
#include "CoreBundleContext.h"
#include "ServiceReferenceBasePrivate.h"

#include <set>
#include <utility>

namespace cppmicroservices
{

    class ServiceObjectsBasePrivate
    {
      public:
        std::shared_ptr<BundleContextPrivate> m_context;
        ServiceReferenceBase m_reference;

        ServiceObjectsBasePrivate(std::shared_ptr<BundleContextPrivate> context, ServiceReferenceBase const& reference)
            : m_context(std::move(context))
            , m_reference(reference)
        {
        }

        InterfaceMapConstPtr
        GetServiceInterfaceMap()
        {
            InterfaceMapConstPtr result;

            bool isPrototypeScope
                = m_reference.GetProperty(Constants::SERVICE_SCOPE).ToString() == Constants::SCOPE_PROTOTYPE;

            if (isPrototypeScope)
            {
                result = m_reference.d.Load()->GetPrototypeService(MakeBundleContext(m_context).GetBundle());
            }
            else
            {
                result = m_reference.d.Load()->GetServiceInterfaceMap(
                    GetPrivate(MakeBundleContext(m_context).GetBundle()).get());
            }

            return result;
        }
    };

    ServiceObjectsBase::ServiceObjectsBase(std::shared_ptr<BundleContextPrivate> const& context,
                                           ServiceReferenceBase const& reference)
        : d(new ServiceObjectsBasePrivate(context, reference))
    {
        if (!reference)
        {
            throw std::invalid_argument("The service reference is invalid");
        }
    }

    std::shared_ptr<void>
    ServiceObjectsBase::GetService() const
    {
        if (!d->m_reference)
        {
            return nullptr;
        }

        auto interfaceMap = d->GetServiceInterfaceMap();
        // interfaceMap can be null under certain circumstance e.g. if a constructor throws an
        // exception in the service implementation.  In that case, we bail out early.
        if (!interfaceMap)
        {
            return nullptr;
        }

        auto bundle_ = d->m_context->bundle.lock();
        if (!bundle_)
        {
            return nullptr;
        }

        auto h = std::make_shared<ServiceHolder<void>>(bundle_, d->m_reference, nullptr, interfaceMap);
        auto deleter = h->interfaceMap->find(d->m_reference.GetInterfaceId())->second.get();
        return std::shared_ptr<void>(h, deleter);
    }

    InterfaceMapConstPtr
    ServiceObjectsBase::GetServiceInterfaceMap() const
    {
        InterfaceMapConstPtr result;
        if (!d->m_reference)
        {
            return result;
        }
        // copy construct a new map to be handed out to consumers
        auto interfaceMap = d->GetServiceInterfaceMap();
        // There are some circumstances that will cause the interfaceMap to be empty
        // like when another thread has unregistered the service.
        if (!interfaceMap)
        {
            return result;
        }
        result = std::make_shared<InterfaceMap const>(*(interfaceMap.get()));

        auto bundle_ = d->m_context->bundle.lock();
        if (!bundle_)
        {
            return nullptr;
        }
        auto sh = new ServiceHolder<void> { bundle_, d->m_reference, nullptr, result };
        std::shared_ptr<ServiceHolder<void>> h(sh, CustomServiceDeleter { sh });
        return InterfaceMapConstPtr(h, h->interfaceMap.get());
    }

    ServiceReferenceBase
    ServiceObjectsBase::GetReference() const
    {
        return d->m_reference;
    }

    ServiceObjectsBase::ServiceObjectsBase(ServiceObjectsBase&& other) noexcept : d(std::move(other.d)) {}

    ServiceObjectsBase::~ServiceObjectsBase() = default;

    ServiceObjectsBase&
    ServiceObjectsBase::operator=(ServiceObjectsBase&& other) noexcept
    {
        d = std::move(other.d);
        return *this;
    }

    ServiceObjects<void>::ServiceObjects(ServiceObjects&& other) noexcept : ServiceObjectsBase(std::move(other)) {}

    ServiceObjects<void>&
    ServiceObjects<void>::operator=(ServiceObjects&& other) noexcept
    {
        ServiceObjectsBase::operator=(std::move(other));
        return *this;
    }

    InterfaceMapConstPtr
    ServiceObjects<void>::GetService() const
    {
        return this->ServiceObjectsBase::GetServiceInterfaceMap();
    }

    ServiceReferenceU
    ServiceObjects<void>::GetServiceReference() const
    {
        return this->ServiceObjectsBase::GetReference();
    }

    ServiceObjects<void>::ServiceObjects(std::shared_ptr<BundleContextPrivate> const& context,
                                         ServiceReferenceU const& reference)
        : ServiceObjectsBase(context, reference)
    {
    }
} // namespace cppmicroservices
