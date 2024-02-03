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

#ifndef __COMPONENTMANAGER_HPP__
#define __COMPONENTMANAGER_HPP__

#include "../metadata/ComponentMetadata.hpp"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/ServiceFactory.h"
#include <future>
#include <memory>

namespace cppmicroservices
{
    namespace scrimpl
    {

        class ComponentConfiguration;
        /**
         * This interface provides the information about the current state of a component.
         * It is used by ServiceComponentRuntimeImpl and ComponentContextImpl classes.
         */
        class ComponentManager
        {
          public:
            ComponentManager() = default;
            ComponentManager(ComponentManager const&) = delete;
            ComponentManager(ComponentManager&&) = delete;
            ComponentManager& operator=(ComponentManager const&) = delete;
            ComponentManager& operator=(ComponentManager&&) = delete;
            virtual ~ComponentManager() = default;

            /**
             * Waits for the provided future from the asynchronous thread pool and executes
             * the task on current thread if the thread pool has stalled.
             *
             * We determine a likely stall by timeout (50ms) while waiting for the future. After this timeout, we
             * atomically compare and set the bool, asyncStarted, to 'take ownership' of execution of this task. If that
             * bool was already set, then the spwaned thread has already taken ownership of the task and we do not need
             * to execute it again. Otherwise, we execute the task. If we do not hit the timeout, then there was no
             * stall and we can get() the future.
             *
             * \param fut The future to wait on
             * \param asyncStarted The bool used to synchronize the waiting and posted thread
             * \return void, once the future has been satisfied
             */
            virtual void WaitForFuture(std::shared_future<void>& fut, std::shared_ptr<std::atomic<bool>> asyncStarted)
                = 0;

            /**
             * Returns the name of the component managed by this object. The name is the same
             * as specified in the component description.
             */
            virtual std::string GetName() const = 0;

            /**
             * Returns the Id of the Bundle this component belongs to.
             */
            virtual unsigned long GetBundleId() const = 0;

            /**
             * Returns true if the component is enabled, false otherwise
             */
            virtual bool IsEnabled() const = 0;

            /**
             * This method changes the state of the ComponentManager to ENABLED. The method returns
             * immediately after changing the state. Any configurations created as a result of the
             * state change will happen asynchronously on a separate thread.
             *
             * \param asyncStarted The bool used to synchronize the waiting and posted thread
             * \parblock
             * If returned future IS blocked on: create asyncStarted value and pass in on calling thread.
             * Once the future is returned, call WaitForFuture() with the returned future and your asyncStarted value
             *
             * If future IS NOT blocked on: call Enable(), or explicitly Enable(nullptr), no other action is required
             * \endparblock
             * \return std::shared_future<void> assosciated with the state change
             */
            virtual std::shared_future<void> Enable(std::shared_ptr<std::atomic<bool>> asyncStarted = nullptr) = 0;

            /**
             * This method changes the state of the ComponentManager to DISABLED. The method returns
             * immediately after changing the state. Any configurations deleted as a result of the
             * state change will happen asynchronously on a separate thread. 
             *
             * \param asyncStarted The bool used to synchronize the waiting and posted thread
             * \parblock
             * If future IS blocked on: create asyncStarted value and pass in on calling thread.
             * Once the future is returned, call WaitForFuture() with the returned future and your asyncStarted value
             *
             * If future IS NOT blocked on: call Disable(), or explicitly Disable(nullptr), no other action is required
             * \endparblock
             * \return std::shared_future<void> assosciated with the state change
             */
            virtual std::shared_future<void> Disable(std::shared_ptr<std::atomic<bool>> asyncStarted = nullptr) = 0;

            /**
             * Returns a vector of ComponentConfiguration objects representing each of the configurations
             * created for the component.
             */
            virtual std::vector<std::shared_ptr<ComponentConfiguration>> GetComponentConfigurations() const = 0;

            /**
             * Returns the metadata object representing the component description for the
             * component managed by this object.
             */
            virtual std::shared_ptr<metadata::ComponentMetadata const> GetMetadata() const = 0;
        };
    } // namespace scrimpl
} // namespace cppmicroservices
#endif // __COMPONENTMANAGER_HPP__
