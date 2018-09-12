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

#include "cppmicroservices/ServiceRegistrationBase.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/ServiceFactory.h"

#include "BundlePrivate.h"
#include "CoreBundleContext.h"
#include "ServiceListenerEntry.h"
#include "ServiceRegistrationBasePrivate.h"
#include "ServiceRegistry.h"

#include <stdexcept>

US_MSVC_DISABLE_WARNING(
  4503) // decorated name length exceeded, name was truncated

namespace cppmicroservices {

ServiceRegistrationBase::ServiceRegistrationBase()
  : regdata_ptr(nullptr)
{}

ServiceRegistrationBase::ServiceRegistrationBase(
  const ServiceRegistrationBase& reg)
  : regdata_ptr(reg.regdata_ptr)
{
}

ServiceRegistrationBase::ServiceRegistrationBase(ServiceRegistrationBase&& reg)
  : regdata_ptr(nullptr)
{
  std::swap(regdata_ptr, reg.regdata_ptr);
}

ServiceRegistrationBase::ServiceRegistrationBase(const std::shared_ptr<ServiceRegistrationBasePrivate>& registrationPrivate)
  : regdata_ptr(registrationPrivate)
{
}

ServiceRegistrationBase::ServiceRegistrationBase(BundlePrivate* bundle,
                                                 const InterfaceMapConstPtr& service,
                                                 Properties&& props)
  : regdata_ptr(ServiceRegistrationBasePrivate::create(bundle,
                                                       service,
                                                       std::move(props)))
{}

ServiceRegistrationBase::operator bool() const
{
  return regdata_ptr != nullptr;
}

ServiceRegistrationBase& ServiceRegistrationBase::operator=(std::nullptr_t)
{
  regdata_ptr.reset();
  return *this;
}

ServiceRegistrationBase::~ServiceRegistrationBase()
{
}

ServiceReferenceBase ServiceRegistrationBase::GetReference(const std::string& interfaceId) const
{
  if (!regdata_ptr)
    throw std::logic_error("ServiceRegistrationBase object invalid");
  if (!regdata_ptr->available)
    throw std::logic_error("Service is unregistered");
 
  auto l = regdata_ptr->Lock();
  US_UNUSED(l);
  ServiceReferenceBase ref = regdata_ptr->reference;
  ref.SetInterfaceId(interfaceId);
  return ref;
}

void ServiceRegistrationBase::SetProperties(const ServiceProperties& props)
{
  if (!regdata_ptr)
    throw std::logic_error("ServiceRegistrationBase object invalid");

  ServiceEvent modifiedEndMatchEvent;
  ServiceEvent modifiedEvent;

  ServiceListeners::ServiceListenerEntries before;

  if (regdata_ptr->available) {
    {
      auto l = regdata_ptr->Lock();
      US_UNUSED(l);
      if (!regdata_ptr->available)
        throw std::logic_error("Service is unregistered");
      modifiedEndMatchEvent =
        ServiceEvent(ServiceEvent::SERVICE_MODIFIED_ENDMATCH, regdata_ptr->reference);
      modifiedEvent =
        ServiceEvent(ServiceEvent::SERVICE_MODIFIED, regdata_ptr->reference);
    }

    // This calls into service event listener hooks. We must not hold any looks here
    regdata_ptr->bundle->coreCtx->listeners.GetMatchingServiceListeners(
      modifiedEndMatchEvent, before);

    int old_rank = 0;
    int new_rank = 0;
    std::vector<std::string> classes;
    {
      auto l = regdata_ptr->Lock();
      US_UNUSED(l);
      if (!regdata_ptr->available)
        throw std::logic_error("Service is unregistered");

      {
        auto l2 = regdata_ptr->properties.Lock();
        US_UNUSED(l2);

        Any any = regdata_ptr->properties.Value_unlocked(Constants::SERVICE_RANKING);
        if (any.Type() == typeid(int))
          old_rank = any_cast<int>(any);

        classes = ref_any_cast<std::vector<std::string>>(
          regdata_ptr->properties.Value_unlocked(Constants::OBJECTCLASS));

        long int sid = any_cast<long int>(
          regdata_ptr->properties.Value_unlocked(Constants::SERVICE_ID));
        regdata_ptr->properties = ServiceRegistry::CreateServiceProperties(
          props, classes, false, false, sid);

        any = regdata_ptr->properties.Value_unlocked(Constants::SERVICE_RANKING);
        if (any.Type() == typeid(int))
          new_rank = any_cast<int>(any);
      }
    }
    if (old_rank != new_rank) {
      regdata_ptr->bundle->coreCtx->services.UpdateServiceRegistrationOrder(*this,
                                                                  classes);
    }
  } else {
    throw std::logic_error("Service is unregistered");
  }

  // Notify listeners, we must no hold any locks here
  ServiceListeners::ServiceListenerEntries matchingListeners;
  regdata_ptr->bundle->coreCtx->listeners.GetMatchingServiceListeners(modifiedEvent,
                                                            matchingListeners);
  regdata_ptr->bundle->coreCtx->listeners.ServiceChanged(
    matchingListeners, modifiedEvent, before);

  regdata_ptr->bundle->coreCtx->listeners.ServiceChanged(before, modifiedEndMatchEvent);
}

void ServiceRegistrationBase::Unregister()
{
  if (!regdata_ptr)
    throw std::logic_error("ServiceRegistrationBase object invalid");

  if (regdata_ptr->unregistering)
    return; // Silently ignore redundant unregistration.

  CoreBundleContext* coreContext = nullptr;

  if (regdata_ptr->available) {
    // Lock the service registry first
    auto l1 = regdata_ptr->bundle->coreCtx->services.Lock();
    US_UNUSED(l1);
    auto l2 = regdata_ptr->Lock();
    US_UNUSED(l2);
    if (regdata_ptr->unregistering)
      return;
    regdata_ptr->unregistering = true;

    regdata_ptr->bundle->coreCtx->services.RemoveServiceRegistration_unlocked(*this);
    coreContext = regdata_ptr->bundle->coreCtx;
  } else {
    throw std::logic_error("Service is unregistered");
  }

  if (coreContext) {
    // Notify listeners. We must not hold any locks here.
    ServiceListeners::ServiceListenerEntries listeners;
    ServiceEvent unregisteringEvent(ServiceEvent::SERVICE_UNREGISTERING,
                                    regdata_ptr->reference);
    coreContext->listeners.GetMatchingServiceListeners(unregisteringEvent,
                                                       listeners);
    coreContext->listeners.ServiceChanged(listeners, unregisteringEvent);
  }

  std::shared_ptr<ServiceFactory> serviceFactory;
  ServiceRegistrationBasePrivate::BundleToServicesMap prototypeServiceInstances;
  ServiceRegistrationBasePrivate::BundleToServiceMap bundleServiceInstance;

  {
    auto l = regdata_ptr->Lock();
    US_UNUSED(l);
    regdata_ptr->available = false;
    InterfaceMap::const_iterator factoryIter =
      regdata_ptr->service->find("org.cppmicroservices.factory");
    if (regdata_ptr->bundle && factoryIter != regdata_ptr->service->end()) {
      serviceFactory =
        std::static_pointer_cast<ServiceFactory>(factoryIter->second);
    }
    if (serviceFactory) {
      prototypeServiceInstances = regdata_ptr->prototypeServiceInstances;
      bundleServiceInstance = regdata_ptr->bundleServiceInstance;
    }
  }

  if (serviceFactory) {
    // unget all prototype services
    for (auto const& i : prototypeServiceInstances) {
      for (auto const& service : i.second) {
        try {
          serviceFactory->UngetService(
            MakeBundle(i.first->shared_from_this()), *this, service);
        } catch (...) {
          std::string message(
            "ServiceFactory UngetService implementation threw an exception");
          regdata_ptr->bundle->coreCtx->listeners.SendFrameworkEvent(
            FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING,
                           MakeBundle(regdata_ptr->bundle->shared_from_this()),
                           message,
                           std::current_exception()));
        }
      }
    }

    // unget bundle scope services
    for (auto const& i : bundleServiceInstance) {
      try {
        serviceFactory->UngetService(
          MakeBundle(i.first->shared_from_this()), *this, i.second);
      } catch (...) {
        std::string message(
          "ServiceFactory UngetService implementation threw an exception");
        regdata_ptr->bundle->coreCtx->listeners.SendFrameworkEvent(
          FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING,
                         MakeBundle(regdata_ptr->bundle->shared_from_this()),
                         message,
                         std::current_exception()));
      }
    }
  }

  {
    auto l = regdata_ptr->Lock();
    US_UNUSED(l);

    regdata_ptr->bundle = nullptr;
    regdata_ptr->dependents.clear();
    regdata_ptr->service.reset();
    regdata_ptr->prototypeServiceInstances.clear();
    regdata_ptr->bundleServiceInstance.clear();
    // increment the reference count, since "d->reference" was used originally
    // to keep d alive.
    regdata_ptr->reference = nullptr;
    regdata_ptr->unregistering = false;
  }
}

