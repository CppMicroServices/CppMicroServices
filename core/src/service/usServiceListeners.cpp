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

#include "usUtils_p.h"

#include "usServiceListeners_p.h"
#include "usServiceReferenceBasePrivate.h"

#include "usCoreModuleContext_p.h"
#include "usModule.h"
#include "usModuleContext.h"


US_BEGIN_NAMESPACE

const int ServiceListeners::OBJECTCLASS_IX = 0;
const int ServiceListeners::SERVICE_ID_IX = 1;

ServiceListeners::ServiceListeners(CoreModuleContext* coreCtx)
  : coreCtx(coreCtx)
{
  hashedServiceKeys.push_back(ServiceConstants::OBJECTCLASS());
  hashedServiceKeys.push_back(ServiceConstants::SERVICE_ID());
}

void ServiceListeners::AddServiceListener(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& listener,
                                          void* data, const std::string& filter)
{
  US_UNUSED(Lock(this));

  ServiceListenerEntry sle(mc, listener, data, filter);
  RemoveServiceListener_unlocked(sle);

  serviceSet.insert(sle);
  coreCtx->serviceHooks.HandleServiceListenerReg(sle);
  CheckSimple(sle);
}

void ServiceListeners::RemoveServiceListener(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& listener,
                                             void* data)
{
  ServiceListenerEntry entryToRemove(mc, listener, data);

  US_UNUSED(Lock(this));
  RemoveServiceListener_unlocked(entryToRemove);
}

void ServiceListeners::RemoveServiceListener_unlocked(const ServiceListenerEntry& entryToRemove)
{
  ServiceListenerEntries::const_iterator it = serviceSet.find(entryToRemove);
  if (it != serviceSet.end())
  {
    it->SetRemoved(true);
    coreCtx->serviceHooks.HandleServiceListenerUnreg(*it);
    RemoveFromCache(*it);
    serviceSet.erase(it);
  }
}

void ServiceListeners::AddModuleListener(ModuleContext* mc, const ModuleListener& listener, void* data)
{
  MutexLock lock(moduleListenerMapMutex);
  ModuleListenerMap::value_type::second_type& listeners = moduleListenerMap[mc];
  if (std::find_if(listeners.begin(), listeners.end(), std::bind1st(ModuleListenerCompare(), std::make_pair(listener, data))) == listeners.end())
  {
    listeners.push_back(std::make_pair(listener, data));
  }
}

void ServiceListeners::RemoveModuleListener(ModuleContext* mc, const ModuleListener& listener, void* data)
{
  MutexLock lock(moduleListenerMapMutex);
  moduleListenerMap[mc].remove_if(std::bind1st(ModuleListenerCompare(), std::make_pair(listener, data)));
}

void ServiceListeners::ModuleChanged(const ModuleEvent& evt)
{
  ModuleListenerMap filteredModuleListeners;
  coreCtx->moduleHooks.FilterModuleEventReceivers(evt, filteredModuleListeners);

  for(ModuleListenerMap::iterator iter = filteredModuleListeners.begin(), end = filteredModuleListeners.end();
      iter != end; ++iter)
  {
    for (ModuleListenerMap::mapped_type::iterator iter2 = iter->second.begin(), end2 = iter->second.end();
         iter2 != end2; ++iter2)
    {
      try
      {
        (iter2->first)(evt);
      }
      catch (const std::exception& e)
      {
        US_WARN << "Module listener threw an exception: " << e.what();
      }
    }
  }
}

void ServiceListeners::RemoveAllListeners(ModuleContext* mc)
{
  {
    US_UNUSED(Lock(this));
    for (ServiceListenerEntries::iterator it = serviceSet.begin();
         it != serviceSet.end(); )
    {

      if (it->GetModuleContext() == mc)
      {
        RemoveFromCache(*it);
        serviceSet.erase(it++);
      }
      else
      {
        ++it;
      }
    }
  }

  {
    MutexLock lock(moduleListenerMapMutex);
    moduleListenerMap.erase(mc);
  }
}

void ServiceListeners::HooksModuleStopped(ModuleContext* mc)
{
  US_UNUSED(Lock(this));
  std::vector<ServiceListenerEntry> entries;
  for (ServiceListenerEntries::iterator it = serviceSet.begin();
       it != serviceSet.end(); )
  {
    if (it->GetModuleContext() == mc)
    {
      entries.push_back(*it);
    }
  }
  coreCtx->serviceHooks.HandleServiceListenerUnreg(entries);
}

void ServiceListeners::ServiceChanged(ServiceListenerEntries& receivers,
                                      const ServiceEvent& evt)
{
  ServiceListenerEntries matchBefore;
  ServiceChanged(receivers, evt, matchBefore);
}

