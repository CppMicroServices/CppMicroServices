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

#ifndef CPPMICROSERVICES_BUNDLESTARTINGSTATE_HPP
#define CPPMICROSERVICES_BUNDLESTARTINGSTATE_HPP

#include "BundleLifecycleState.hpp"

namespace cppmicroservices
{

    class BundleStartingState final : public BundleLifecycleState
    {
      public:
        BundleStartingState();
        explicit BundleStartingState(std::shared_future<void> blockUntil);
        ~BundleStartingState() override = default;

        void Start(BundlePrivate& bundle, uint32_t options) override;
        std::exception_ptr Stop(BundlePrivate& bundle, uint32_t options) override;
        void Uninstall(BundlePrivate& bundle) override;

        Bundle::State
        GetBundleState() const override
        {
            return Bundle::STATE_STARTING;
        }

        void
        WaitForTransitionTask() override
        {
            ready.get();
        }

      private:
        std::shared_future<void> ready;
    };

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_BUNDLESTARTINGSTATE_HPP
