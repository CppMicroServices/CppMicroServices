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

#include "usModuleHooks_p.h"

#include "usModuleEventHook.h"
#include "usModuleFindHook.h"
#include "usCoreModuleContext_p.h"
#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usServiceReferenceBasePrivate.h"

US_BEGIN_NAMESPACE

ModuleHooks::ModuleHooks(CoreModuleContext* ctx)
  : coreCtx(ctx)
{
}

Module* ModuleHooks::FilterModule(const ModuleContext* mc, Module* module) const
{
  if(module == NULL)
  {
    return NULL;
  }

  std::vector<ServiceRegistrationBase> srl;
  coreCtx->services.Get(us_service_interface_iid<ModuleFindHook>(), srl);
  if (srl.empty())
  {
    return module;
  }
  else
  {
    std::vector<Module*> ml;
    ml.push_back(module);
    this->FilterModules(mc, ml);
    return ml.empty() ? NULL : module;
  }
}

void ModuleHooks::FilterModules(const ModuleContext* mc, std::vector<Module*>& modules) const
{
  std::vector<ServiceRegistrationBase> srl;
  coreCtx->services.Get(us_service_interface_iid<ModuleFindHook>(), srl);
  ShrinkableVector<Module*> filtered(modules);

  std::sort(srl.begin(), srl.end());
  for (std::vector<ServiceRegistrationBase>::reverse_iterator srBaseIter = srl.rbegin(), srBaseEnd = srl.rend();
       srBaseIter != srBaseEnd; ++srBaseIter)
  {
    ServiceReference<ModuleFindHook> sr = srBaseIter->GetReference();
    ModuleFindHook* const fh = reinterpret_cast<ModuleFindHook*>(sr.d->GetService(GetModuleContext()->GetModule()));
    if (fh != NULL)
    {
      try
      {
        fh->Find(mc, filtered);
      }
      catch (const std::exception& e)
      {
        US_WARN << "Failed to call Module FindHook  #" << sr.GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                << ": " << e.what();
      }
      catch (...)
      {
        US_WARN << "Failed to call Module FindHook  #" << sr.GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                << ": unknown exception type";
      }
    }
  }
}

void ModuleHooks::FilterModuleEventReceivers(const ModuleEvent& evt,
                                             ServiceListeners::ModuleListenerMap& moduleListeners)
{
  std::vector<ServiceRegistrationBase> eventHooks;
  coreCtx->services.Get(us_service_interface_iid<ModuleEventHook>(), eventHooks);

  {
    MutexLock lock(coreCtx->listeners.moduleListenerMapMutex);
    moduleListeners = coreCtx->listeners.moduleListenerMap;
  }

  if(!eventHooks.empty())
  {
    std::vector<ModuleContext*> moduleContexts;
    for (ServiceListeners::ModuleListenerMap::iterator le = moduleListeners.begin(),
         leEnd = moduleListeners.end(); le != leEnd; ++le)
    {
      moduleContexts.push_back(le->first);
    }
    std::sort(moduleContexts.begin(), moduleContexts.end());
    moduleContexts.erase(std::unique(moduleContexts.begin(), moduleContexts.end()), moduleContexts.end());

    const std::size_t unfilteredSize = moduleContexts.size();
    ShrinkableVector<ModuleContext*> filtered(moduleContexts);

    std::sort(eventHooks.begin(), eventHooks.end());
    for (std::vector<ServiceRegistrationBase>::reverse_iterator iter = eventHooks.rbegin(),
         iterEnd = eventHooks.rend(); iter != iterEnd; ++iter)
    {
      ServiceReference<ModuleEventHook> sr;
      try
      {
        sr = iter->GetReference();
      }
      catch (const std::logic_error& e)
      {
        US_WARN << "Failed to get event hook service reference: " << e.what();
        continue;
      }

      ModuleEventHook* eh = reinterpret_cast<ModuleEventHook*>(sr.d->GetService(GetModuleContext()->GetModule()));
      if (eh != NULL)
      {
        try
        {
          eh->Event(evt, filtered);
        }
        catch (const std::exception& e)
        {
          US_WARN << "Failed to call Module EventHook #" << sr.GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                  << ": " << e.what();
        }
        catch (...)
        {
          US_WARN << "Failed to call Module EventHook #" << sr.GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                  << ": unknown exception type";
        }
      }
    }

    if (unfilteredSize != moduleContexts.size())
    {
      for (ServiceListeners::ModuleListenerMap::iterator le = moduleListeners.begin();
           le != moduleListeners.end();)
      {
        if(std::find(moduleContexts.begin(), moduleContexts.end(), le->first) == moduleContexts.end())
        {
          moduleListeners.erase(le++);
        }
        else
        {
          ++le;
        }
      }
    }
  }
}

US_END_NAMESPACE
