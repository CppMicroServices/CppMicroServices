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

#include "usServiceHooks_p.h"

#include "usGetModuleContext.h"
#include "usCoreModuleContext_p.h"
#include "usServiceEventListenerHook.h"
#include "usServiceFindHook.h"
#include "usServiceListenerHook.h"
#include "usServiceReferenceBasePrivate.h"

US_BEGIN_NAMESPACE

ServiceHooks::ServiceHooks(CoreModuleContext* coreCtx)
  : coreCtx(coreCtx)
  , listenerHookTracker(NULL)
  , bOpen(false)
{
}

ServiceHooks::~ServiceHooks()
{
  this->Close();
}

ServiceHooks::TrackedType ServiceHooks::AddingService(const ServiceReferenceType& reference)
{
  ServiceListenerHook* lh = GetModuleContext()->GetService(reference);
  try
  {
    lh->Added(coreCtx->listeners.GetListenerInfoCollection());
  }
  catch (const std::exception& e)
  {
    US_WARN << "Failed to call listener hook  #" << reference.GetProperty(ServiceConstants::SERVICE_ID()).ToString()
               << ": " << e.what();
  }
  catch (...)
  {
    US_WARN << "Failed to call listener hook  #" << reference.GetProperty(ServiceConstants::SERVICE_ID()).ToString()
               << ": unknown exception type";
  }
  return lh;
}

void ServiceHooks::ModifiedService(const ServiceReferenceType& /*reference*/, TrackedType /*service*/)
{
  // noop
}

void ServiceHooks::RemovedService(const ServiceReferenceType& reference, TrackedType /*service*/)
{
  GetModuleContext()->UngetService(reference);
}

void ServiceHooks::Open()
{
  US_UNUSED(Lock(this));

  listenerHookTracker = new ServiceTracker<ServiceListenerHook>(GetModuleContext(), this);
  listenerHookTracker->Open();

  bOpen = true;
}

void ServiceHooks::Close()
{
  US_UNUSED(Lock(this));
  if (listenerHookTracker)
  {
    listenerHookTracker->Close();
    delete listenerHookTracker;
    listenerHookTracker = NULL;
  }

  bOpen = false;
}

bool ServiceHooks::IsOpen() const
{
  US_UNUSED(Lock(this));
  return bOpen;
}

void ServiceHooks::FilterServiceReferences(ModuleContext* mc, const std::string& service,
                                           const std::string& filter, std::vector<ServiceReferenceBase>& refs)
{
  std::vector<ServiceRegistrationBase> srl;
  coreCtx->services.Get_unlocked(us_service_interface_iid<ServiceFindHook>(), srl);
  if (!srl.empty())
  {
    ShrinkableVector<ServiceReferenceBase> filtered(refs);

    std::sort(srl.begin(), srl.end());
    for (std::vector<ServiceRegistrationBase>::reverse_iterator fhrIter = srl.rbegin(), fhrEnd = srl.rend();
         fhrIter != fhrEnd; ++fhrIter)
    {
      ServiceReference<ServiceFindHook> sr = fhrIter->GetReference();
      ServiceFindHook* const fh = reinterpret_cast<ServiceFindHook*>(sr.d->GetService(GetModuleContext()->GetModule()));
      if (fh != NULL)
      {
        try
        {
          fh->Find(mc, service, filter, filtered);
        }
        catch (const std::exception& e)
        {
          US_WARN << "Failed to call find hook  #" << sr.GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                  << ": " << e.what();
        }
        catch (...)
        {
          US_WARN << "Failed to call find hook  #" << sr.GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                  << ": unknown exception type";
        }
      }
    }
  }
}

