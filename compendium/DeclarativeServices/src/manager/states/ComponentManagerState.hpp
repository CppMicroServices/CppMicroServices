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

#ifndef ComponentManagerState_hpp
#define ComponentManagerState_hpp

#include "../ComponentConfigurationImpl.hpp"
#include <future>
#include <memory>
#include <string>
#include <vector>

namespace cppmicroservices
{
    namespace scrimpl
    {

        class ComponentManagerImpl;
        class ComponentConfiguration;

        /**
         * Interface for the state objects used in ComponentManager's state machine.
         */
        class ComponentManagerState : public std::enable_shared_from_this<ComponentManagerState>
        {
          public:
            ComponentManagerState() = default;
            virtual ~ComponentManagerState() = default;
            ComponentManagerState(ComponentManagerState const&) = delete;
            ComponentManagerState& operator=(ComponentManagerState const&) = delete;
            ComponentManagerState(ComponentManagerState&&) = delete;
            ComponentManagerState& operator=(ComponentManagerState&&) = delete;

            /**
             * Implementation of this method must handle the Enable state transition for current state
             */
            virtual std::shared_future<void> Enable(ComponentManagerImpl& cm, std::shared_ptr<std::atomic<bool>> asyncStarted)
                = 0;

            /**
             * Implementation of this method must handle the Disable state transition for current state
             */
            virtual std::shared_future<void> Disable(ComponentManagerImpl& cm, std::shared_ptr<std::atomic<bool>> asyncStarted)
                = 0;

            /**
             * Implementation returns true if the current state is enabled state, false otherwise
             */
            virtual bool IsEnabled(ComponentManagerImpl const& cm) const = 0;

            /**
             * Implementation returns a \c std::vector of configurations created for
             * the component passed as parameter
             */
            virtual std::vector<std::shared_ptr<ComponentConfiguration>> GetConfigurations(
                ComponentManagerImpl const& cm) const
                = 0;

            /**
             * Implementation returns a \c std::shared_future object representing the async
             * task spawned due to transitioning to this state.
             */
            virtual std::shared_future<void> GetFuture() const = 0;
        };
    } // namespace scrimpl
} // namespace cppmicroservices

#endif /* ComponentManagerState_hpp */
