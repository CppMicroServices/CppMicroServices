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

#ifndef ComponentConfigurationState_hpp
#define ComponentConfigurationState_hpp

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/ServiceReference.h"
#include <future>
#include <memory>
#include <string>
#include <vector>

#include "cppmicroservices/servicecomponent/detail/ComponentInstance.hpp"
#include "cppmicroservices/servicecomponent/runtime/dto/ComponentConfigurationDTO.hpp"

using cppmicroservices::service::component::detail::ComponentInstance;
using cppmicroservices::service::component::runtime::dto::ComponentState;

namespace cppmicroservices
{
    namespace scrimpl
    {
        class ComponentConfigurationImpl;

        /**
         * Interface for state objects used in {@link ComponentConfigurationImpl} class
         */
        class ComponentConfigurationState : public std::enable_shared_from_this<ComponentConfigurationState>
        {
          public:
            ComponentConfigurationState() = default;
            virtual ~ComponentConfigurationState() = default;
            ComponentConfigurationState(ComponentConfigurationState const&) = delete;
            ComponentConfigurationState& operator=(ComponentConfigurationState const&) = delete;
            ComponentConfigurationState(ComponentConfigurationState&&) = delete;
            ComponentConfigurationState& operator=(ComponentConfigurationState&&) = delete;

            /**
             * Implementation must handle the transition from \c UNSATISFIED_REFERENCE to \c SATISFIED
             *
             * \param mgr is the {@link ComponentConfigurationImpl} object whose state needs to change
             */
            virtual void Register(ComponentConfigurationImpl& mgr) = 0;

            /**
             * Implementation must handle the transition from \c SATISFIED to \c ACTIVE
             *
             * \param mgr is the {@link ComponentConfigurationImpl} object whose state needs to change
             * \param clientBundle is the bundle responsible for triggering the state change
             */
            virtual std::shared_ptr<ComponentInstance> Activate(ComponentConfigurationImpl& mgr,
                                                                cppmicroservices::Bundle const& clientBundle)
                = 0;

            /**
             * Implementation must handle the transition from \c SATISFIED or \c ACTIVE to \c UNSATISFIED_REFERENCE
             *
             * @param mgr is the {@link ComponentConfigurationImpl} object whose state needs to change
             */
            virtual void Deactivate(ComponentConfigurationImpl& mgr) = 0;

            /**
             * Implementation must modify the properties of a component instance when a configuration
             * object on which it is dependent changes.
             * @return boolean
             *    - true if the component has a Modified method.
             *    - false if the component does not have a Modified method. The
             *      component has been Deactivated
             */
            virtual bool Modified(ComponentConfigurationImpl& mgr) = 0;

            /**
             * Implementation must handle dynamic rebinding in any state.
             *
             * \param mgr is the {@link ComponentConfigurationImpl} object whose state needs to change
             * \param refName is the name of the reference as defined in the SCR JSON
             * \param svcRefToBind is the service reference to the target service to bind. A default
             *  constructed \c ServiceReference<void> denotes that there is no service to bind.
             * \param svcRefToUnbind is the service reference to the target service to unbind.
             *  A default constructed \c ServiceReference<void> denotes that there is no service to unbind.
             */
            virtual void Rebind(ComponentConfigurationImpl& mgr,
                                std::string const& refName,
                                ServiceReference<void> const& svcRefToBind,
                                ServiceReference<void> const& svcRefToUnbind)
                = 0;

            /**
             * Returns the state as a {@link ComponentState} enum value
             */
            virtual ComponentState GetValue() const = 0;

            /**
             * Implementation must wait until the transition task is finished
             */
            virtual void WaitForTransitionTask() = 0;
        };
    } // namespace scrimpl
} // namespace cppmicroservices
#endif // ComponentConfigurationState_hpp