void ServiceListeners::ServiceChanged(ServiceListenerEntries& receivers,
                                      const ServiceEvent& evt,
                                      ServiceListenerEntries& matchBefore)
{
  int n = 0;

  if (!matchBefore.empty())
  {
    for (ServiceListenerEntries::const_iterator l = receivers.begin();
         l != receivers.end(); ++l)
    {
      matchBefore.erase(*l);
    }
  }

  for (ServiceListenerEntries::const_iterator l = receivers.begin();
       l != receivers.end(); ++l)
  {
    if (!l->IsRemoved())
    {
      try
      {
        ++n;
        l->CallDelegate(evt);
      }
      catch (...)
      {
        US_WARN << "Service listener"
                << " in " << l->GetModuleContext()->GetModule()->GetName()
                << " threw an exception!";
      }
    }
  }

  //US_DEBUG << "Notified " << n << " listeners";
}

void ServiceListeners::GetMatchingServiceListeners(const ServiceEvent& evt, ServiceListenerEntries& set,
                                                   bool lockProps)
{
  US_UNUSED(Lock(this));

  // Filter the original set of listeners
  ServiceListenerEntries receivers = serviceSet;
  coreCtx->serviceHooks.FilterServiceEventReceivers(evt, receivers);

  // Check complicated or empty listener filters
  for (std::list<ServiceListenerEntry>::const_iterator sse = complicatedListeners.begin();
       sse != complicatedListeners.end(); ++sse)
  {
    if (receivers.count(*sse) == 0) continue;
    const LDAPExpr& ldapExpr = sse->GetLDAPExpr();
    if (ldapExpr.IsNull() ||
        ldapExpr.Evaluate(evt.GetServiceReference().d->GetProperties(), false))
    {
      set.insert(*sse);
    }
  }

  //US_DEBUG << "Added " << set.size() << " out of " << n
  //         << " listeners with complicated filters";

  // Check the cache
  const std::vector<std::string> c(any_cast<std::vector<std::string> >
                                 (evt.GetServiceReference().d->GetProperty(ServiceConstants::OBJECTCLASS(), lockProps)));
  for (std::vector<std::string>::const_iterator objClass = c.begin();
       objClass != c.end(); ++objClass)
  {
    AddToSet(set, receivers, OBJECTCLASS_IX, *objClass);
  }

  long service_id = any_cast<long>(evt.GetServiceReference().d->GetProperty(ServiceConstants::SERVICE_ID(), lockProps));
  std::stringstream ss;
  ss << service_id;
  AddToSet(set, receivers, SERVICE_ID_IX, ss.str());
}

std::vector<ServiceListenerHook::ListenerInfo> ServiceListeners::GetListenerInfoCollection() const
{
  US_UNUSED(Lock(this));
  std::vector<ServiceListenerHook::ListenerInfo> result;
  result.reserve(serviceSet.size());
  for (ServiceListenerEntries::const_iterator iter = serviceSet.begin(),
       iterEnd = serviceSet.end(); iter != iterEnd; ++iter)
  {
    result.push_back(*iter);
  }
  return result;
}

void ServiceListeners::RemoveFromCache(const ServiceListenerEntry& sle)
{
  if (!sle.GetLocalCache().empty())
  {
    for (std::size_t i = 0; i < hashedServiceKeys.size(); ++i)
    {
      CacheType& keymap = cache[i];
      std::vector<std::string>& l = sle.GetLocalCache()[i];
      for (std::vector<std::string>::const_iterator it = l.begin();
           it != l.end(); ++it)
      {
        std::list<ServiceListenerEntry>& sles = keymap[*it];
        sles.remove(sle);
        if (sles.empty())
        {
          keymap.erase(*it);
        }
      }
    }
  }
  else
  {
    complicatedListeners.remove(sle);
  }
}

 void ServiceListeners::CheckSimple(const ServiceListenerEntry& sle) {
   if (sle.GetLDAPExpr().IsNull())
   {
     complicatedListeners.push_back(sle);
   }
   else
   {
     LDAPExpr::LocalCache local_cache;
     if (sle.GetLDAPExpr().IsSimple(hashedServiceKeys, local_cache, false))
     {
       sle.GetLocalCache() = local_cache;
       for (std::size_t i = 0; i < hashedServiceKeys.size(); ++i)
       {
         for (std::vector<std::string>::const_iterator it = local_cache[i].begin();
              it != local_cache[i].end(); ++it)
         {
           std::list<ServiceListenerEntry>& sles = cache[i][*it];
           sles.push_back(sle);
         }
       }
     }
     else
     {
       //US_DEBUG << "Too complicated filter: " << sle.GetFilter();
       complicatedListeners.push_back(sle);
     }
   }
 }

void ServiceListeners::AddToSet(ServiceListenerEntries& set,
                                const ServiceListenerEntries& receivers,
                                int cache_ix, const std::string& val)
{
  std::list<ServiceListenerEntry>& l = cache[cache_ix][val];
  if (!l.empty())
  {
    //US_DEBUG << hashedServiceKeys[cache_ix] << " matches " << l.size();

    for (std::list<ServiceListenerEntry>::const_iterator entry = l.begin();
         entry != l.end(); ++entry)
    {
      if (receivers.count(*entry))
      {
        set.insert(*entry);
      }
    }
  }
  else
  {
    //US_DEBUG << hashedServiceKeys[cache_ix] << " matches none";
  }
}

US_END_NAMESPACE
