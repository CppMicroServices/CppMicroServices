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

#include "BundleInstalledState.hpp"

#include "../BundlePrivate.h"
#include "BundleResolvedState.hpp"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/FrameworkEvent.h"

#include "../CoreBundleContext.h"

namespace cppmicroservices
{

    BundleInstalledState::BundleInstalledState()
    {
        std::promise<void> prom;
        ready = prom.get_future();
        prom.set_value();
    }

    BundleInstalledState::BundleInstalledState(std::shared_future<void> blockUntil)
        : ready(std::move(blockUntil))
    {
    }

    void
    BundleInstalledState::Start(BundlePrivate& bundle, uint32_t /*options*/)
    {
        // Auto-resolve then activate
        bundle.ResolveAndActivate();
    }

    std::exception_ptr
    BundleInstalledState::Stop(BundlePrivate& /*bundle*/, uint32_t /*options*/)
    {
        // Already stopped -- no-op
        return nullptr;
    }

    void
    BundleInstalledState::Uninstall(BundlePrivate& bundle)
    {
        bundle.RemoveAndCleanupBundle();
    }

    Bundle::State
    BundleInstalledState::GetUpdatedState(BundlePrivate& bundle)
    {
        auto currState = shared_from_this();
        std::promise<void> transitionAction;
        auto fut = transitionAction.get_future();
        auto resolvedState = std::make_shared<BundleResolvedState>(std::move(fut));

        if (currState->GetBundleState() == Bundle::STATE_INSTALLED)
        {
            if (bundle.CompareAndSetState(&currState, resolvedState))
            {
                currState->WaitForTransitionTask();
                try
                {
                    bundle.coreCtx->listeners.BundleChanged(
                        { BundleEvent::BUNDLE_RESOLVED, MakeBundle(bundle.shared_from_this()) });
                    transitionAction.set_value();
                }
                catch (...)
                {
                    bundle.resolveFailException = std::current_exception();
                    bundle.coreCtx->listeners.SendFrameworkEvent(
                        FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_ERROR,
                                       MakeBundle(bundle.shared_from_this()),
                                       std::string(),
                                       std::current_exception()));
                    transitionAction.set_value();
                }
                return bundle.GetBundleStateEnum();
            }
        }
        return currState->GetBundleState();
    }

} // namespace cppmicroservices
