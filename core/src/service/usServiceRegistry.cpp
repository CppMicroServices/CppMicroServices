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

#include <iterator>
#include <stdexcept>
#include <cassert>

#include "usServiceRegistry_p.h"
#include "usServiceFactory.h"
#include "usPrototypeServiceFactory.h"
#include "usServiceRegistry_p.h"
#include "usServiceRegistrationBasePrivate.h"
#include "usBundlePrivate.h"
#include "usCoreBundleContext_p.h"


namespace us {

ServicePropertiesImpl ServiceRegistry::CreateServiceProperties(const ServiceProperties& in,
                                                               const std::vector<std::string>& classes,
                                                               bool isFactory, bool isPrototypeFactory,
                                                               long sid)
{
  static std::atomic<long> nextServiceID(1);
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
    props.insert(std::make_pair(ServiceConstants::SERVICE_SCOPE(), ServiceConstants::SCOPE_BUNDLE()));
  }
  else
  {
    props.insert(std::make_pair(ServiceConstants::SERVICE_SCOPE(), ServiceConstants::SCOPE_SINGLETON()));
  }

  return ServicePropertiesImpl(props);
}

ServiceRegistry::ServiceRegistry(CoreBundleContext* coreCtx)
  : core(coreCtx)
{

}

ServiceRegistrationBase ServiceRegistry::RegisterService(BundlePrivate* bundle,
                                                     const InterfaceMap& service,
                                                     const ServiceProperties& properties)
{
  if (service.empty())
  {
    throw std::invalid_argument("Can't register empty InterfaceMap as a service");
  }

  // Check if we got a service factory
  bool isFactory = service.count("org.cppmicroservices.factory") > 0;
  bool isPrototypeFactory = (isFactory ? dynamic_cast<PrototypeServiceFactory*>(reinterpret_cast<ServiceFactory*>(service.find("org.cppmicroservices.factory")->second)) != nullptr : false);

  std::vector<std::string> classes;
  // Check if service implements claimed classes and that they exist.
  for (auto i : service)
  {
    if (i.first.empty() || (!isFactory && i.second == nullptr))
    {
      throw std::invalid_argument("Can't register as null class");
    }
    classes.push_back(i.first);
  }

  ServiceRegistrationBase res(bundle, service,
                              CreateServiceProperties(properties, classes, isFactory, isPrototypeFactory));
  {
    auto l = this->Lock();
    services.insert(std::make_pair(res, classes));
    serviceRegistrations.push_back(res);
    for (auto clazz : classes)
    {
      std::vector<ServiceRegistrationBase>& s = classServices[clazz];
      std::vector<ServiceRegistrationBase>::iterator ip =
          std::lower_bound(s.begin(), s.end(), res);
      s.insert(ip, res);
    }
  }

  ServiceReferenceBase r = res.GetReference(std::string());
  ServiceListeners::ServiceListenerEntries listeners;
  ServiceEvent registeredEvent(ServiceEvent::REGISTERED, r);
  bundle->coreCtx->listeners.GetMatchingServiceListeners(registeredEvent, listeners);
  bundle->coreCtx->listeners.ServiceChanged(listeners,
                                            registeredEvent);
  return res;
}

void ServiceRegistry::UpdateServiceRegistrationOrder(const ServiceRegistrationBase& sr,
                                                     const std::vector<std::string>& classes)
{
  auto l = this->Lock();
  for (auto& clazz : classes)
  {
    std::vector<ServiceRegistrationBase>& s = classServices[clazz];
    s.erase(std::remove(s.begin(), s.end(), sr), s.end());
    s.insert(std::lower_bound(s.begin(), s.end(), sr), sr);
  }
}

void ServiceRegistry::Get(const std::string& clazz,
                          std::vector<ServiceRegistrationBase>& serviceRegs) const
{
  this->Lock(), Get_unlocked(clazz, serviceRegs);
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

ServiceReferenceBase ServiceRegistry::Get(BundlePrivate* bundle, const std::string& clazz) const
{
  auto l = this->Lock();
  try
  {
    std::vector<ServiceReferenceBase> srs;
    Get_unlocked(clazz, "", bundle, srs);
    US_DEBUG << "get service ref " << clazz << " for module "
             << bundle->info.name << " = " << srs.size() << " refs";

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
                          BundlePrivate* bundle, std::vector<ServiceReferenceBase>& res) const
{
  this->Lock(), Get_unlocked(clazz, filter, bundle, res);
}

void ServiceRegistry::Get_unlocked(const std::string& clazz, const std::string& filter,
                          BundlePrivate* bundle, std::vector<ServiceReferenceBase>& res) const
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
        for(auto className : matched)
        {
          auto i = classServices.find(className);
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

    if (filter.empty() || ldap.Evaluate(ServicePropertiesHandle(s->d->properties, true), false))
    {
      res.push_back(sri);
    }
  }

  if (!res.empty())
  {
    if (bundle != nullptr)
    {
      core->serviceHooks.FilterServiceReferences(bundle->bundleContext, clazz, filter, res);
    }
    else
    {
      core->serviceHooks.FilterServiceReferences(nullptr, clazz, filter, res);
    }
  }
}

void ServiceRegistry::RemoveServiceRegistration(const ServiceRegistrationBase& sr)
{
  auto l = this->Lock();
  RemoveServiceRegistration_unlocked(sr);
}

void ServiceRegistry::RemoveServiceRegistration_unlocked(const ServiceRegistrationBase& sr)
{
  std::vector<std::string> classes;
  {
    auto l2 = sr.d->properties.Lock();
    assert(sr.d->properties.Value_unlocked(ServiceConstants::OBJECTCLASS()).Type() == typeid(std::vector<std::string>));
    classes = ref_any_cast<std::vector<std::string> >(
          sr.d->properties.Value_unlocked(ServiceConstants::OBJECTCLASS()));
  }
  services.erase(sr);
  serviceRegistrations.erase(std::remove(serviceRegistrations.begin(), serviceRegistrations.end(), sr),
                             serviceRegistrations.end());
  for (auto& clazz : classes)
  {
    std::vector<ServiceRegistrationBase>& s = classServices[clazz];
    if (s.size() > 1)
    {
      s.erase(std::remove(s.begin(), s.end(), sr), s.end());
    }
    else
    {
      classServices.erase(clazz);
    }
  }
}

void ServiceRegistry::GetRegisteredByBundle(BundlePrivate* p,
                                            std::vector<ServiceRegistrationBase>& res) const
{
  auto l = this->Lock();

  for (auto& sr : serviceRegistrations)
  {
    if (sr.d->bundle == p)
    {
      res.push_back(sr);
    }
  }
}

void ServiceRegistry::GetUsedByBundle(Bundle* p,
                                      std::vector<ServiceRegistrationBase>& res) const
{
  auto l = this->Lock();

  for (auto& sr : serviceRegistrations)
  {
    if (sr.d->IsUsedByBundle(p))
    {
      res.push_back(sr);
    }
  }
}

}
