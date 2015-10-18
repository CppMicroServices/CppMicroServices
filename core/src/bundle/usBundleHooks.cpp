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

#include "usBundleHooks_p.h"

#include "usBundleEventHook.h"
#include "usBundleFindHook.h"
#include "usCoreBundleContext_p.h"
#include "usGetBundleContext.h"
#include "usBundleContext.h"
#include "usServiceReferenceBasePrivate.h"

namespace us {

BundleHooks::BundleHooks(CoreBundleContext* ctx)
  : coreCtx(ctx)
{
}

Bundle* BundleHooks::FilterBundle(const BundleContext* mc, Bundle* bundle) const
{
  if(bundle == NULL)
  {
    return NULL;
  }

  std::vector<ServiceRegistrationBase> srl;
  coreCtx->services.Get(us_service_interface_iid<BundleFindHook>(), srl);
  if (srl.empty())
  {
    return bundle;
  }
  else
  {
    std::vector<Bundle*> ml;
    ml.push_back(bundle);
    this->FilterBundles(mc, ml);
    return ml.empty() ? NULL : bundle;
  }
}

void BundleHooks::FilterBundles(const BundleContext* mc, std::vector<Bundle*>& bundles) const
{
  std::vector<ServiceRegistrationBase> srl;
  coreCtx->services.Get(us_service_interface_iid<BundleFindHook>(), srl);
  ShrinkableVector<Bundle*> filtered(bundles);

  std::sort(srl.begin(), srl.end());
  for (std::vector<ServiceRegistrationBase>::reverse_iterator srBaseIter = srl.rbegin(), srBaseEnd = srl.rend();
       srBaseIter != srBaseEnd; ++srBaseIter)
  {
    ServiceReference<BundleFindHook> sr = srBaseIter->GetReference();
    BundleFindHook* const fh = reinterpret_cast<BundleFindHook*>(sr.d->GetService(GetBundleContext()->GetBundle()));
    if (fh != NULL)
    {
      try
      {
        fh->Find(mc, filtered);
      }
      catch (const std::exception& e)
      {
        US_WARN << "Failed to call Bundle FindHook  #" << sr.GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                << ": " << e.what();
      }
      catch (...)
      {
        US_WARN << "Failed to call Bundle FindHook  #" << sr.GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                << ": unknown exception type";
      }
    }
  }
}

void BundleHooks::FilterBundleEventReceivers(const BundleEvent& evt,
                                             ServiceListeners::BundleListenerMap& bundleListeners)
{
  std::vector<ServiceRegistrationBase> eventHooks;
  coreCtx->services.Get(us_service_interface_iid<BundleEventHook>(), eventHooks);

  {
    //typedef decltype(coreCtx->listeners.bundleListenerMap) T;
    typedef MultiThreaded<> T; // gcc 4.6 workaround; the above line leads to a segfault with gcc 4.6
    T::Lock{coreCtx->listeners.bundleListenerMap};
    bundleListeners = coreCtx->listeners.bundleListenerMap.value;
  }

  if(!eventHooks.empty())
  {
    std::vector<BundleContext*> bundleContexts;
    for (ServiceListeners::BundleListenerMap::iterator le = bundleListeners.begin(),
         leEnd = bundleListeners.end(); le != leEnd; ++le)
    {
      bundleContexts.push_back(le->first);
    }
    std::sort(bundleContexts.begin(), bundleContexts.end());
    bundleContexts.erase(std::unique(bundleContexts.begin(), bundleContexts.end()), bundleContexts.end());

    const std::size_t unfilteredSize = bundleContexts.size();
    ShrinkableVector<BundleContext*> filtered(bundleContexts);

    std::sort(eventHooks.begin(), eventHooks.end());
    for (std::vector<ServiceRegistrationBase>::reverse_iterator iter = eventHooks.rbegin(),
         iterEnd = eventHooks.rend(); iter != iterEnd; ++iter)
    {
      ServiceReference<BundleEventHook> sr;
      try
      {
        sr = iter->GetReference();
      }
      catch (const std::logic_error& e)
      {
        US_WARN << "Failed to get event hook service reference: " << e.what();
        continue;
      }

      BundleEventHook* eh = reinterpret_cast<BundleEventHook*>(sr.d->GetService(GetBundleContext()->GetBundle()));
      if (eh != NULL)
      {
        try
        {
          eh->Event(evt, filtered);
        }
        catch (const std::exception& e)
        {
          US_WARN << "Failed to call Bundle EventHook #" << sr.GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                  << ": " << e.what();
        }
        catch (...)
        {
          US_WARN << "Failed to call Bundle EventHook #" << sr.GetProperty(ServiceConstants::SERVICE_ID()).ToString()
                  << ": unknown exception type";
        }
      }
    }

    if (unfilteredSize != bundleContexts.size())
    {
      for (ServiceListeners::BundleListenerMap::iterator le = bundleListeners.begin();
           le != bundleListeners.end();)
      {
        if(std::find(bundleContexts.begin(), bundleContexts.end(), le->first) == bundleContexts.end())
        {
          bundleListeners.erase(le++);
        }
        else
        {
          ++le;
        }
      }
    }
  }
}

}
