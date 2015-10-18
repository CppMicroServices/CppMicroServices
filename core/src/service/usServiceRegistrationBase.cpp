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

namespace us {

ServiceRegistrationBase::ServiceRegistrationBase()
  : d(0)
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

ServiceRegistrationBase::ServiceRegistrationBase(BundlePrivate* bundle, const InterfaceMap& service,
                                                 const ServicePropertiesImpl& props)
  : d(new ServiceRegistrationBasePrivate(bundle, service, props))
{

}

ServiceRegistrationBase::operator bool_type() const
{
  return d != NULL ? &ServiceRegistrationBase::d : NULL;
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

  ServiceReferenceBase ref = d->reference;
  ref.SetInterfaceId(interfaceId);
  return ref;
}

void ServiceRegistrationBase::SetProperties(const ServiceProperties& props)
{
  if (!d) throw std::logic_error("ServiceRegistrationBase object invalid");

  typedef decltype(d->eventLock) T; // gcc 4.6 workaround
  T::Lock l(d->eventLock);

  ServiceEvent modifiedEndMatchEvent(ServiceEvent::MODIFIED_ENDMATCH, d->reference);
  ServiceListeners::ServiceListenerEntries before;
  // TBD, optimize the locking of services
  {
    //MutexLock lock2(d->bundle->coreCtx->globalFwLock);

    if (d->available)
    {
      // NYI! Optimize the MODIFIED_ENDMATCH code
      int old_rank = 0;
      int new_rank = 0;

      std::vector<std::string> classes;
      {
        typedef decltype(d->propsLock) T; // gcc 4.6 workaround
        T::Lock l2(d->propsLock);

        {
          const Any& any = d->properties.Value(ServiceConstants::SERVICE_RANKING());
          if (any.Type() == typeid(int)) old_rank = any_cast<int>(any);
        }

        d->bundle->coreCtx->listeners.GetMatchingServiceListeners(modifiedEndMatchEvent, before, false);
        classes = ref_any_cast<std::vector<std::string> >(d->properties.Value(ServiceConstants::OBJECTCLASS()));
        long int sid = any_cast<long int>(d->properties.Value(ServiceConstants::SERVICE_ID()));
        d->properties = ServiceRegistry::CreateServiceProperties(props, classes, false, false, sid);

        {
          const Any& any = d->properties.Value(ServiceConstants::SERVICE_RANKING());
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
  }
  ServiceEvent modifiedEvent(ServiceEvent::MODIFIED, d->reference);
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
  {
    typedef decltype(d->eventLock) T; // gcc 4.6 workaround
    T::Lock l(d->eventLock);
    if (d->unregistering) return;
    d->unregistering = true;

    if (d->available)
    {
      if (d->bundle)
      {
        d->bundle->coreCtx->services.RemoveServiceRegistration(*this);
      }
    }
    else
    {
      throw std::logic_error("Service is unregistered");
    }
  }

  if (d->bundle)
  {
    ServiceListeners::ServiceListenerEntries listeners;
    ServiceEvent unregisteringEvent(ServiceEvent::UNREGISTERING, d->reference);
    d->bundle->coreCtx->listeners.GetMatchingServiceListeners(unregisteringEvent, listeners);
    d->bundle->coreCtx->listeners.ServiceChanged(
          listeners,
          unregisteringEvent);
  }

  {
    typedef decltype(d->eventLock) T; // gcc 4.6 workaround
    T::Lock l(d->eventLock);
    {
      typedef decltype(d->propsLock) P; // gcc 4.6 workaround
      P::Lock l2(d->propsLock);
      d->available = false;
      InterfaceMap::const_iterator factoryIter = d->service.find("org.cppmicroservices.factory");
      if (d->bundle && factoryIter != d->service.end())
      {
        ServiceFactory* serviceFactory = reinterpret_cast<ServiceFactory*>(factoryIter->second);
        ServiceRegistrationBasePrivate::BundleToServicesMap::const_iterator end = d->prototypeServiceInstances.end();

        // unget all prototype services
        for (ServiceRegistrationBasePrivate::BundleToServicesMap::const_iterator i = d->prototypeServiceInstances.begin();
             i != end; ++i)
        {
          for (std::list<InterfaceMap>::const_iterator listIter = i->second.begin();
               listIter != i->second.end(); ++listIter)
          {
            const InterfaceMap& service = *listIter;
            try
            {
              // NYI, don't call inside lock
              serviceFactory->UngetService(i->first, *this, service);
            }
            catch (const std::exception& /*ue*/)
            {
              US_WARN << "ServiceFactory UngetService implementation threw an exception";
            }
          }
        }

        // unget bundle scope services
        ServiceRegistrationBasePrivate::BundleToServiceMap::const_iterator bundleEnd = d->bundleServiceInstance.end();
        for (ServiceRegistrationBasePrivate::BundleToServiceMap::const_iterator i = d->bundleServiceInstance.begin();
             i != bundleEnd; ++i)
        {
          try
          {
            // NYI, don't call inside lock
            serviceFactory->UngetService(i->first, *this, i->second);
          }
          catch (const std::exception& /*ue*/)
          {
            US_WARN << "ServiceFactory UngetService implementation threw an exception";
          }
        }
      }
      d->bundle = nullptr;
      d->dependents.clear();
      d->service.clear();
      d->prototypeServiceInstances.clear();
      d->bundleServiceInstance.clear();
      // increment the reference count, since "d->reference" was used originally
      // to keep d alive.
      ++d->ref;
      d->reference = nullptr;
      d->unregistering = false;
    }
  }
}

bool ServiceRegistrationBase::operator<(const ServiceRegistrationBase& o) const
{
  if ((!d && !o.d) || !o.d) return false;
  if (!d) return true;
  return d->reference <(o.d->reference);
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
