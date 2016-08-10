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

#include "FrameworkPrivate.h"

#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"

#include "BundleContextPrivate.h"

namespace cppmicroservices {

FrameworkPrivate::FrameworkPrivate(CoreBundleContext* fwCtx)
    : BundlePrivate(fwCtx)
{
    // default the internal framework event to what should be
    // returned if a client calls WaitForStop while this
    // framework is not in an active, starting or stopping state.
    stopEvent = FrameworkEventInternal{ true, FrameworkEvent::Type::FRAMEWORK_ERROR, std::string(), nullptr };
}

void FrameworkPrivate::DoInit()
{
  state = Bundle::STATE_STARTING;
  coreCtx->Init();
}

void FrameworkPrivate::Init()
{
  auto l = Lock();
  WaitOnOperation(*this, l, "Framework::Init", true);

  switch (static_cast<Bundle::State>(state.load()))
  {
  case Bundle::STATE_INSTALLED:
  case Bundle::STATE_RESOLVED:
    break;
  case Bundle::STATE_STARTING:
  case Bundle::STATE_ACTIVE:
    return;
  default:
    std::stringstream ss;
    ss << state;
    throw std::logic_error("INTERNAL ERROR, Illegal state, " + ss.str());
  }
  this->DoInit();
}

void FrameworkPrivate::InitSystemBundle()
{
  bundleContext.Store(std::make_shared<BundleContextPrivate>(this));

  // TODO Capabilities
  /*
      std::string sp;
      sp.append(coreCtx->frameworkProperties.getProperty(Constants::FRAMEWORK_SYSTEMCAPABILITIES));
      // Add in extra system capabilities
      std::string epc = coreCtx->frameworkProperties.getProperty(Constants::FRAMEWORK_SYSTEMCAPABILITIES_EXTRA);
      if (!epc.empty())
      {
        if (!sp.empty())
        {
          sp.append(',');
        }
        sp.append(epc);
      }
      provideCapabilityString = sp;
      */

  // TODO Wiring
  /*
      BundleGeneration* gen = new BundleGeneration(this, exportPackageString,
                                                        provideCapabilityString);
      generations.add(gen);
      gen->SetWired();
      fwWiring = new FrameworkWiringImpl(coreCtx);
      */
  timeStamp = Clock::now();
}

void FrameworkPrivate::UninitSystemBundle()
{
  auto bc = bundleContext.Exchange(std::shared_ptr<BundleContextPrivate>());
  if (bc) bc->Invalidate();
}

FrameworkEvent FrameworkPrivate::WaitForStop(const std::chrono::milliseconds& timeout)
{
  auto l = Lock();
  // Already stopped?
  if (((Bundle::STATE_INSTALLED | Bundle::STATE_RESOLVED) & state) == 0)
  {
    stopEvent = FrameworkEventInternal{ false, FrameworkEvent::Type::FRAMEWORK_ERROR, std::string(), std::exception_ptr() };
    if (timeout == std::chrono::milliseconds::zero())
    {
      Wait(l, [&] { return stopEvent.valid; });
    }
    else
    {
      WaitFor(l, timeout, [&] { return stopEvent.valid; });
    }
    if (!stopEvent.valid)
    {
      return FrameworkEvent(
            FrameworkEvent::Type::FRAMEWORK_WAIT_TIMEDOUT,
            MakeBundle(this->shared_from_this()),
            std::string(),
            std::exception_ptr()
            );
    }
  }
  else if (!stopEvent.valid)
  {
    // Return this if stop or update have not been called and framework is
    // stopped.
    stopEvent = FrameworkEventInternal{
          true,
          FrameworkEvent::Type::FRAMEWORK_STOPPED,
          std::string(),
          std::exception_ptr()};
  }
  if (shutdownThread.joinable()) shutdownThread.join();
  return FrameworkEvent(stopEvent.type, MakeBundle(this->shared_from_this()), stopEvent.msg, stopEvent.excPtr);
}

void FrameworkPrivate::Shutdown(bool restart)
{
  auto l = Lock(); US_UNUSED(l);
  bool wasActive = false;
  switch (static_cast<Bundle::State>(state.load()))
  {
  case Bundle::STATE_INSTALLED:
  case Bundle::STATE_RESOLVED:
    ShutdownDone_unlocked(false);
    break;
  case Bundle::STATE_ACTIVE:
    wasActive = true;
    // Fall through
  case Bundle::STATE_STARTING:
  {
    const bool wa = wasActive;
#ifdef US_ENABLE_THREADING_SUPPORT
    if (!shutdownThread.joinable())
    {
      shutdownThread = std::thread(std::bind(&FrameworkPrivate::Shutdown0, this, restart, wa));
    }
#else
    Shutdown0(restart, wa);
#endif
    break;
  }
  case Bundle::STATE_UNINSTALLED:
  case Bundle::STATE_STOPPING:
    // Shutdown already inprogress
    break;
  }
}

void FrameworkPrivate::Shutdown0(bool restart, bool wasActive)
{
  try
  {
    {
      auto l = Lock();
      WaitOnOperation(*this, l, std::string("Framework::") + (restart ? "Update" : "Stop"), true);
      operation = OP_DEACTIVATING;
      state = Bundle::STATE_STOPPING;
    }
    coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::BUNDLE_STOPPING, MakeBundle(this->shared_from_this())));
    if (wasActive)
    {
      StopAllBundles();
    }
    coreCtx->Uninit0();
    {
      auto l = Lock(); US_UNUSED(l);
      coreCtx->Uninit1();
      ShutdownDone_unlocked(restart);
    }
    if (restart)
    {
      if (wasActive)
      {
        Start(0);
      }
      else
      {
        Init();
      }
    }
  }
  catch (...)
  {
    auto l = Lock(); US_UNUSED(l);
    SystemShuttingdownDone_unlocked(
          FrameworkEventInternal{
            true,
            FrameworkEvent::Type::FRAMEWORK_ERROR,
            std::string(),
            std::current_exception()
          }
          );
  }
}

