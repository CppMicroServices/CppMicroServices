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

#include "ServiceReferenceBasePrivate.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/ServiceException.h"
#include "cppmicroservices/ServiceFactory.h"

#include "BundlePrivate.h"
#include "CoreBundleContext_p.h"
#include "ServiceRegistrationBasePrivate.h"
#include "ServiceRegistry_p.h"

#include <cassert>

US_MSVC_DISABLE_WARNING(4503) // decorated name length exceeded, name was truncated

namespace cppmicroservices {

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

InterfaceMapConstPtr ServiceReferenceBasePrivate::GetServiceFromFactory(
      BundlePrivate* bundle,
      const std::shared_ptr<ServiceFactory>& factory)
{
  assert(factory && "Factory service pointer is nullptr");
  InterfaceMapConstPtr s;
  try
  {
    InterfaceMapConstPtr smap = factory->GetService(MakeBundle(bundle->shared_from_this()),
                                                    ServiceRegistrationBase(registration));
    if (!smap || smap->empty())
    {
      registration->bundle->coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING, MakeBundle(bundle->shared_from_this()), std::string("ServiceFactory returned an empty or nullptr interface map."), std::make_exception_ptr(std::logic_error("ServiceFactory produced null"))));
      return smap;
    }
    std::vector<std::string> classes = (registration->properties.Lock(),
                                        any_cast<std::vector<std::string>>(registration->properties.Value_unlocked(Constants::OBJECTCLASS)));
    for (auto clazz : classes)
    {
      if (smap->find(clazz) == smap->end() && clazz != "org.cppmicroservices.factory")
      {
        std::string message("ServiceFactory produced an object that did not implement: " + clazz);
        registration->bundle->coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING, MakeBundle(bundle->shared_from_this()), message, std::make_exception_ptr(std::logic_error(message.c_str()))));
        return nullptr;
      }
    }
    s = smap;
  }
  catch (...)
  {
    s.reset();
    registration->bundle->coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_ERROR, MakeBundle(bundle->shared_from_this()), std::string("ServiceFactory threw an unknown exception."), std::current_exception()));
  }
  return s;
}

InterfaceMapConstPtr ServiceReferenceBasePrivate::GetPrototypeService(const Bundle& bundle)
{
  InterfaceMapConstPtr s;
  {
    if (registration->available)
    {
      auto factory = std::static_pointer_cast<ServiceFactory>(
                                                  registration->GetService("org.cppmicroservices.factory"));
      s = GetServiceFromFactory(GetPrivate(bundle).get(), factory);
      registration->Lock(), registration->prototypeServiceInstances[GetPrivate(bundle).get()].push_back(s);
    }
  }
  return s;
}

std::shared_ptr<void> ServiceReferenceBasePrivate::GetService(BundlePrivate* bundle)
{
  auto s = ExtractInterface(GetServiceInterfaceMap(bundle), interfaceId);
  if (!s)
  {
    registration->Lock(), --registration->dependents[bundle];
  }
  return s;
}

InterfaceMapConstPtr ServiceReferenceBasePrivate::GetServiceInterfaceMap(BundlePrivate* bundle)
{
  InterfaceMapConstPtr s;
  if (!registration->available) return s;

  std::shared_ptr<ServiceFactory> serviceFactory;
  int count = 0;

  {
    auto l = registration->Lock(); US_UNUSED(l);
    if (!registration->available) return s;
    serviceFactory = std::static_pointer_cast<ServiceFactory>(
          registration->GetService_unlocked("org.cppmicroservices.factory"));
    count = registration->dependents[bundle];
  }

  if (!serviceFactory)
  {
    auto l = registration->Lock(); US_UNUSED(l);
          s = registration->service;
    if (s && !s->empty()) ++registration->dependents[bundle];
  }
  else
  {
    if (count == 0)
    {
      s = GetServiceFromFactory(bundle, serviceFactory);

      auto l = registration->Lock(); US_UNUSED(l);
      if (registration->dependents[bundle] == 0)
      {
        registration->bundleServiceInstance.insert(std::make_pair(bundle, s));
      }
      else
      {
        // There was a race and we now have one instance too much. Return the
        // already produced instance and ignore the additional one.
        s = registration->bundleServiceInstance[bundle];
      }

      if (s && !s->empty()) ++registration->dependents[bundle];
    }
    else
    {
      auto l = registration->Lock(); US_UNUSED(l);
      // return the already produced instance
      s = registration->bundleServiceInstance[bundle];
      if (s && !s->empty()) ++registration->dependents[bundle];
    }
  }
  return s;
}

