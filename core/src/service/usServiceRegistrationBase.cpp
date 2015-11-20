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

#include "usServiceRegistrationBase.h"
#include "usServiceRegistrationBasePrivate.h"
#include "usServiceListenerEntry_p.h"
#include "usServiceRegistry_p.h"
#include "usServiceFactory.h"

#include "usBundlePrivate.h"
#include "usCoreBundleContext_p.h"

#include <stdexcept>

US_MSVC_DISABLE_WARNING(4503) // decorated name length exceeded, name was truncated

namespace us {

ServiceRegistrationBase::ServiceRegistrationBase()
  : d(nullptr)
{
}

ServiceRegistrationBase::ServiceRegistrationBase(const ServiceRegistrationBase& reg)
  : d(reg.d)
{
  if (d) ++d->ref;
}

ServiceRegistrationBase::ServiceRegistrationBase(ServiceRegistrationBasePrivate* registrationPrivate)
  : d(registrationPrivate)
{
  if (d) ++d->ref;
}

ServiceRegistrationBase::ServiceRegistrationBase(BundlePrivate* bundle, const InterfaceMapConstPtr& service,
                                                 ServicePropertiesImpl&& props)
  : d(new ServiceRegistrationBasePrivate(bundle, service, std::move(props)))
{
}

ServiceRegistrationBase::operator bool() const
{
  return d != nullptr;
}

ServiceRegistrationBase& ServiceRegistrationBase::operator=(std::nullptr_t)
{
  if (d && !--d->ref)
  {
    delete d;
  }
  d = nullptr;
  return *this;
}

ServiceRegistrationBase::~ServiceRegistrationBase()
{
  if (d && !--d->ref)
    delete d;
}

ServiceReferenceBase ServiceRegistrationBase::GetReference(const std::string& interfaceId) const
{
  if (!d) throw std::logic_error("ServiceRegistrationBase object invalid");
  if (!d->available) throw std::logic_error("Service is unregistered");

  auto l = d->Lock(); US_UNUSED(l);
  ServiceReferenceBase ref = d->reference;
  ref.SetInterfaceId(interfaceId);
  return ref;
}

void ServiceRegistrationBase::SetProperties(const ServiceProperties& props)
{
  if (!d) throw std::logic_error("ServiceRegistrationBase object invalid");

  ServiceEvent modifiedEndMatchEvent;
  ServiceEvent modifiedEvent;

  ServiceListeners::ServiceListenerEntries before;

  if (d->available)
  {
    {
      auto l = d->Lock(); US_UNUSED(l);
      if (!d->available) throw std::logic_error("Service is unregistered");
      modifiedEndMatchEvent = ServiceEvent(ServiceEvent::MODIFIED_ENDMATCH, d->reference);
      modifiedEvent = ServiceEvent(ServiceEvent::MODIFIED, d->reference);
    }

    // This calls into service event listener hooks. We must not hold any looks here
    d->bundle->coreCtx->listeners.GetMatchingServiceListeners(modifiedEndMatchEvent, before);

    int old_rank = 0;
    int new_rank = 0;
    std::vector<std::string> classes;
    {
      auto l = d->Lock(); US_UNUSED(l);
      if (!d->available) throw std::logic_error("Service is unregistered");

      {
        auto l2 = d->properties.Lock(); US_UNUSED(l2);

        Any any = d->properties.Value_unlocked(ServiceConstants::SERVICE_RANKING());
        if (any.Type() == typeid(int)) old_rank = any_cast<int>(any);

        classes = ref_any_cast<std::vector<std::string> >(d->properties.Value_unlocked(ServiceConstants::OBJECTCLASS()));

        long int sid = any_cast<long int>(d->properties.Value_unlocked(ServiceConstants::SERVICE_ID()));
        d->properties = ServiceRegistry::CreateServiceProperties(props, classes, false, false, sid);

        any = d->properties.Value_unlocked(ServiceConstants::SERVICE_RANKING());
        if (any.Type() == typeid(int)) new_rank = any_cast<int>(any);
      }
    }
    if (old_rank != new_rank)
    {
      d->bundle->coreCtx->services.UpdateServiceRegistrationOrder(*this, classes);
    }
  }
  else
  {
    throw std::logic_error("Service is unregistered");
  }

  // Notify listeners, we must no hold any locks here
  ServiceListeners::ServiceListenerEntries matchingListeners;
  d->bundle->coreCtx->listeners.GetMatchingServiceListeners(modifiedEvent, matchingListeners);
  d->bundle->coreCtx->listeners.ServiceChanged(matchingListeners,
                                               modifiedEvent,
                                               before);

  d->bundle->coreCtx->listeners.ServiceChanged(before,
                                               modifiedEndMatchEvent);
}

void ServiceRegistrationBase::Unregister()
{
  if (!d) throw std::logic_error("ServiceRegistrationBase object invalid");

  if (d->unregistering) return; // Silently ignore redundant unregistration.

  CoreBundleContext* coreContext = nullptr;

  if (d->available)
  {
    // Lock the service registry first
    auto l1 = d->bundle->coreCtx->services.Lock(); US_UNUSED(l1);
    auto l2 = d->Lock(); US_UNUSED(l2);
    if (d->unregistering) return;
    d->unregistering = true;

    d->bundle->coreCtx->services.RemoveServiceRegistration_unlocked(*this);
    coreContext = d->bundle->coreCtx;
  }
  else
  {
    throw std::logic_error("Service is unregistered");
  }

  if (coreContext)
  {
    // Notify listeners. We must not hold any locks here.
    ServiceListeners::ServiceListenerEntries listeners;
    ServiceEvent unregisteringEvent(ServiceEvent::UNREGISTERING, d->reference);
    coreContext->listeners.GetMatchingServiceListeners(unregisteringEvent, listeners);
    coreContext->listeners.ServiceChanged(
          listeners,
          unregisteringEvent);
  }

  std::shared_ptr<ServiceFactory> serviceFactory;
  ServiceRegistrationBasePrivate::BundleToServicesMap prototypeServiceInstances;
  ServiceRegistrationBasePrivate::BundleToServiceMap bundleServiceInstance;

  {
    auto l = d->Lock(); US_UNUSED(l);
    d->available = false;
    InterfaceMap::const_iterator factoryIter = d->service->find("org.cppmicroservices.factory");
    if (d->bundle && factoryIter != d->service->end())
    {
      serviceFactory = std::static_pointer_cast<ServiceFactory>(factoryIter->second);
    }
    if (serviceFactory)
    {
      prototypeServiceInstances = d->prototypeServiceInstances;
      bundleServiceInstance = d->bundleServiceInstance;
    }
  }

  if (serviceFactory)
  {
    // unget all prototype services
    for (auto const& i : prototypeServiceInstances)
    {
      for (auto const& service : i.second)
      {
        try
        {
          serviceFactory->UngetService(i.first, *this, service);
        }
        catch (const std::exception& /*ue*/)
        {
          US_WARN << "ServiceFactory UngetService implementation threw an exception";
        }
      }
    }

    // unget module scope services
    for (auto const& i : bundleServiceInstance)
    {
      try
      {
        serviceFactory->UngetService(i.first, *this, i.second);
      }
      catch (const std::exception& /*ue*/)
      {
        US_WARN << "ServiceFactory UngetService implementation threw an exception";
      }
    }
  }

  {
    auto l = d->Lock(); US_UNUSED(l);

    d->bundle = nullptr;
    d->dependents.clear();
    d->service.reset();
    d->prototypeServiceInstances.clear();
    d->bundleServiceInstance.clear();
    // increment the reference count, since "d->reference" was used originally
    // to keep d alive.
    ++d->ref;
    d->reference = nullptr;
    d->unregistering = false;
  }
}

bool ServiceRegistrationBase::operator<(const ServiceRegistrationBase& o) const
{
  if (this == &o || d == o.d) return false;

  if ((!d && !o.d) || !o.d) return false;
  if (!d) return true;

  ServiceReferenceBase sr1;
  ServiceReferenceBase sr2;
  {
    d->Lock(), sr1 = d->reference;
    o.d->Lock(), sr2 = o.d->reference;
  }
  return sr1 < sr2;
}

bool ServiceRegistrationBase::operator==(const ServiceRegistrationBase& registration) const
{
  return d == registration.d;
}

ServiceRegistrationBase& ServiceRegistrationBase::operator=(const ServiceRegistrationBase& registration)
{
  ServiceRegistrationBasePrivate* curr_d = d;
  d = registration.d;
  if (d) ++d->ref;

  if (curr_d && !--curr_d->ref)
    delete curr_d;

  return *this;
}

}
