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

#include "usServiceReferencePrivate.h"

#include "usServiceFactory.h"
#include "usServiceException.h"
#include "usServiceRegistry_p.h"
#include "usServiceRegistrationPrivate.h"

#include "usModule.h"
#include "usModulePrivate.h"

#include <usCoreModuleContext_p.h>

US_BEGIN_NAMESPACE

typedef ServiceRegistrationPrivate::MutexLocker MutexLocker;

ServiceReferencePrivate::ServiceReferencePrivate(ServiceRegistrationPrivate* reg)
  : ref(1), registration(reg)
{
  if(registration) registration->ref.Ref();
}

ServiceReferencePrivate::~ServiceReferencePrivate()
{
  if (registration && !registration->ref.Deref())
    delete registration;
}

US_BASECLASS_NAME* ServiceReferencePrivate::GetService(Module* module)
{
  US_BASECLASS_NAME* s = 0;
  {
    MutexLocker lock(registration->propsLock);
    if (registration->available)
    {
      int count = registration->dependents[module];
      if (count == 0)
      {
        registration->dependents[module] = 1;
        #ifdef US_ENABLE_SERVICE_FACTORY_SUPPORT
        const std::list<std::string>& classes =
            ref_any_cast<std::list<std::string> >(registration->properties[ServiceConstants::OBJECTCLASS()]);
        if (ServiceFactory* serviceFactory = dynamic_cast<ServiceFactory*>(registration->GetService()))
        {
          try
          {
            s = serviceFactory->GetService(module, ServiceRegistration(registration));
          }
          catch (...)
          {
            US_WARN << "ServiceFactory threw an exception";
            return 0;
          }
          if (s == 0) {
            US_WARN << "ServiceFactory produced null";
            return 0;
          }
          for (std::list<std::string>::const_iterator i = classes.begin();
               i != classes.end(); ++i)
          {
            if (!registration->module->coreCtx->services.CheckServiceClass(s, *i))
            {
              US_WARN << "ServiceFactory produced an object "
                           "that did not implement: " << (*i);
              return 0;
            }
          }
          registration->serviceInstances.insert(std::make_pair(module, s));
        }
        else
        #endif
        {
          s = registration->GetService();
        }
      }
      else
      {
        registration->dependents.insert(std::make_pair(module, count + 1));
        #ifdef US_ENABLE_SERVICE_FACTORY_SUPPORT
        if (dynamic_cast<ServiceFactory*>(registration->GetService()))
        {
          s = registration->serviceInstances[module];
        }
        else
        #endif
        {
          s = registration->GetService();
        }
      }
    }
  }
  return s;
}

bool ServiceReferencePrivate::UngetService(Module* module, bool checkRefCounter)
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
    #ifdef US_ENABLE_SERVICE_FACTORY_SUPPORT
    US_BASECLASS_NAME* sfi = registration->serviceInstances[module];
    registration->serviceInstances.erase(module);
    if (sfi != 0)
    {
      try
      {
        dynamic_cast<ServiceFactory*>(
              registration->GetService())->UngetService(module, ServiceRegistration(registration), sfi);
      }
      catch (const std::exception& /*e*/)
      {
        US_WARN << "ServiceFactory threw an exception";
      }
    }
    #endif
    registration->dependents.erase(module);
  }

  return hadReferences;
}

ServiceProperties ServiceReferencePrivate::GetProperties() const
{
  return registration->properties;
}

Any ServiceReferencePrivate::GetProperty(const std::string& key, bool lock) const
{
  if (lock)
  {
    MutexLocker lock(registration->propsLock);
    ServiceProperties::const_iterator iter = registration->properties.find(key);
    if (iter != registration->properties.end())
      return iter->second;
  }
  else
  {
    ServiceProperties::const_iterator iter = registration->properties.find(key);
    if (iter != registration->properties.end())
      return iter->second;
  }
  return Any();
}

US_END_NAMESPACE
