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


#include "usServiceReferenceBasePrivate.h"

#include "usServiceFactory.h"
#include "usServiceException.h"
#include "usServiceRegistry_p.h"
#include "usServiceRegistrationBasePrivate.h"

#include "usModule.h"
#include "usModulePrivate.h"

#include <usCoreModuleContext_p.h>

#include <cassert>

#ifdef _MSC_VER
#pragma warning(disable:4503) // decorated name length exceeded, name was truncated
#endif

US_BEGIN_NAMESPACE

ServiceReferenceBasePrivate::ServiceReferenceBasePrivate(ServiceRegistrationBasePrivate* reg)
  : ref(1), registration(reg)
{
  if(registration) registration->ref.Ref();
}

ServiceReferenceBasePrivate::~ServiceReferenceBasePrivate()
{
  if (registration && !registration->ref.Deref())
    delete registration;
}

InterfaceMap ServiceReferenceBasePrivate::GetServiceFromFactory(Module* module,
                                                                ServiceFactory* factory,
                                                                bool isModuleScope)
{
  assert(factory && "Factory service pointer is NULL");
  InterfaceMap s;
  try
  {
    InterfaceMap smap = factory->GetService(module,
                                            ServiceRegistrationBase(registration));
    if (smap.empty())
    {
      US_WARN << "ServiceFactory produced null";
      return smap;
    }
    const std::vector<std::string>& classes =
        ref_any_cast<std::vector<std::string> >(registration->properties.Value(ServiceConstants::OBJECTCLASS()));
    for (std::vector<std::string>::const_iterator i = classes.begin();
         i != classes.end(); ++i)
    {
      if (smap.find(*i) == smap.end() && *i != "org.cppmicroservices.factory")
      {
        US_WARN << "ServiceFactory produced an object "
                   "that did not implement: " << (*i);
        smap.clear();
        return smap;
      }
    }
    s = smap;

    if (isModuleScope)
    {
      registration->moduleServiceInstance.insert(std::make_pair(module, smap));
    }
    else
    {
      registration->prototypeServiceInstances[module].push_back(smap);
    }
  }
  catch (...)
  {
    US_WARN << "ServiceFactory threw an exception";
    s.clear();
  }
  return s;
}

InterfaceMap ServiceReferenceBasePrivate::GetPrototypeService(Module* module)
{
  InterfaceMap s;
  {
    MutexLock lock(registration->propsLock);
    if (registration->available)
    {
      ServiceFactory* factory = reinterpret_cast<ServiceFactory*>(
            registration->GetService("org.cppmicroservices.factory"));
      s = GetServiceFromFactory(module, factory, false);
    }
  }
  return s;
}

void* ServiceReferenceBasePrivate::GetService(Module* module)
{
  void* s = NULL;
  {
    MutexLock lock(registration->propsLock);
    if (registration->available)
    {
      ServiceFactory* serviceFactory = reinterpret_cast<ServiceFactory*>(
            registration->GetService("org.cppmicroservices.factory"));

      const int count = registration->dependents[module];
      if (count == 0)
      {
        if (serviceFactory)
        {
          const InterfaceMap im = GetServiceFromFactory(module, serviceFactory, true);
          s = im.find(interfaceId)->second;
        }
        else
        {
          s = registration->GetService(interfaceId);
        }
      }
      else
      {
        if (serviceFactory)
        {
          // return the already produced instance
          s = registration->moduleServiceInstance[module][interfaceId];
        }
        else
        {
          s = registration->GetService(interfaceId);
        }
      }

      if (s)
      {
        registration->dependents[module] = count + 1;
      }
    }
  }
  return s;
}

InterfaceMap ServiceReferenceBasePrivate::GetServiceInterfaceMap(Module* module)
{
  InterfaceMap s;
  {
    MutexLock lock(registration->propsLock);
    if (registration->available)
    {
      ServiceFactory* serviceFactory = reinterpret_cast<ServiceFactory*>(
            registration->GetService("org.cppmicroservices.factory"));

      const int count = registration->dependents[module];
      if (count == 0)
      {
        if (serviceFactory)
        {
          s = GetServiceFromFactory(module, serviceFactory, true);
        }
        else
        {
          s = registration->service;
        }
      }
      else
      {
        if (serviceFactory)
        {
          // return the already produced instance
          s = registration->moduleServiceInstance[module];
        }
        else
        {
          s = registration->service;
        }
      }

      if (!s.empty())
      {
        registration->dependents[module] = count + 1;
      }
    }
  }
  return s;
}

bool ServiceReferenceBasePrivate::UngetPrototypeService(Module* module, const InterfaceMap& service)
{
  MutexLock lock(registration->propsLock);

  ServiceRegistrationBasePrivate::ModuleToServicesMap::iterator iter =
      registration->prototypeServiceInstances.find(module);
  if (iter == registration->prototypeServiceInstances.end())
  {
    return false;
  }

  std::list<InterfaceMap>& prototypeServiceMaps = iter->second;

  for (std::list<InterfaceMap>::iterator imIter = prototypeServiceMaps.begin();
       imIter != prototypeServiceMaps.end(); ++imIter)
  {
    if (service == *imIter)
    {
      try
      {
        ServiceFactory* sf = reinterpret_cast<ServiceFactory*>(
                               registration->GetService("org.cppmicroservices.factory"));
        sf->UngetService(module, ServiceRegistrationBase(registration), service);
      }
      catch (const std::exception& /*e*/)
      {
        US_WARN << "ServiceFactory threw an exception";
      }
      prototypeServiceMaps.erase(imIter);
      if (prototypeServiceMaps.empty())
      {
        registration->prototypeServiceInstances.erase(iter);
      }
      return true;
    }
  }

  return false;
}

bool ServiceReferenceBasePrivate::UngetService(Module* module, bool checkRefCounter)
{
  MutexLock lock(registration->propsLock);
  bool hadReferences = false;
  bool removeService = false;

  int count= registration->dependents[module];
  if (count > 0)
  {
    hadReferences = true;
  }

  if(checkRefCounter)
  {
    if (count > 1)
    {
      registration->dependents[module] = count - 1;
    }
    else if(count == 1)
    {
      removeService = true;
    }
  }
  else
  {
    removeService = true;
  }

  if (removeService)
  {
    InterfaceMap sfi = registration->moduleServiceInstance[module];
    registration->moduleServiceInstance.erase(module);
    if (!sfi.empty())
    {
      try
      {
        ServiceFactory* sf = reinterpret_cast<ServiceFactory*>(
                               registration->GetService("org.cppmicroservices.factory"));
        sf->UngetService(module, ServiceRegistrationBase(registration), sfi);
      }
      catch (const std::exception& /*e*/)
      {
        US_WARN << "ServiceFactory threw an exception";
      }
    }
    registration->dependents.erase(module);
  }

  return hadReferences && removeService;
}

const ServicePropertiesImpl& ServiceReferenceBasePrivate::GetProperties() const
{
  return registration->properties;
}

Any ServiceReferenceBasePrivate::GetProperty(const std::string& key, bool lock) const
{
  if (lock)
  {
    MutexLock lock(registration->propsLock);
    return registration->properties.Value(key);
  }
  else
  {
    return registration->properties.Value(key);
  }
}

bool ServiceReferenceBasePrivate::IsConvertibleTo(const std::string& interfaceId) const
{
  return registration ? registration->service.find(interfaceId) != registration->service.end() : false;
}

US_END_NAMESPACE
