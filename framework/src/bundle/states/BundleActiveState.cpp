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

#include "BundleActiveState.hpp"

#include "../BundlePrivate.h"
#include "BundleStoppingState.hpp"

namespace cppmicroservices
{

    BundleActiveState::BundleActiveState()
    {
        std::promise<void> prom;
        ready = prom.get_future();
        prom.set_value();
    }

    BundleActiveState::BundleActiveState(std::shared_future<void> blockUntil)
        : ready(std::move(blockUntil))
    {
    }

    void
    BundleActiveState::Start(BundlePrivate& /*bundle*/, uint32_t /*options*/)
    {
        // Already active -- no-op
        return;
    }

    std::exception_ptr
    BundleActiveState::Stop(BundlePrivate& bundle, uint32_t /*options*/)
    {
        auto currState = shared_from_this();
        std::promise<void> transitionAction;
        auto fut = transitionAction.get_future();
        auto stoppingState = std::make_shared<BundleStoppingState>(std::move(fut));

        while (currState->GetBundleState() == Bundle::STATE_ACTIVE)
        {
            if (bundle.CompareAndSetState(&currState, stoppingState))
            {
                currState->WaitForTransitionTask();
                bundle.wasStarted = true;
                auto savedException = bundle.DeactivateAndTransitionToResolved(stoppingState);
                transitionAction.set_value();
                return savedException;
            }
        }
        return currState->Stop(bundle, 0);
    }

    void
    BundleActiveState::Uninstall(BundlePrivate& bundle)
    {
        // Stop first, then uninstall
        auto exception = Stop(bundle, 0);
        if (exception)
        {
            bundle.ReportFrameworkError(exception);
        }
        bundle.RemoveAndCleanupBundle();
    }

} // namespace cppmicroservices