void ServiceHooks::FilterServiceEventReceivers(const ServiceEvent& evt,
                                               ServiceListeners::ServiceListenerEntries& receivers)
{
  std::vector<ServiceRegistrationBase> eventListenerHooks;
  coreCtx->services.Get_unlocked(us_service_interface_iid<ServiceEventListenerHook>(), eventListenerHooks);
  if (!eventListenerHooks.empty())
  {
    std::sort(eventListenerHooks.begin(), eventListenerHooks.end());
    std::map<ModuleContext*, std::vector<ServiceListenerHook::ListenerInfo> > listeners;
    for (ServiceListeners::ServiceListenerEntries::iterator sleIter = receivers.begin(),
         sleEnd = receivers.end(); sleIter != sleEnd; ++sleIter)
    {
      listeners[sleIter->GetModuleContext()].push_back(*sleIter);
    }

    std::map<ModuleContext*, ShrinkableVector<ServiceListenerHook::ListenerInfo> > shrinkableListeners;
    for (std::map<ModuleContext*, std::vector<ServiceListenerHook::ListenerInfo> >::iterator iter = listeners.begin(),
         iterEnd = listeners.end(); iter != iterEnd; ++iter)
    {
      shrinkableListeners.insert(std::make_pair(iter->first, ShrinkableVector<ServiceListenerHook::ListenerInfo>(iter->second)));
    }

    ShrinkableMap<ModuleContext*, ShrinkableVector<ServiceListenerHook::ListenerInfo> > filtered(shrinkableListeners);

    for(std::vector<ServiceRegistrationBase>::reverse_iterator sriIter = eventListenerHooks.rbegin(),
        sriEnd = eventListenerHooks.rend(); sriIter != sriEnd; ++sriIter)
    {
      ServiceReference<ServiceEventListenerHook> sr = sriIter->GetReference();
      ServiceEventListenerHook* elh = reinterpret_cast<ServiceEventListenerHook*>(sr.d->GetService(GetModuleContext()->GetModule()));
      if(elh != NULL)
      {
        try
        {
          elh->Event(evt, filtered);
        }
        catch(const std::exception& e)
        {
          US_WARN << "Failed to call event hook  #" << sr.GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                  << ": " << e.what();
        }
        catch(...)
        {
          US_WARN << "Failed to call event hook  #" << sr.GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                  << ": unknown exception type";
        }
      }
    }
    receivers.clear();
    for(std::map<ModuleContext*, std::vector<ServiceListenerHook::ListenerInfo> >::iterator iter = listeners.begin(),
        iterEnd = listeners.end(); iter != iterEnd; ++iter)
    {
      receivers.insert(iter->second.begin(), iter->second.end());
    }
  }
}

void ServiceHooks::HandleServiceListenerReg(const ServiceListenerEntry& sle)
{
  if(!IsOpen() || listenerHookTracker->Size() == 0)
  {
    return;
  }

  std::vector<ServiceReference<ServiceListenerHook> > srl
      = listenerHookTracker->GetServiceReferences();

  if (!srl.empty())
  {
    std::sort(srl.begin(), srl.end());

    std::vector<ServiceListenerHook::ListenerInfo> set;
    set.push_back(sle);
    for (std::vector<ServiceReference<ServiceListenerHook> >::reverse_iterator srIter = srl.rbegin(),
         srEnd = srl.rend(); srIter != srEnd; ++srIter)
    {
      ServiceListenerHook* lh = listenerHookTracker->GetService(*srIter);
      try
      {
        lh->Added(set);
      }
      catch (const std::exception& e)
      {
        US_WARN << "Failed to call listener hook #" << srIter->GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                << ": " << e.what();
      }
      catch (...)
      {
        US_WARN << "Failed to call listener hook #" << srIter->GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                << ": unknown exception";
      }
    }
  }
}

void ServiceHooks::HandleServiceListenerUnreg(const ServiceListenerEntry& sle)
{
  if(IsOpen())
  {
    std::vector<ServiceListenerEntry> set;
    set.push_back(sle);
    HandleServiceListenerUnreg(set);
  }
}

void ServiceHooks::HandleServiceListenerUnreg(const std::vector<ServiceListenerEntry>& set)
{
  if(!IsOpen() || listenerHookTracker->Size() == 0)
  {
    return;
  }

  std::vector<ServiceReference<ServiceListenerHook> > srl
      = listenerHookTracker->GetServiceReferences();

  if (!srl.empty())
  {
    std::vector<ServiceListenerHook::ListenerInfo> lis;
    for (std::vector<ServiceListenerEntry>::const_iterator sleIter = set.begin(),
         sleEnd = set.end(); sleIter != sleEnd; ++sleIter)
    {
      lis.push_back(*sleIter);
    }

    std::sort(srl.begin(), srl.end());
    for (std::vector<ServiceReference<ServiceListenerHook> >::reverse_iterator srIter = srl.rbegin(),
         srEnd = srl.rend(); srIter != srEnd; ++srIter)
    {
      ServiceListenerHook* const lh = listenerHookTracker->GetService(*srIter);
      try
      {
        lh->Removed(lis);
      }
      catch (const std::exception& e)
      {
        US_WARN << "Failed to call listener hook #" << srIter->GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                << ": " << e.what();
      }
      catch (...)
      {
        US_WARN << "Failed to call listener hook #" << srIter->GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                << ": unknown exception type";
      }
    }
  }
}

US_END_NAMESPACE
