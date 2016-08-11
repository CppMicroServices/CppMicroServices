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

#include "cppmicroservices/Framework.h"

#include "cppmicroservices/FrameworkEvent.h"

#include "BundleStorage.h"
#include "FrameworkPrivate.h"

namespace cppmicroservices {

namespace {

FrameworkPrivate* pimpl(const std::shared_ptr<BundlePrivate>& p)
{
  return static_cast<FrameworkPrivate*>(p.get());
}

}

Framework::Framework(const std::shared_ptr<FrameworkPrivate>& d)
  : Bundle(d)
{
}

void Framework::Init()
{
  pimpl(d)->Init();
}

FrameworkEvent Framework::WaitForStop(const std::chrono::milliseconds& timeout)
{
  return pimpl(d)->WaitForStop(timeout);
}

void Framework::Start(uint32_t )
{
  Start();
}

void Framework::Start()
{
  std::vector<long> bundlesToStart;
  {
    auto l = d->Lock();
    d->WaitOnOperation(*d.get(), l, "Framework::Start", true);

    switch (d->state.load())
    {
    case STATE_INSTALLED:
    case STATE_RESOLVED:
      pimpl(d)->DoInit();
      // Fall through
    case STATE_STARTING:
      d->operation = BundlePrivate::OP_ACTIVATING;
      break;
    case STATE_ACTIVE:
      return;
    default:
      std::stringstream ss;
      ss << d->state;
      throw std::runtime_error("INTERNAL ERROR, Illegal state, " + ss.str());
    }
    bundlesToStart = pimpl(d)->coreCtx->storage->GetStartOnLaunchBundles();
  }

  // Start bundles according to their autostart setting.
  for (auto i : bundlesToStart)
  {
    auto b = d->coreCtx->bundleRegistry.GetBundle(i);
    try
    {
      const int32_t autostartSetting = b->barchive->GetAutostartSetting();
      // Launch must not change the autostart setting of a bundle
      int option = Bundle::START_TRANSIENT;
      if (Bundle::START_ACTIVATION_POLICY == autostartSetting)
      {
        // Transient start according to the bundles activation policy.
        option |= Bundle::START_ACTIVATION_POLICY;
      }
      b->Start(option);
    }
    catch (...)
    {
      pimpl(d)->coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_ERROR, MakeBundle(b->shared_from_this()), std::string(), std::current_exception()));
    }
  }

  {
    auto l = d->Lock(); US_UNUSED(l);
    d->state = STATE_ACTIVE;
    d->operation = BundlePrivate::OP_IDLE;
  }
  d->NotifyAll();
  pimpl(d)->coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_STARTED, MakeBundle(d->shared_from_this()), std::string()));
}

void Framework::Stop(uint32_t )
{
  Stop();
}

void Framework::Stop()
{
  pimpl(d)->Shutdown(false);
}

void Framework::Uninstall()
{
  throw std::runtime_error("Cannot uninstall a system bundle.");
}

std::string Framework::GetLocation() const
{
  // OSGi Core release 6, section 4.6:
  //  The system bundle GetLocation method returns the string: "System Bundle"
  return std::string("System Bundle");
}

}
