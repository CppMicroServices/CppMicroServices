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

#ifndef CCActiveState_hpp
#define CCActiveState_hpp

#include "../ConcurrencyUtil.hpp"
#include "CCSatisfiedState.hpp"
#include "cppmicroservices/detail/CounterLatch.h"

using cppmicroservices::service::component::runtime::dto::ComponentState;

namespace cppmicroservices
{
    namespace scrimpl
    {

        /**
         * This class represents the {\code ComponentState::ACTIVE} state of a
         * component configuration.
         */
        class CCActiveState final : public CCSatisfiedState
        {
          public:
            CCActiveState();
            ~CCActiveState() override = default;
            CCActiveState(CCActiveState const&) = delete;
            CCActiveState& operator=(CCActiveState const&) = delete;
            CCActiveState(CCActiveState&&) = delete;
            CCActiveState& operator=(CCActiveState&&) = delete;

            void Register(ComponentConfigurationImpl&) override {
                // no-op, already resolved
            };
            std::shared_ptr<ComponentInstance> Activate(ComponentConfigurationImpl& mgr,
                                                        cppmicroservices::Bundle const& clientBundle) override;

            void Deactivate(ComponentConfigurationImpl& mgr) override;

            /**
             * Modifies the properties of the component instance when a configuration object on
             * which it is dependent changes. No state change. R
             * @return
             *    - true if the component has a Modified method.
             *    - false if the component does not have a Modified method. The
             *      component has been Deactivated
             */
            bool Modified(ComponentConfigurationImpl&) override;

            /**
             * Rebind to a target service. This operation does not transition to another state.
             *
             * When rebinding the new target service must be bound first before the old
             * target service is unbound.
             * This reversed order allows the component to not have to handle the inevitable gap
             * between the unbind and bind methods.
             */
            void Rebind(ComponentConfigurationImpl& mgr,
                        std::string const& refName,
                        ServiceReference<void> const& svcRefToBind,
                        ServiceReference<void> const& svcRefToUnbind) override;

            /**
             * Returns {@link ComponentState::ACTIVE} to indicate the
             * state represented by this object
             */
            ComponentState
            GetValue() const override
            {
                return ComponentState::ACTIVE;
            }
            void
            WaitForTransitionTask() override
            {
                latch.Wait();
            }

          private:
            detail::CounterLatch latch;
        };
    } // namespace scrimpl
} // namespace cppmicroservices
#endif // CCActiveState_hpp
