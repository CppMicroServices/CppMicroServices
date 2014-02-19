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

#include "usServiceRegistrationBase.h"
#include "usServiceRegistrationBasePrivate.h"
#include "usServiceListenerEntry_p.h"
#include "usServiceRegistry_p.h"
#include "usServiceFactory.h"

#include "usModulePrivate.h"
#include "usCoreModuleContext_p.h"

#include <stdexcept>

US_BEGIN_NAMESPACE

ServiceRegistrationBase::ServiceRegistrationBase()
  : d(0)
{

}

ServiceRegistrationBase::ServiceRegistrationBase(const ServiceRegistrationBase& reg)
  : d(reg.d)
{
  if (d) d->ref.Ref();
}

ServiceRegistrationBase::ServiceRegistrationBase(ServiceRegistrationBasePrivate* registrationPrivate)
  : d(registrationPrivate)
{
  if (d) d->ref.Ref();
}

ServiceRegistrationBase::ServiceRegistrationBase(ModulePrivate* module, const InterfaceMap& service,
                                                 const ServicePropertiesImpl& props)
  : d(new ServiceRegistrationBasePrivate(module, service, props))
{

}

ServiceRegistrationBase::operator bool_type() const
{
  return d != NULL ? &ServiceRegistrationBase::d : NULL;
}

ServiceRegistrationBase& ServiceRegistrationBase::operator=(int null)
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

ServiceRegistrationBase::~ServiceRegistrationBase()
{
  if (d && !d->ref.Deref())
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

  MutexLock lock(d->eventLock);

  ServiceEvent modifiedEndMatchEvent(ServiceEvent::MODIFIED_ENDMATCH, d->reference);
  ServiceListeners::ServiceListenerEntries before;
  // TBD, optimize the locking of services
  {
    //MutexLock lock2(d->module->coreCtx->globalFwLock);

    if (d->available)
    {
      // NYI! Optimize the MODIFIED_ENDMATCH code
      int old_rank = 0;
      int new_rank = 0;

      std::vector<std::string> classes;
      {
        MutexLock lock3(d->propsLock);

        {
          const Any& any = d->properties.Value(ServiceConstants::SERVICE_RANKING());
          if (any.Type() == typeid(int)) old_rank = any_cast<int>(any);
        }

        d->module->coreCtx->listeners.GetMatchingServiceListeners(modifiedEndMatchEvent, before, false);
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
        d->module->coreCtx->services.UpdateServiceRegistrationOrder(*this, classes);
      }
    }
    else
    {
      throw std::logic_error("Service is unregistered");
    }
  }
  ServiceEvent modifiedEvent(ServiceEvent::MODIFIED, d->reference);
  ServiceListeners::ServiceListenerEntries matchingListeners;
  d->module->coreCtx->listeners.GetMatchingServiceListeners(modifiedEvent, matchingListeners);
  d->module->coreCtx->listeners.ServiceChanged(matchingListeners,
                                               modifiedEvent,
                                               before);

  d->module->coreCtx->listeners.ServiceChanged(before,
                                               modifiedEndMatchEvent);
}

void ServiceRegistrationBase::Unregister()
{
  if (!d) throw std::logic_error("ServiceRegistrationBase object invalid");

  if (d->unregistering) return; // Silently ignore redundant unregistration.
  {
    MutexLock lock(d->eventLock);
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
    ServiceEvent unregisteringEvent(ServiceEvent::UNREGISTERING, d->reference);
    d->module->coreCtx->listeners.GetMatchingServiceListeners(unregisteringEvent, listeners);
    d->module->coreCtx->listeners.ServiceChanged(
          listeners,
          unregisteringEvent);
  }

  {
    MutexLock lock(d->eventLock);
    {
      MutexLock lock2(d->propsLock);
      d->available = false;
      InterfaceMap::const_iterator factoryIter = d->service.find("org.cppmicroservices.factory");
      if (d->module && factoryIter != d->service.end())
      {
        ServiceFactory* serviceFactory = reinterpret_cast<ServiceFactory*>(factoryIter->second);
        ServiceRegistrationBasePrivate::ModuleToServicesMap::const_iterator end = d->prototypeServiceInstances.end();

        // unget all prototype services
        for (ServiceRegistrationBasePrivate::ModuleToServicesMap::const_iterator i = d->prototypeServiceInstances.begin();
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

        // unget module scope services
        ServiceRegistrationBasePrivate::ModuleToServiceMap::const_iterator moduleEnd = d->moduleServiceInstance.end();
        for (ServiceRegistrationBasePrivate::ModuleToServiceMap::const_iterator i = d->moduleServiceInstance.begin();
             i != moduleEnd; ++i)
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
      d->module = 0;
      d->dependents.clear();
      d->service.clear();
      d->prototypeServiceInstances.clear();
      d->moduleServiceInstance.clear();
      // increment the reference count, since "d->reference" was used originally
      // to keep d alive.
      d->ref.Ref();
      d->reference = 0;
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
  if (d) d->ref.Ref();

  if (curr_d && !curr_d->ref.Deref())
    delete curr_d;

  return *this;
}

US_END_NAMESPACE
