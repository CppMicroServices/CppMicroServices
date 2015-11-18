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

namespace us {

ServiceReferenceBasePrivate::ServiceReferenceBasePrivate(ServiceRegistrationBasePrivate* reg)
  : ref(1), registration(reg)
{
  if(registration) ++registration->ref;
}

ServiceReferenceBasePrivate::~ServiceReferenceBasePrivate()
{
  if (registration && !--registration->ref)
    delete registration;
}

InterfaceMapConstPtr ServiceReferenceBasePrivate::GetServiceFromFactory(std::shared_ptr<Bundle> bundle,
                                                                        const std::shared_ptr<ServiceFactory>& factory,
                                                                        bool isBundleScope)
{
  assert(factory && "Factory service pointer is NULL");
  InterfaceMapConstPtr s;
  try
  {
    InterfaceMapConstPtr smap = factory->GetService(bundle,
                                                    ServiceRegistrationBase(registration));
    if (!smap || smap->empty())
    {
      US_WARN << "ServiceFactory produced null";
      return smap;
    }
    const std::vector<std::string>& classes =
        ref_any_cast<std::vector<std::string> >(registration->properties.Value(ServiceConstants::OBJECTCLASS()));
    for (std::vector<std::string>::const_iterator i = classes.begin();
         i != classes.end(); ++i)
    {
      if (smap->find(*i) == smap->end() && *i != "org.cppmicroservices.factory")
      {
        US_WARN << "ServiceFactory produced an object "
                   "that did not implement: " << (*i);
        return nullptr;
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
    s.reset();
  }
  return s;
}

InterfaceMapConstPtr ServiceReferenceBasePrivate::GetPrototypeService(const std::shared_ptr<Bundle>& bundle)
{
  InterfaceMapConstPtr s;
  {
    typedef decltype(registration->propsLock) T; // gcc 4.6 workaround
    T::Lock l(registration->propsLock);
    if (registration->available)
    {
      std::shared_ptr<ServiceFactory> factory = std::static_pointer_cast<ServiceFactory>(
                                                  registration->GetService("org.cppmicroservices.factory"));
      s = GetServiceFromFactory(bundle, factory, false);
    }
  }
  return s;
}

std::shared_ptr<void> ServiceReferenceBasePrivate::GetService(const std::shared_ptr<Bundle>& bundle)
{
  std::shared_ptr<void> s;
  {
    typedef decltype(registration->propsLock) T; // gcc 4.6 workaround
    T::Lock l(registration->propsLock);
    if (registration->available)
    {
      std::shared_ptr<ServiceFactory> serviceFactory = std::static_pointer_cast<ServiceFactory>(
                                                         registration->GetService("org.cppmicroservices.factory"));

      const int count = registration->dependents[bundle];
      if (count == 0)
      {
        if (serviceFactory)
        {
          const InterfaceMapConstPtr im = GetServiceFromFactory(bundle, serviceFactory, true);
          s = im->find(interfaceId)->second;
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
          s = registration->bundleServiceInstance[bundle]->at(interfaceId);
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

InterfaceMapConstPtr ServiceReferenceBasePrivate::GetServiceInterfaceMap(const std::shared_ptr<Bundle>& bundle)
{
  InterfaceMapConstPtr s;
  {
    typedef decltype(registration->propsLock) T; // gcc 4.6 workaround
    T::Lock l(registration->propsLock);
    if (registration->available)
    {
      std::shared_ptr<ServiceFactory> serviceFactory = std::static_pointer_cast<ServiceFactory>(
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

      if (!s->empty())
      {
        registration->dependents[bundle] = count + 1;
      }
    }
  }
  return s;
}

bool ServiceReferenceBasePrivate::UngetPrototypeService(const std::shared_ptr<Bundle>& bundle, const InterfaceMapConstPtr& service)
{
  if (!service)
  {
    return false;
  }
  
  typedef decltype(registration->propsLock) T; // gcc 4.6 workaround
  T::Lock l(registration->propsLock);

  ServiceRegistrationBasePrivate::BundleToServicesMap::iterator iter =
      registration->prototypeServiceInstances.find(bundle);
  if (iter == registration->prototypeServiceInstances.end())
  {
    return false;
  }

  std::list<InterfaceMapConstPtr>& prototypeServiceMaps = iter->second;

  for (std::list<InterfaceMapConstPtr>::iterator imIter = prototypeServiceMaps.begin();
       imIter != prototypeServiceMaps.end(); ++imIter)
  {
    // compare the contents of the map
    if (service.get() == (*imIter).get())
    {
      try
      {
        std::shared_ptr<ServiceFactory> sf = std::static_pointer_cast<ServiceFactory>(
                                               registration->GetService("org.cppmicroservices.factory"));
        sf->UngetService(bundle, ServiceRegistrationBase(registration), *imIter);
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

bool ServiceReferenceBasePrivate::UngetService(const std::shared_ptr<Bundle>& bundle, bool checkRefCounter)
{
  typedef decltype(registration->propsLock) T; // gcc 4.6 workaround
  T::Lock l(registration->propsLock);
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
    InterfaceMapConstPtr sfi = registration->bundleServiceInstance[bundle];
    registration->bundleServiceInstance.erase(bundle);
    if (sfi && !sfi->empty())
    {
      try
      {
        std::shared_ptr<ServiceFactory> sf = std::static_pointer_cast<ServiceFactory>(
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
    typedef decltype(registration->propsLock) T; // gcc 4.6 workaround
    T::Lock l(registration->propsLock);
    return registration->properties.Value(key);
  }
  else
  {
    return registration->properties.Value(key);
  }
}

bool ServiceReferenceBasePrivate::IsConvertibleTo(const std::string& interfaceId) const
{
  return registration ? registration->service->find(interfaceId) != registration->service->end() : false;
}

}
