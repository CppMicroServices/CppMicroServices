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

#include <iterator>
#include <stdexcept>
#include <cassert>

#include "usServiceRegistry_p.h"
#include "usServiceFactory.h"
#include "usPrototypeServiceFactory.h"
#include "usServiceRegistry_p.h"
#include "usServiceRegistrationBasePrivate.h"
#include "usModulePrivate.h"
#include "usCoreModuleContext_p.h"


US_BEGIN_NAMESPACE

ServicePropertiesImpl ServiceRegistry::CreateServiceProperties(const ServiceProperties& in,
                                                               const std::vector<std::string>& classes,
                                                               bool isFactory, bool isPrototypeFactory,
                                                               long sid)
{
  static long nextServiceID = 1;
  ServiceProperties props(in);

  if (!classes.empty())
  {
    props.insert(std::make_pair(ServiceConstants::OBJECTCLASS(), classes));
  }

  props.insert(std::make_pair(ServiceConstants::SERVICE_ID(), sid != -1 ? sid : nextServiceID++));

  if (isPrototypeFactory)
  {
    props.insert(std::make_pair(ServiceConstants::SERVICE_SCOPE(), ServiceConstants::SCOPE_PROTOTYPE()));
  }
  else if (isFactory)
  {
    props.insert(std::make_pair(ServiceConstants::SERVICE_SCOPE(), ServiceConstants::SCOPE_MODULE()));
  }
  else
  {
    props.insert(std::make_pair(ServiceConstants::SERVICE_SCOPE(), ServiceConstants::SCOPE_SINGLETON()));
  }

  return ServicePropertiesImpl(props);
}

ServiceRegistry::ServiceRegistry(CoreModuleContext* coreCtx)
  : core(coreCtx)
{

}

ServiceRegistry::~ServiceRegistry()
{
  Clear();
}

void ServiceRegistry::Clear()
{
  services.clear();
  serviceRegistrations.clear();
  classServices.clear();
  core = 0;
}

ServiceRegistrationBase ServiceRegistry::RegisterService(ModulePrivate* module,
                                                     const InterfaceMap& service,
                                                     const ServiceProperties& properties)
{
  if (service.empty())
  {
    throw std::invalid_argument("Can't register empty InterfaceMap as a service");
  }

  // Check if we got a service factory
  bool isFactory = service.count("org.cppmicroservices.factory") > 0;
  bool isPrototypeFactory = (isFactory ? dynamic_cast<PrototypeServiceFactory*>(reinterpret_cast<ServiceFactory*>(service.find("org.cppmicroservices.factory")->second)) != NULL : false);

  std::vector<std::string> classes;
  // Check if service implements claimed classes and that they exist.
  for (InterfaceMap::const_iterator i = service.begin();
       i != service.end(); ++i)
  {
    if (i->first.empty() || (!isFactory && i->second == NULL))
    {
      throw std::invalid_argument("Can't register as null class");
    }
    classes.push_back(i->first);
  }

  ServiceRegistrationBase res(module, service,
                              CreateServiceProperties(properties, classes, isFactory, isPrototypeFactory));
  {
    MutexLock lock(mutex);
    services.insert(std::make_pair(res, classes));
    serviceRegistrations.push_back(res);
    for (std::vector<std::string>::const_iterator i = classes.begin();
         i != classes.end(); ++i)
    {
      std::vector<ServiceRegistrationBase>& s = classServices[*i];
      std::vector<ServiceRegistrationBase>::iterator ip =
          std::lower_bound(s.begin(), s.end(), res);
      s.insert(ip, res);
    }
  }

  ServiceReferenceBase r = res.GetReference(std::string());
  ServiceListeners::ServiceListenerEntries listeners;
  ServiceEvent registeredEvent(ServiceEvent::REGISTERED, r);
  module->coreCtx->listeners.GetMatchingServiceListeners(registeredEvent, listeners);
  module->coreCtx->listeners.ServiceChanged(listeners,
                                            registeredEvent);
  return res;
}

void ServiceRegistry::UpdateServiceRegistrationOrder(const ServiceRegistrationBase& sr,
                                                     const std::vector<std::string>& classes)
{
  MutexLock lock(mutex);
  for (std::vector<std::string>::const_iterator i = classes.begin();
       i != classes.end(); ++i)
  {
    std::vector<ServiceRegistrationBase>& s = classServices[*i];
    s.erase(std::remove(s.begin(), s.end(), sr), s.end());
    s.insert(std::lower_bound(s.begin(), s.end(), sr), sr);
  }
}

void ServiceRegistry::Get(const std::string& clazz,
                          std::vector<ServiceRegistrationBase>& serviceRegs) const
{
  MutexLock lock(mutex);
  Get_unlocked(clazz, serviceRegs);
}

void ServiceRegistry::Get_unlocked(const std::string& clazz,
                                   std::vector<ServiceRegistrationBase>& serviceRegs) const
{
  MapClassServices::const_iterator i = classServices.find(clazz);
  if (i != classServices.end())
  {
    serviceRegs = i->second;
  }
}

