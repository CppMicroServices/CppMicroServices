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
#include "usServiceReferencePrivate.h"

#include "usModule.h"

//#include <usLogMacros.h>

US_BEGIN_NAMESPACE

const int ServiceListeners::OBJECTCLASS_IX = 0;
const int ServiceListeners::SERVICE_ID_IX = 1;

ServiceListeners::ServiceListeners()
{
  hashedServiceKeys.push_back(ServiceConstants::OBJECTCLASS());
  hashedServiceKeys.push_back(ServiceConstants::SERVICE_ID());
}

void ServiceListeners::AddServiceListener(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& listener,
                                          void* data, const std::string& filter)
{
  MutexLocker lock(mutex);

  ServiceListenerEntry sle(mc->GetModule(), listener, data, filter);
  if (serviceSet.find(sle) != serviceSet.end())
  {
    RemoveServiceListener_unlocked(sle);
  }
  serviceSet.insert(sle);
  CheckSimple(sle);
}

void ServiceListeners::RemoveServiceListener(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& listener,
                                             void* data)
{
  ServiceListenerEntry entryToRemove(mc->GetModule(), listener, data);

  MutexLocker lock(mutex);
  RemoveServiceListener_unlocked(entryToRemove);
}

void ServiceListeners::RemoveServiceListener_unlocked(const ServiceListenerEntry& entryToRemove)
{
  for (ServiceListenerEntries::iterator it = serviceSet.begin();
       it != serviceSet.end(); ++it)
  {
    if (it->operator ==(entryToRemove))
    {
      it->SetRemoved(true);
      RemoveFromCache(*it);
      serviceSet.erase(it);
      break;
    }
  }
}

void ServiceListeners::AddModuleListener(ModuleContext* mc, const ModuleListener& listener, void* data)
{
  MutexLocker lock(moduleListenerMapMutex);
  ModuleListenerMap::value_type::second_type& listeners = moduleListenerMap[mc];
  if (std::find_if(listeners.begin(), listeners.end(), std::bind1st(ModuleListenerCompare(), std::make_pair(listener, data))) == listeners.end())
  {
    listeners.push_back(std::make_pair(listener, data));
  }
}

void ServiceListeners::RemoveModuleListener(ModuleContext* mc, const ModuleListener& listener, void* data)
{
  MutexLocker lock(moduleListenerMapMutex);
  moduleListenerMap[mc].remove_if(std::bind1st(ModuleListenerCompare(), std::make_pair(listener, data)));
}

void ServiceListeners::ModuleChanged(const ModuleEvent& evt)
{
  MutexLocker lock(moduleListenerMapMutex);
  for(ModuleListenerMap::iterator i = moduleListenerMap.begin();
      i != moduleListenerMap.end(); ++i)
  {
    for(std::list<std::pair<ModuleListener, void*> >::iterator j = i->second.begin();
        j != i->second.end(); ++j)
    {
      (j->first)(evt);
    }
  }
}

void ServiceListeners::RemoveAllListeners(ModuleContext* mc)
{
  {
    MutexLocker lock(mutex);
    for (ServiceListenerEntries::iterator it = serviceSet.begin();
         it != serviceSet.end(); )
    {

      if (it->GetModule() == mc->GetModule())
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
    MutexLocker lock(moduleListenerMapMutex);
    moduleListenerMap.erase(mc);
  }
}

void ServiceListeners::ServiceChanged(const ServiceListenerEntries& receivers,
                                      const ServiceEvent& evt)
{
  ServiceListenerEntries matchBefore;
  ServiceChanged(receivers, evt, matchBefore);
}

void ServiceListeners::ServiceChanged(const ServiceListenerEntries& receivers,
                                      const ServiceEvent& evt,
                                      ServiceListenerEntries& matchBefore)
{
  ServiceReference sr = evt.GetServiceReference();
  int n = 0;

  for (ServiceListenerEntries::const_iterator l = receivers.begin();
       l != receivers.end(); ++l)
  {
    if (!matchBefore.empty())
    {
      matchBefore.erase(*l);
    }

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
            #ifdef US_MODULE_SUPPORT_ENABLED
                << " in " << l->GetModule()->GetName()
            #endif
                << " threw an exception!";
      }
    }
  }

  //US_DEBUG << "Notified " << n << " listeners";
}

void ServiceListeners::GetMatchingServiceListeners(const ServiceReference& sr, ServiceListenerEntries& set,
                                                   bool lockProps)
{
  MutexLocker lock(mutex);

  // Check complicated or empty listener filters
  int n = 0;
  for (std::list<ServiceListenerEntry>::const_iterator sse = complicatedListeners.begin();
       sse != complicatedListeners.end(); ++sse)
  {
    ++n;
    if (sse->GetLDAPExpr().IsNull() ||
        sse->GetLDAPExpr().Evaluate(sr.d->GetProperties(), false))
    {
      set.insert(*sse);
    }
  }

  //US_DEBUG << "Added " << set.size() << " out of " << n
  //         << " listeners with complicated filters";

  // Check the cache
  const std::list<std::string> c(any_cast<std::list<std::string> >
                                 (sr.d->GetProperty(ServiceConstants::OBJECTCLASS(), lockProps)));
  for (std::list<std::string>::const_iterator objClass = c.begin();
       objClass != c.end(); ++objClass)
  {
    AddToSet(set, OBJECTCLASS_IX, *objClass);
  }

  long service_id = any_cast<long>(sr.d->GetProperty(ServiceConstants::SERVICE_ID(), lockProps));
  std::stringstream ss;
  ss << service_id;
  AddToSet(set, SERVICE_ID_IX, ss.str());
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
                                int cache_ix, const std::string& val)
{
  std::list<ServiceListenerEntry>& l = cache[cache_ix][val];
  if (!l.empty())
  {
    //US_DEBUG << hashedServiceKeys[cache_ix] << " matches " << l.size();

    for (std::list<ServiceListenerEntry>::const_iterator entry = l.begin();
         entry != l.end(); ++entry)
    {
      set.insert(*entry);
    }
  }
  else
  {
    //US_DEBUG << hashedServiceKeys[cache_ix] << " matches none";
  }
}

US_END_NAMESPACE
