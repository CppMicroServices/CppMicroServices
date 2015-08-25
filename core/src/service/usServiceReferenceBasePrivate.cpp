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


#include "usServiceReferenceBasePrivate.h"

#include "usServiceFactory.h"
#include "usServiceException.h"
#include "usServiceRegistry_p.h"
#include "usServiceRegistrationBasePrivate.h"

#include "usBundle.h"
#include "usBundlePrivate.h"

#include <usCoreBundleContext_p.h>

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

InterfaceMap ServiceReferenceBasePrivate::GetServiceFromFactory(Bundle* bundle,
                                                                ServiceFactory* factory,
                                                                bool isBundleScope)
{
  assert(factory && "Factory service pointer is NULL");
  InterfaceMap s;
  try
  {
    InterfaceMap smap = factory->GetService(bundle,
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

    if (isBundleScope)
    {
      registration->bundleServiceInstance.insert(std::make_pair(bundle, smap));
    }
    else
    {
      registration->prototypeServiceInstances[bundle].push_back(smap);
    }
  }
  catch (...)
  {
    US_WARN << "ServiceFactory threw an exception";
    s.clear();
  }
  return s;
}

InterfaceMap ServiceReferenceBasePrivate::GetPrototypeService(Bundle* bundle)
{
  InterfaceMap s;
  {
    MutexLock lock(registration->propsLock);
    if (registration->available)
    {
      ServiceFactory* factory = reinterpret_cast<ServiceFactory*>(
            registration->GetService("org.cppmicroservices.factory"));
      s = GetServiceFromFactory(bundle, factory, false);
    }
  }
  return s;
}

void* ServiceReferenceBasePrivate::GetService(Bundle* bundle)
{
  void* s = NULL;
  {
    MutexLock lock(registration->propsLock);
    if (registration->available)
    {
      ServiceFactory* serviceFactory = reinterpret_cast<ServiceFactory*>(
            registration->GetService("org.cppmicroservices.factory"));

      const int count = registration->dependents[bundle];
      if (count == 0)
      {
        if (serviceFactory)
        {
          const InterfaceMap im = GetServiceFromFactory(bundle, serviceFactory, true);
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
          s = registration->bundleServiceInstance[bundle][interfaceId];
        }
        else
        {
          s = registration->GetService(interfaceId);
        }
      }

      if (s)
      {
        registration->dependents[bundle] = count + 1;
      }
    }
  }
  return s;
}

InterfaceMap ServiceReferenceBasePrivate::GetServiceInterfaceMap(Bundle* bundle)
{
  InterfaceMap s;
  {
    MutexLock lock(registration->propsLock);
    if (registration->available)
    {
      ServiceFactory* serviceFactory = reinterpret_cast<ServiceFactory*>(
            registration->GetService("org.cppmicroservices.factory"));

      const int count = registration->dependents[bundle];
      if (count == 0)
      {
        if (serviceFactory)
        {
          s = GetServiceFromFactory(bundle, serviceFactory, true);
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
          s = registration->bundleServiceInstance[bundle];
        }
        else
        {
          s = registration->service;
        }
      }

      if (!s.empty())
      {
        registration->dependents[bundle] = count + 1;
      }
    }
  }
  return s;
}

bool ServiceReferenceBasePrivate::UngetPrototypeService(Bundle* bundle, const InterfaceMap& service)
{
  MutexLock lock(registration->propsLock);

  ServiceRegistrationBasePrivate::BundleToServicesMap::iterator iter =
      registration->prototypeServiceInstances.find(bundle);
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
        sf->UngetService(bundle, ServiceRegistrationBase(registration), service);
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

bool ServiceReferenceBasePrivate::UngetService(Bundle* bundle, bool checkRefCounter)
{
  MutexLock lock(registration->propsLock);
  bool hadReferences = false;
  bool removeService = false;

  int count= registration->dependents[bundle];
  if (count > 0)
  {
    hadReferences = true;
  }

  if(checkRefCounter)
  {
    if (count > 1)
    {
      registration->dependents[bundle] = count - 1;
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
    InterfaceMap sfi = registration->bundleServiceInstance[bundle];
    registration->bundleServiceInstance.erase(bundle);
    if (!sfi.empty())
    {
      try
      {
        ServiceFactory* sf = reinterpret_cast<ServiceFactory*>(
                               registration->GetService("org.cppmicroservices.factory"));
        sf->UngetService(bundle, ServiceRegistrationBase(registration), sfi);
      }
      catch (const std::exception& /*e*/)
      {
        US_WARN << "ServiceFactory threw an exception";
      }
    }
    registration->dependents.erase(bundle);
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