bool ServiceReferenceBasePrivate::UngetPrototypeService(const std::shared_ptr<BundlePrivate>& bundle, const InterfaceMapConstPtr& service)
{
  std::list<InterfaceMapConstPtr> prototypeServiceMaps;
  std::shared_ptr<ServiceFactory> sf;

  {
    auto l = registration->Lock(); US_UNUSED(l);
    auto iter = registration->prototypeServiceInstances.find(bundle.get());
    if (iter == registration->prototypeServiceInstances.end())
    {
      return false;
    }

    prototypeServiceMaps = iter->second;
    sf = std::static_pointer_cast<ServiceFactory>(
          registration->GetService_unlocked("org.cppmicroservices.factory"));
  }

  if (!sf) return false;

  for (auto imIter = prototypeServiceMaps.begin(); imIter != prototypeServiceMaps.end(); ++imIter)
  {
    // compare the contents of the map
    if (*service.get() == *imIter->get())
    {
      try
      {
        sf->UngetService(MakeBundle(bundle), ServiceRegistrationBase(registration), service);
      }
      catch (...)
      {
        std::string message("ServiceFactory threw an exception");
        registration->bundle->coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING, MakeBundle(bundle->shared_from_this()), message, std::current_exception()));
      }

      auto l = registration->Lock(); US_UNUSED(l);
      auto iter = registration->prototypeServiceInstances.find(bundle.get());
      if (iter == registration->prototypeServiceInstances.end()) return true;

      auto serviceIter = std::find(iter->second.begin(), iter->second.end(), service);
      if (serviceIter != iter->second.end()) iter->second.erase(serviceIter);
      if (iter->second.empty())
      {
        registration->prototypeServiceInstances.erase(iter);
      }
      return true;
    }
  }

  return false;
}

bool ServiceReferenceBasePrivate::UngetService(const std::shared_ptr<BundlePrivate>& bundle, bool checkRefCounter)
{
  bool hadReferences = false;
  bool removeService = false;
  InterfaceMapConstPtr sfi;
  std::shared_ptr<ServiceFactory> sf;

  {
    auto l = registration->Lock(); US_UNUSED(l);
    int count= registration->dependents[bundle.get()];
    if (count > 0)
    {
      hadReferences = true;
    }

    if(checkRefCounter)
    {
      if (count > 1)
      {
        registration->dependents[bundle.get()] = count - 1;
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
      sfi = registration->bundleServiceInstance[bundle.get()];
      if (sfi && !sfi->empty())
      {
        sf = std::static_pointer_cast<ServiceFactory>(
              registration->GetService_unlocked("org.cppmicroservices.factory"));
      }
      registration->bundleServiceInstance.erase(bundle.get());
      registration->dependents.erase(bundle.get());
    }
  }

  if (sf && sfi && !sfi->empty())
  {
    try
    {
      sf->UngetService(MakeBundle(bundle), ServiceRegistrationBase(registration), sfi);
    }
    catch (...)
    {
      std::string message("ServiceFactory threw an exception");
      registration->bundle->coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING, MakeBundle(bundle->shared_from_this()), message, std::current_exception()));
    }
  }

  return hadReferences && removeService;
}

PropertiesHandle ServiceReferenceBasePrivate::GetProperties() const
{
  return PropertiesHandle(registration->properties, true);
}

bool ServiceReferenceBasePrivate::IsConvertibleTo(const std::string& interfaceId) const
{
  return registration ? registration->Lock(), registration->service->find(interfaceId) != registration->service->end() : false;
}

}