void FrameworkPrivate::ShutdownDone_unlocked(bool restart)
{
  auto t = restart ? FrameworkEvent::Type::FRAMEWORK_STOPPED_UPDATE : FrameworkEvent::Type::FRAMEWORK_STOPPED;
  SystemShuttingdownDone_unlocked(
        FrameworkEventInternal{
          true,
          t,
          std::string(),
          std::exception_ptr()});
}

void FrameworkPrivate::StopAllBundles()
{
  // Stop all active bundles, in reverse bundle ID order
  auto activeBundles = coreCtx->bundleRegistry.GetActiveBundles();
  for (auto iter = activeBundles.rbegin(); iter != activeBundles.rend(); ++iter)
  {
    auto b = *iter;
    try
    {
      if (((Bundle::STATE_ACTIVE | Bundle::STATE_STARTING) & b->state) != 0)
      {
        // Stop bundle without changing its autostart setting.
        b->Stop(Bundle::StopOptions::STOP_TRANSIENT);
      }
    }
    catch (...)
    {
        coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(
            FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_ERROR,
                MakeBundle(b),
                std::string(),
                std::current_exception())));
    }
  }

  auto allBundles = coreCtx->bundleRegistry.GetBundles();

  // Set state to BUNDLE_INSTALLED
  for (auto b : allBundles)
  {
    if (b->id != 0)
    {
      auto l = coreCtx->resolver.Lock();
      b->SetStateInstalled(false, l);
    }
  }
}

void FrameworkPrivate::SystemShuttingdownDone_unlocked(const FrameworkEventInternal& fe)
{
  if (state != Bundle::STATE_INSTALLED)
  {
    state = Bundle::STATE_RESOLVED;
    operation = OP_IDLE;
    NotifyAll();
  }
  stopEvent = fe;
}

}
