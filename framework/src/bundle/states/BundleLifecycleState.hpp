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

#ifndef CPPMICROSERVICES_BUNDLELIFECYCLESTATE_HPP
#define CPPMICROSERVICES_BUNDLELIFECYCLESTATE_HPP

#include "cppmicroservices/Bundle.h"

#include <exception>
#include <future>
#include <memory>

namespace cppmicroservices
{

    class BundlePrivate;

    /**
     * Abstract base class for bundle lifecycle states.
     *
     * Each concrete state handles the lifecycle operations (Start, Stop, Uninstall)
     * valid in that state. State transitions use atomic compare-and-swap on
     * BundlePrivate::lifecycleState, and each state holds a std::shared_future<void>
     * to coordinate transitions (callers wait on the future before starting their
     * own work, preventing duplicate transitions).
     *
     * This follows the same pattern as ComponentConfigurationState in
     * DeclarativeServices.
     */
    class BundleLifecycleState : public std::enable_shared_from_this<BundleLifecycleState>
    {
      public:
        BundleLifecycleState() = default;
        virtual ~BundleLifecycleState() = default;
        BundleLifecycleState(BundleLifecycleState const&) = delete;
        BundleLifecycleState& operator=(BundleLifecycleState const&) = delete;
        BundleLifecycleState(BundleLifecycleState&&) = delete;
        BundleLifecycleState& operator=(BundleLifecycleState&&) = delete;

        /**
         * Handle a Start request in this state.
         */
        virtual void Start(BundlePrivate& bundle, uint32_t options) = 0;

        /**
         * Handle a Stop request in this state.
         * Returns any exception that occurred during stopping.
         */
        virtual std::exception_ptr Stop(BundlePrivate& bundle, uint32_t options) = 0;

        /**
         * Handle an Uninstall request in this state.
         */
        virtual void Uninstall(BundlePrivate& bundle) = 0;

        /**
         * Get updated bundle state. Default returns GetBundleState().
         * Overridden by BundleInstalledState to auto-resolve.
         */
        virtual Bundle::State
        GetUpdatedState(BundlePrivate& /*bundle*/)
        {
            return GetBundleState();
        }

        /**
         * Returns the Bundle::State enum value this state object represents.
         */
        virtual Bundle::State GetBundleState() const = 0;

        /**
         * Block until this state's transition work is complete.
         */
        virtual void WaitForTransitionTask() = 0;
    };

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_BUNDLELIFECYCLESTATE_HPP
