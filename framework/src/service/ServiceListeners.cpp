/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

#include "cppmicroservices/GlobalConfig.h"

US_MSVC_PUSH_DISABLE_WARNING(4180) // qualifier applied to function type has no meaning; ignored

#include <cassert>

#include "ServiceListeners.h"

#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/ListenerFunctors.h"

#include "BundleContextPrivate.h"
#include "BundlePrivate.h"
#include "CoreBundleContext.h"
#include "Properties.h"
#include "ServiceReferenceBasePrivate.h"
#include "Utils.h"

namespace cppmicroservices {

/**
 * Predicate functions used for comparison when adding and removing listeners.
 * These are legacy functions to support backwards compatibility.
 * These will be removed when the only way to remove listeners is through tokens.
 * (when the removal of listeners through supplying names of callables is removed)
 */
auto BundleListenerCompareListenerData = [](const BundleListener& l1, void* d1,
                                            const std::pair<ListenerTokenId,
                                                            ServiceListeners::BundleListenerEntry>& p2)
{
  return d1 == std::get<1>(p2.second) &&
    l1.target<void(const BundleEvent&)>() == std::get<0>(p2.second).target<void(const BundleEvent&)>();
};

auto FrameworkListenerCompareListenerData = [](const FrameworkListener& l1, void* d1,
                                               const std::pair<ListenerTokenId,
                                                               ServiceListeners::FrameworkListenerEntry>& p2)
{
  return d1 == std::get<1>(p2.second) &&
    l1.target<void(const FrameworkEvent&)>() == std::get<0>(p2.second).target<void(const FrameworkEvent&)>();
};

ServiceListeners::ServiceListeners(CoreBundleContext* coreCtx)
  : coreCtx(coreCtx), listenerId(0)
{
  hashedServiceKeys.push_back(Constants::OBJECTCLASS);
  hashedServiceKeys.push_back(Constants::SERVICE_ID);
}

void ServiceListeners::Clear()
{
  bundleListenerMap.Lock(), bundleListenerMap.value.clear();
  {
    auto l = this->Lock(); US_UNUSED(l);
    serviceSet.clear();
    hashedServiceKeys.clear();
    complicatedListeners.clear();
    cache[0].clear();
    cache[1].clear();
  }

  frameworkListenerMap.Lock(), frameworkListenerMap.value.clear();
}

ListenerToken ServiceListeners::MakeListenerToken()
{
  return ListenerToken(++listenerId);
}

ListenerToken ServiceListeners::AddServiceListener(const std::shared_ptr<BundleContextPrivate>& context, const ServiceListener& listener,
                                                   void* data, const std::string& filter)
{
  // The following condition is true only if the listener is a non-static member function.
  // In that case, if we already have a listener with the same ServiceListener and data, we remove that.
  // Effectively, this replaces the existing listener with the new listener if it has the same ServiceListener
  // and data.
  //
  // Note: Only the "data" part is used to compare for sameness. This is the address of the object
  // which has the non-static member function as the listener.
  // The "listener" part comparison always returns "true", because std::function objects aren't
  // equality comparable and the exact type must be known for the std::function::target<> function to not
  // return a nullptr. The exact type is not known because std::function erases the type.
  if (data != nullptr)
  {
    RemoveServiceListener(context, listener, data);
  }

  auto token = MakeListenerToken();
  ServiceListenerEntry sle(context, listener, data, token.Id(), filter);
  {
    auto l = this->Lock(); US_UNUSED(l);
    serviceSet.insert(sle);
    CheckSimple_unlocked(sle);
  }
  coreCtx->serviceHooks.HandleServiceListenerReg(sle);
  return token;
}

// Depending on whether RemoveServiceListener is called with a ListenerTokenId
// or a std::pair<listener, data>, one of the following two overloaded predicate
// functions is returned to RemoveServiceListener.
static std::function<bool(const ServiceListenerEntry&)>
GetUnaryPredicate(const std::shared_ptr<BundleContextPrivate>& context,
                  const std::pair<const ServiceListener&, void*>& listener_data)
{
  auto& listener = listener_data.first;
  auto& data = listener_data.second;
  return [&context, &listener, &data](const ServiceListenerEntry& entry) {
    return entry.Contains(context, listener, data);
  };
}

static std::function<bool(const ServiceListenerEntry&)>
GetUnaryPredicate(const std::shared_ptr<BundleContextPrivate>& context,
                  const ListenerTokenId& tokenId)
{
  return [&context, &tokenId](const ServiceListenerEntry& entry) {
    return entry.Contains(context, tokenId);
  };
}

template <class T>
void ServiceListeners::RemoveServiceListener(const std::shared_ptr<BundleContextPrivate>& context, const T& listenerprops)
{
  ServiceListenerEntry sle;
  {
    auto l = this->Lock(); US_UNUSED(l);
    auto pred = GetUnaryPredicate(context, listenerprops);
    auto it = std::find_if(serviceSet.begin(), serviceSet.end(), pred);
    if (it != serviceSet.end())
    {
      sle = *it;
      it->SetRemoved(true);
      RemoveFromCache_unlocked(*it);
      serviceSet.erase(it);
    }
  }
  if (!sle.IsNull())
  {
    coreCtx->serviceHooks.HandleServiceListenerUnreg(sle);
  }
}

void ServiceListeners::RemoveServiceListener(const std::shared_ptr<BundleContextPrivate>& context, const ServiceListener& listener,
  void* data)
{
  auto listener_data = std::pair<const ServiceListener&, void*>(listener, data);
  RemoveServiceListener(context, listener_data);
}

ListenerToken ServiceListeners::AddBundleListener(const std::shared_ptr<BundleContextPrivate>& context, const BundleListener& listener, void* data)
{
  auto token = MakeListenerToken();

  auto l = bundleListenerMap.Lock(); US_UNUSED(l);
  auto& listeners = bundleListenerMap.value[context];
  auto ret = listeners.emplace(std::make_pair(token.Id(), std::make_tuple(listener, data)));
  assert(ret.second);
  return token;
}

/**
 * Called by the deprecated RemoveBundleListener(name_of_callable)
 */
void ServiceListeners::RemoveBundleListener(const std::shared_ptr<BundleContextPrivate>& context, const BundleListener& listener, void* data)
{
  auto l = bundleListenerMap.Lock(); US_UNUSED(l);
  auto& listeners = bundleListenerMap.value[context];
  auto it = std::find_if(listeners.begin(), listeners.end(), std::bind(BundleListenerCompareListenerData, listener, data, std::placeholders::_1));
  if (it != listeners.end())
  {
      listeners.erase(it);
  }
}

ListenerToken ServiceListeners::AddFrameworkListener(const std::shared_ptr<BundleContextPrivate>& context, const FrameworkListener& listener, void* data)
{
  auto token = MakeListenerToken();

  auto l = frameworkListenerMap.Lock(); US_UNUSED(l);
  auto& listeners = frameworkListenerMap.value[context];
  auto ret = listeners.emplace(std::make_pair(token.Id(), std::make_tuple(listener, data)));
  assert(ret.second);
  return token;
}

/**
 * Called by the deprecated RemoveFrameworkListener(name_of_callable)
 */
void ServiceListeners::RemoveFrameworkListener(const std::shared_ptr<BundleContextPrivate>& context, const FrameworkListener& listener, void* data)
{
  auto l = frameworkListenerMap.Lock(); US_UNUSED(l);
  auto& listeners = frameworkListenerMap.value[context];
  auto it = std::find_if(listeners.begin(), listeners.end(), std::bind(FrameworkListenerCompareListenerData, listener, data, std::placeholders::_1));
  if (it != listeners.end())
  {
    listeners.erase(it);
  }
}

/**
 * Called by RemoveListener(listenerToken)
 * Iterates over the set of FrameworkListeners, BundleListeners and ServiceListeners to find the tokenId and
 * remove the listener corresponding to the tokenId.
 */
void ServiceListeners::RemoveListener(const std::shared_ptr<BundleContextPrivate>& context, ListenerToken token)
{
  if (!token)
  {
    return;
  }

  auto tokenId = token.Id();
  {
    auto l = frameworkListenerMap.Lock(); US_UNUSED(l);
    auto& listeners = frameworkListenerMap.value[context];
    listeners.erase(tokenId);
  }
  {
    auto l = bundleListenerMap.Lock(); US_UNUSED(l);
    auto& listeners = bundleListenerMap.value[context];
    listeners.erase(tokenId);
  }
  RemoveServiceListener(context, tokenId);
}

void ServiceListeners::SendFrameworkEvent(const FrameworkEvent& evt)
{
  // avoid deadlocks, race conditions and other undefined behavior
  // by using a local snapshot of all listeners.
  // A lock shouldn't be held while calling into user code (e.g. callbacks).
  FrameworkListenerMap listener_snapshot;
  {
    auto l = frameworkListenerMap.Lock(); US_UNUSED(l);
    listener_snapshot = frameworkListenerMap.value;
  }

  for (auto& listeners : listener_snapshot)
  {
    for (auto& listener : listeners.second)
    {
      try
      {
        std::get<0>(listener.second)(evt);
      }
      catch (...)
      {
        // do not send a FrameworkEvent as that could cause a deadlock or an infinite loop.
        // Instead, log to the internal logger
        // @todo send this to the LogService instead when its supported.
        DIAG_LOG(*coreCtx->sink) << "A Framework Listener threw an exception: " << GetLastExceptionStr() << "\n";
      }
    }
  }
}

void ServiceListeners::BundleChanged(const BundleEvent& evt)
{
  BundleListenerMap filteredBundleListeners;
  coreCtx->bundleHooks.FilterBundleEventReceivers(evt, filteredBundleListeners);

  for(auto& bundleListeners : filteredBundleListeners)
  {
    for (auto& bundleListener : bundleListeners.second)
    {
      try
      {
        std::get<0>(bundleListener.second)(evt);
      }
      catch (...)
      {
        SendFrameworkEvent(FrameworkEvent(
            FrameworkEvent::Type::FRAMEWORK_ERROR,
            MakeBundle(bundleListeners.first->bundle->shared_from_this()),
            std::string("Bundle listener threw an exception"),
            std::current_exception()));
      }
    }
  }
}

void ServiceListeners::RemoveAllListeners(const std::shared_ptr<BundleContextPrivate>& context)
{
  {
    auto l = this->Lock(); US_UNUSED(l);
    for (ServiceListenerEntries::iterator it = serviceSet.begin();
         it != serviceSet.end(); )
    {

      if (GetPrivate(it->GetBundleContext()) == context)
      {
        RemoveFromCache_unlocked(*it);
        serviceSet.erase(it++);
      }
      else
      {
        ++it;
      }
    }
  }

  {
    auto l = bundleListenerMap.Lock(); US_UNUSED(l);
    bundleListenerMap.value.erase(context);
  }

  {
    auto l = frameworkListenerMap.Lock(); US_UNUSED(l);
    frameworkListenerMap.value.erase(context);
  }
}

void ServiceListeners::HooksBundleStopped(const std::shared_ptr<BundleContextPrivate>& context)
{
  std::vector<ServiceListenerEntry> entries;
  {
    auto l = this->Lock(); US_UNUSED(l);
    for (auto& sle : serviceSet)
    {
      if (sle.GetBundleContext() == MakeBundleContext(context))
      {
        entries.push_back(sle);
      }
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
    for (auto& l : receivers)
    {
      matchBefore.erase(l);
    }
  }

  for (auto& l : receivers)
  {
    if (!l.IsRemoved())
    {
      try
      {
        ++n;
        l.CallDelegate(evt);
      }
      catch (...)
      {
        std::string message("Service listener in " + l.GetBundleContext().GetBundle().GetSymbolicName() + " threw an exception!");
        SendFrameworkEvent(FrameworkEvent(
            FrameworkEvent::Type::FRAMEWORK_ERROR,
            l.GetBundleContext().GetBundle(),
            message,
            std::current_exception()));
      }
    }
  }

}

void ServiceListeners::GetMatchingServiceListeners(const ServiceEvent& evt, ServiceListenerEntries& set)
{
  // Filter the original set of listeners
  ServiceListenerEntries receivers = (this->Lock(), serviceSet);
  // This must not be called with any locks held
  coreCtx->serviceHooks.FilterServiceEventReceivers(evt, receivers);

  // Get a copy of the service reference and keep it until we are
  // done with its properties.
  auto ref = evt.GetServiceReference();
  auto props = ref.d.load()->GetProperties();

  {
    auto l = this->Lock(); US_UNUSED(l);
    // Check complicated or empty listener filters
    for (auto& sse : complicatedListeners)
    {
      if (receivers.count(sse) == 0) continue;
      const LDAPExpr& ldapExpr = sse.GetLDAPExpr();
      if (ldapExpr.IsNull() ||
          ldapExpr.Evaluate(props, false))
      {
        set.insert(sse);
      }
    }

  // Check the cache
    const auto c = any_cast<std::vector<std::string>>(props->Value_unlocked(Constants::OBJECTCLASS));
    for (auto& objClass : c)
    {
      AddToSet_unlocked(set, receivers, OBJECTCLASS_IX, objClass);
    }

    long service_id = any_cast<long>(props->Value_unlocked(Constants::SERVICE_ID));
    AddToSet_unlocked(set, receivers, SERVICE_ID_IX, cppmicroservices::ToString((service_id)));
  }
}

std::vector<ServiceListenerHook::ListenerInfo> ServiceListeners::GetListenerInfoCollection() const
{
  auto l = this->Lock(); US_UNUSED(l);
  std::vector<ServiceListenerHook::ListenerInfo> result;
  result.reserve(serviceSet.size());
  for (auto info : serviceSet)
  {
    result.push_back(info);
  }
  return result;
}

void ServiceListeners::RemoveFromCache_unlocked(const ServiceListenerEntry& sle)
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

void ServiceListeners::CheckSimple_unlocked(const ServiceListenerEntry& sle)
{
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
       complicatedListeners.push_back(sle);
     }
   }
 }

void ServiceListeners::AddToSet_unlocked(ServiceListenerEntries& set,
                                const ServiceListenerEntries& receivers,
                                int cache_ix, const std::string& val)
{
  std::list<ServiceListenerEntry>& l = cache[cache_ix][val];
  if (!l.empty())
  {

    for (std::list<ServiceListenerEntry>::const_iterator entry = l.begin();
         entry != l.end(); ++entry)
    {
      if (receivers.count(*entry))
      {
        set.insert(*entry);
      }
    }
  }
}

}

US_MSVC_POP_WARNING
