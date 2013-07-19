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

#include "usServiceReferenceBasePrivate.h"

#include "usServiceFactory.h"
#include "usServiceException.h"
#include "usServiceRegistry_p.h"
#include "usServiceRegistrationBasePrivate.h"

#include "usModule.h"
#include "usModulePrivate.h"

#include <usCoreModuleContext_p.h>

US_BEGIN_NAMESPACE

typedef ServiceRegistrationBasePrivate::MutexLocker MutexLocker;

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

void* ServiceReferenceBasePrivate::GetService(Module* module)
{
  void* s = NULL;
  {
    MutexLocker lock(registration->propsLock);
    if (registration->available)
    {
      int count = registration->dependents[module];
      if (count == 0)
      {
        registration->dependents[module] = 1;
        if (void* factoryPtr = registration->GetService("org.cppmicroservices.factory"))
        {
          ServiceFactory* serviceFactory = reinterpret_cast<ServiceFactory*>(factoryPtr);
          try
          {
            const InterfaceMap smap = serviceFactory->GetService(module,
                                                                        ServiceRegistrationBase(registration));
            if (smap.empty())
            {
              US_WARN << "ServiceFactory produced null";
              return NULL;
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
                return NULL;
              }
            }
            s = smap.find(interfaceId)->second;
            registration->serviceInstances.insert(std::make_pair(module, smap));
          }
          catch (...)
          {
            US_WARN << "ServiceFactory threw an exception";
            return NULL;
          }
        }
        else
        {
          s = registration->GetService(interfaceId);
        }
      }
      else
      {
        registration->dependents.insert(std::make_pair(module, count + 1));
        if (registration->GetService("org.cppmicroservices.factory"))
        {
          s = registration->serviceInstances[module][interfaceId];
        }
        else
        {
          s = registration->GetService(interfaceId);
        }
      }
    }
  }
  return s;
}

bool ServiceReferenceBasePrivate::UngetService(Module* module, bool checkRefCounter)
{
  MutexLocker lock(registration->propsLock);
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
    InterfaceMap sfi = registration->serviceInstances[module];
    registration->serviceInstances.erase(module);
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

  return hadReferences;
}

const ServicePropertiesImpl& ServiceReferenceBasePrivate::GetProperties() const
{
  return registration->properties;
}

Any ServiceReferenceBasePrivate::GetProperty(const std::string& key, bool lock) const
{
  if (lock)
  {
    MutexLocker lock(registration->propsLock);
    return registration->properties.Value(key);
  }
  else
  {
    return registration->properties.Value(key);
  }
}

bool ServiceReferenceBasePrivate::IsConvertibleTo(const std::string& interfaceId) const
{
  return registration->service.find(interfaceId) != registration->service.end();
}

US_END_NAMESPACE