ServiceReferenceBase ServiceRegistry::Get(ModulePrivate* module, const std::string& clazz) const
{
  MutexLock lock(mutex);
  try
  {
    std::vector<ServiceReferenceBase> srs;
    Get_unlocked(clazz, "", module, srs);
    US_DEBUG << "get service ref " << clazz << " for module "
             << module->info.name << " = " << srs.size() << " refs";

    if (!srs.empty())
    {
      return srs.back();
    }
  }
  catch (const std::invalid_argument& )
  { }

  return ServiceReferenceBase();
}

void ServiceRegistry::Get(const std::string& clazz, const std::string& filter,
                          ModulePrivate* module, std::vector<ServiceReferenceBase>& res) const
{
  MutexLock lock(mutex);
  Get_unlocked(clazz, filter, module, res);
}

void ServiceRegistry::Get_unlocked(const std::string& clazz, const std::string& filter,
                          ModulePrivate* module, std::vector<ServiceReferenceBase>& res) const
{
  std::vector<ServiceRegistrationBase>::const_iterator s;
  std::vector<ServiceRegistrationBase>::const_iterator send;
  std::vector<ServiceRegistrationBase> v;
  LDAPExpr ldap;
  if (clazz.empty())
  {
    if (!filter.empty())
    {
      ldap = LDAPExpr(filter);
      LDAPExpr::ObjectClassSet matched;
      if (ldap.GetMatchedObjectClasses(matched))
      {
        v.clear();
        for(LDAPExpr::ObjectClassSet::const_iterator className = matched.begin();
            className != matched.end(); ++className)
        {
          MapClassServices::const_iterator i = classServices.find(*className);
          if (i != classServices.end())
          {
            std::copy(i->second.begin(), i->second.end(), std::back_inserter(v));
          }
        }
        if (!v.empty())
        {
          s = v.begin();
          send = v.end();
        }
        else
        {
          return;
        }
      }
      else
      {
        s = serviceRegistrations.begin();
        send = serviceRegistrations.end();
      }
    }
    else
    {
      s = serviceRegistrations.begin();
      send = serviceRegistrations.end();
    }
  }
  else
  {
    MapClassServices::const_iterator it = classServices.find(clazz);
    if (it != classServices.end())
    {
      s = it->second.begin();
      send = it->second.end();
    }
    else
    {
      return;
    }
    if (!filter.empty())
    {
      ldap = LDAPExpr(filter);
    }
  }

  for (; s != send; ++s)
  {
    ServiceReferenceBase sri = s->GetReference(clazz);

    if (filter.empty() || ldap.Evaluate(s->d->properties, false))
    {
      res.push_back(sri);
    }
  }

  if (!res.empty())
  {
    if (module != NULL)
    {
      core->serviceHooks.FilterServiceReferences(module->moduleContext, clazz, filter, res);
    }
    else
    {
      core->serviceHooks.FilterServiceReferences(NULL, clazz, filter, res);
    }
  }
}

void ServiceRegistry::RemoveServiceRegistration(const ServiceRegistrationBase& sr)
{
  MutexLock lock(mutex);

  assert(sr.d->properties.Value(ServiceConstants::OBJECTCLASS()).Type() == typeid(std::vector<std::string>));
  const std::vector<std::string>& classes = ref_any_cast<std::vector<std::string> >(
        sr.d->properties.Value(ServiceConstants::OBJECTCLASS()));
  services.erase(sr);
  serviceRegistrations.erase(std::remove(serviceRegistrations.begin(), serviceRegistrations.end(), sr),
                             serviceRegistrations.end());
  for (std::vector<std::string>::const_iterator i = classes.begin();
       i != classes.end(); ++i)
  {
    std::vector<ServiceRegistrationBase>& s = classServices[*i];
    if (s.size() > 1)
    {
      s.erase(std::remove(s.begin(), s.end(), sr), s.end());
    }
    else
    {
      classServices.erase(*i);
    }
  }
}

void ServiceRegistry::GetRegisteredByModule(ModulePrivate* p,
                                            std::vector<ServiceRegistrationBase>& res) const
{
  MutexLock lock(mutex);

  for (std::vector<ServiceRegistrationBase>::const_iterator i = serviceRegistrations.begin();
       i != serviceRegistrations.end(); ++i)
  {
    if (i->d->module == p)
    {
      res.push_back(*i);
    }
  }
}

void ServiceRegistry::GetUsedByModule(Module* p,
                                      std::vector<ServiceRegistrationBase>& res) const
{
  MutexLock lock(mutex);

  for (std::vector<ServiceRegistrationBase>::const_iterator i = serviceRegistrations.begin();
       i != serviceRegistrations.end(); ++i)
  {
    if (i->d->IsUsedByModule(p))
    {
      res.push_back(*i);
    }
  }
}

US_END_NAMESPACE