bool ServiceRegistrationBase::operator<(const ServiceRegistrationBase& o) const
{
  if (this == &o || regdata_ptr == o.regdata_ptr)
    return false;

  if ((!regdata_ptr && !o.regdata_ptr) || !o.regdata_ptr)
    return false;
  if (!regdata_ptr)
    return true;

  ServiceReferenceBase sr1;
  ServiceReferenceBase sr2;
  {
    regdata_ptr->Lock(), sr1 = regdata_ptr->reference;
    o.regdata_ptr->Lock(), sr2 = o.regdata_ptr->reference;
  }
  return sr1 < sr2;
}

bool ServiceRegistrationBase::operator==(
  const ServiceRegistrationBase& registration) const
{
  return regdata_ptr == registration.regdata_ptr;
}

ServiceRegistrationBase& ServiceRegistrationBase::operator=(
  const ServiceRegistrationBase& registration)
{
  regdata_ptr = registration.regdata_ptr;
  return *this;
}

ServiceRegistrationBase& ServiceRegistrationBase::operator=(
  ServiceRegistrationBase&& registration)
{
  std::swap(regdata_ptr, registration.regdata_ptr);
  return *this;
}

std::ostream& operator<<(std::ostream& os, const ServiceRegistrationBase&)
{
  return os << "cppmicroservices::ServiceRegistrationBase object";
}
}
