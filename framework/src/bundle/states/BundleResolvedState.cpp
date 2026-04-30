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

#include "BundleResolvedState.hpp"

#include "../BundleContextPrivate.h"
#include "../BundlePrivate.h"
#include "BundleActiveState.hpp"
#include "BundleStartingState.hpp"
#include "BundleUninstalledState.hpp"

namespace cppmicroservices
{

    BundleResolvedState::BundleResolvedState()
    {
        std::promise<void> prom;
        ready = prom.get_future();
        prom.set_value();
    }

    BundleResolvedState::BundleResolvedState(std::shared_future<void> blockUntil)
        : ready(std::move(blockUntil))
    {
    }

    void
    BundleResolvedState::Start(BundlePrivate& bundle, uint32_t /*options*/)
    {
        auto currState = shared_from_this();
        std::promise<void> transitionAction;
        auto fut = transitionAction.get_future();
        auto startingState = std::make_shared<BundleStartingState>(std::move(fut));

        while (currState->GetBundleState() == Bundle::STATE_RESOLVED)
        {
            if (bundle.CompareAndSetState(&currState, startingState))
            {
                currState->WaitForTransitionTask();

                std::shared_ptr<BundleContextPrivate> null_expected;
                std::shared_ptr<BundleContextPrivate> ctx(new BundleContextPrivate(&bundle));
                bundle.bundleContext.CompareExchange(null_expected, ctx);

                auto e = bundle.ActivateBundle();

                if (e)
                {
                    transitionAction.set_value();
                    std::rethrow_exception(e);
                }
                transitionAction.set_value();
                return;
            }
        }
        // State changed -- delegate to new state
        bundle.GetLifecycleState()->Start(bundle, 0);
    }

    std::exception_ptr
    BundleResolvedState::Stop(BundlePrivate& /*bundle*/, uint32_t /*options*/)
    {
        // Already stopped -- no-op
        return nullptr;
    }

    void
    BundleResolvedState::Uninstall(BundlePrivate& bundle)
    {
        bundle.RemoveAndCleanupBundle();
    }

} // namespace cppmicroservices
