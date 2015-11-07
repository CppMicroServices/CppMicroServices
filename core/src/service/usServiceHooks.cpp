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

#include "usServiceHooks_p.h"

#include "usGetBundleContext.h"
#include "usCoreBundleContext_p.h"
#include "usServiceEventListenerHook.h"
#include "usServiceFindHook.h"
#include "usServiceListenerHook.h"
#include "usServiceReferenceBasePrivate.h"
#include "usListenerFunctors.h"

namespace us {

ServiceHooks::ServiceHooks(CoreBundleContext* coreCtx)
  : coreCtx(coreCtx)
  , listenerHookTracker()
  , bOpen(false)
{
}

ServiceHooks::~ServiceHooks()
{
  this->Close();
}

ServiceHooks::TrackedType ServiceHooks::AddingService(const ServiceReferenceType& reference)
{
  ServiceListenerHook* lh = GetBundleContext()->GetService(reference);
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
  GetBundleContext()->UngetService(reference);
}

void ServiceHooks::Open()
{
  {
    auto l = this->Lock(); US_UNUSED(l);
    listenerHookTracker.reset(new ServiceTracker<ServiceListenerHook>(GetBundleContext(), this));
  }
  listenerHookTracker->Open();

  bOpen = true;
}

void ServiceHooks::Close()
{
  auto l = this->Lock(); US_UNUSED(l);
  if (listenerHookTracker)
  {
    listenerHookTracker->Close();
    listenerHookTracker.reset();
  }

  bOpen = false;
}

bool ServiceHooks::IsOpen() const
{
  return bOpen;
}

void ServiceHooks::FilterServiceReferences(BundleContext* mc, const std::string& service,
                                           const std::string& filter, std::vector<ServiceReferenceBase>& refs)
{
  std::vector<ServiceRegistrationBase> srl;
  coreCtx->services.Get_unlocked(us_service_interface_iid<ServiceFindHook>(), srl);
  if (!srl.empty())
  {
    ShrinkableVector<ServiceReferenceBase> filtered(refs);

    std::sort(srl.begin(), srl.end());
    for (auto fhrIter = srl.rbegin(), fhrEnd = srl.rend(); fhrIter != fhrEnd; ++fhrIter)
    {
      ServiceReference<ServiceFindHook> sr = fhrIter->GetReference();
      ServiceFindHook* const fh = reinterpret_cast<ServiceFindHook*>(sr.d.load()->GetService(GetBundleContext()->GetBundle().get()));
      if (fh != nullptr)
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
  coreCtx->services.Get(us_service_interface_iid<ServiceEventListenerHook>(), eventListenerHooks);
  if (!eventListenerHooks.empty())
  {
    std::sort(eventListenerHooks.begin(), eventListenerHooks.end());
    std::map<BundleContext*, std::vector<ServiceListenerHook::ListenerInfo> > listeners;
    for (auto& sle : receivers)
    {
      listeners[sle.GetBundleContext()].push_back(sle);
    }

    std::map<BundleContext*, ShrinkableVector<ServiceListenerHook::ListenerInfo> > shrinkableListeners;
    for (auto& l : listeners)
    {
      shrinkableListeners.insert(std::make_pair(l.first, ShrinkableVector<ServiceListenerHook::ListenerInfo>(l.second)));
    }

    ShrinkableMap<BundleContext*, ShrinkableVector<ServiceListenerHook::ListenerInfo> > filtered(shrinkableListeners);

    for(auto sriIter = eventListenerHooks.rbegin(), sriEnd = eventListenerHooks.rend(); sriIter != sriEnd; ++sriIter)
    {
      ServiceReference<ServiceEventListenerHook> sr = sriIter->GetReference();
      ServiceEventListenerHook* elh = reinterpret_cast<ServiceEventListenerHook*>(sr.d.load()->GetService(GetBundleContext()->GetBundle().get()));
      if(elh != nullptr)
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
    for(auto l : listeners)
    {
      receivers.insert(l.second.begin(), l.second.end());
    }
  }
}

void ServiceHooks::HandleServiceListenerReg(const ServiceListenerEntry& sle)
{
  if(!IsOpen() || listenerHookTracker->Size() == 0)
  {
    return;
  }

  auto srl = listenerHookTracker->GetServiceReferences();
  if (!srl.empty())
  {
    std::sort(srl.begin(), srl.end());

    std::vector<ServiceListenerHook::ListenerInfo> set;
    set.push_back(sle);
    for (auto srIter = srl.rbegin(), srEnd = srl.rend(); srIter != srEnd; ++srIter)
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

  auto srl = listenerHookTracker->GetServiceReferences();
  if (!srl.empty())
  {
    std::vector<ServiceListenerHook::ListenerInfo> lis;
    for (auto& sle : set)
    {
      lis.push_back(sle);
    }

    std::sort(srl.begin(), srl.end());
    for (auto srIter = srl.rbegin(), srEnd = srl.rend(); srIter != srEnd; ++srIter)
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

}
