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

#include <usConfig.h>
#ifdef US_ENABLE_SERVICE_FACTORY_SUPPORT
#include US_BASECLASS_HEADER
#endif

#include "usServiceRegistration.h"
#include "usServiceRegistrationPrivate.h"
#include "usServiceListenerEntry_p.h"
#include "usServiceRegistry_p.h"
#include "usServiceFactory.h"

#include "usModulePrivate.h"
#include "usCoreModuleContext_p.h"

#include <stdexcept>

US_BEGIN_NAMESPACE

typedef ServiceRegistrationPrivate::MutexLocker MutexLocker;

ServiceRegistration::ServiceRegistration()
  : d(0)
{

}

ServiceRegistration::ServiceRegistration(const ServiceRegistration& reg)
  : d(reg.d)
{
  if (d) d->ref.Ref();
}

ServiceRegistration::ServiceRegistration(ServiceRegistrationPrivate* registrationPrivate)
  : d(registrationPrivate)
{
  if (d) d->ref.Ref();
}

ServiceRegistration::ServiceRegistration(ModulePrivate* module, US_BASECLASS_NAME* service,
                                         const ServiceProperties& props)
  : d(new ServiceRegistrationPrivate(module, service, props))
{

}

ServiceRegistration::operator bool() const
{
  return d != 0;
}

ServiceRegistration& ServiceRegistration::operator=(int null)
{
  if (null == 0)
  {
    if (d && !d->ref.Deref())
    {
      delete d;
    }
    d = 0;
  }
  return *this;
}

ServiceRegistration::~ServiceRegistration()
{
  if (d && !d->ref.Deref())
    delete d;
}

ServiceReference ServiceRegistration::GetReference() const
{
  if (!d) throw std::logic_error("ServiceRegistration object invalid");
  if (!d->available) throw std::logic_error("Service is unregistered");

  return d->reference;
}

void ServiceRegistration::SetProperties(const ServiceProperties& props)
{
  if (!d) throw std::logic_error("ServiceRegistration object invalid");

  MutexLocker lock(d->eventLock);

  ServiceListeners::ServiceListenerEntries before;
  // TBD, optimize the locking of services
  {
    //MutexLocker lock2(d->module->coreCtx->globalFwLock);
    MutexLocker lock3(d->propsLock);

    if (d->available)
    {
      // NYI! Optimize the MODIFIED_ENDMATCH code
      int old_rank = any_cast<int>(d->properties[ServiceConstants::SERVICE_RANKING()]);
      d->module->coreCtx->listeners.GetMatchingServiceListeners(d->reference, before, false);
      const std::list<std::string>& classes = ref_any_cast<std::list<std::string> >(d->properties[ServiceConstants::OBJECTCLASS()]);
      long int sid = any_cast<long int>(d->properties[ServiceConstants::SERVICE_ID()]);
      d->properties = ServiceRegistry::CreateServiceProperties(props, classes, sid);
      int new_rank = any_cast<int>(d->properties[ServiceConstants::SERVICE_RANKING()]);
      if (old_rank != new_rank)
      {
        d->module->coreCtx->services.UpdateServiceRegistrationOrder(*this, classes);
      }
    }
    else
    {
      throw std::logic_error("Service is unregistered");
    }
  }
  ServiceListeners::ServiceListenerEntries matchingListeners;
  d->module->coreCtx->listeners.GetMatchingServiceListeners(d->reference, matchingListeners);
  d->module->coreCtx->listeners.ServiceChanged(matchingListeners,
                                               ServiceEvent(ServiceEvent::MODIFIED, d->reference),
                                               before);

  d->module->coreCtx->listeners.ServiceChanged(before,
                                               ServiceEvent(ServiceEvent::MODIFIED_ENDMATCH, d->reference));
}

void ServiceRegistration::Unregister()
{
  if (!d) throw std::logic_error("ServiceRegistration object invalid");

  if (d->unregistering) return; // Silently ignore redundant unregistration.
  {
    MutexLocker lock(d->eventLock);
    if (d->unregistering) return;
    d->unregistering = true;

    if (d->available)
    {
      if (d->module)
      {
        d->module->coreCtx->services.RemoveServiceRegistration(*this);
      }
    }
    else
    {
      throw std::logic_error("Service is unregistered");
    }
  }

  if (d->module)
  {
    ServiceListeners::ServiceListenerEntries listeners;
    d->module->coreCtx->listeners.GetMatchingServiceListeners(d->reference, listeners);
     d->module->coreCtx->listeners.ServiceChanged(
         listeners,
         ServiceEvent(ServiceEvent::UNREGISTERING, d->reference));
  }

  {
    MutexLocker lock(d->eventLock);
    {
      MutexLocker lock2(d->propsLock);
      d->available = false;
      #ifdef US_ENABLE_SERVICE_FACTORY_SUPPORT
      if (d->module)
      {
        ServiceRegistrationPrivate::ModuleToServicesMap::const_iterator end = d->serviceInstances.end();
        for (ServiceRegistrationPrivate::ModuleToServicesMap::const_iterator i = d->serviceInstances.begin();
             i != end; ++i)
        {
          US_BASECLASS_NAME* obj = i->second;
          try
          {
            // NYI, don't call inside lock
            dynamic_cast<ServiceFactory*>(d->service)->UngetService(i->first,
                                                                    *this,
                                                                    obj);
          }
          catch (const std::exception& /*ue*/)
          {
            US_WARN << "ServiceFactory UngetService implementation threw an exception";
          }
        }
      }
      #endif
      d->module = 0;
      d->dependents.clear();
      d->service = 0;
      d->serviceInstances.clear();
      d->reference = 0;
      d->unregistering = false;
    }
  }
}

bool ServiceRegistration::operator<(const ServiceRegistration& o) const
{
  if (!d) return true;
  return d->reference <(o.d->reference);
}

bool ServiceRegistration::operator==(const ServiceRegistration& registration) const
{
  return d == registration.d;
}

ServiceRegistration& ServiceRegistration::operator=(const ServiceRegistration& registration)
{
  ServiceRegistrationPrivate* curr_d = d;
  d = registration.d;
  if (d) d->ref.Ref();

  if (curr_d && !curr_d->ref.Deref())
    delete curr_d;

  return *this;
}

US_END_NAMESPACE
