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

#ifndef CMDisabledState_hpp
#define CMDisabledState_hpp

#if defined(USING_GTEST)
#    include "gtest/gtest_prod.h"
#else
#    define FRIEND_TEST(x, y)
#endif
#include "ComponentManagerState.hpp"
#include "../SingleInvokeTask.hpp"

namespace cppmicroservices
{
    namespace scrimpl
    {

        /**
         * This class represents the disabled state of {\code ComponentManagerImpl}
         */
        class CMDisabledState final : public ComponentManagerState
        {
          public:
            /**
             * Default Constructor - constructs the object with a valid future
             *
             * \note {@link ComponentManagerImpl} uses this constructor for initial state.
             * All others use the constructor with a future param.
             */
            CMDisabledState();

            /**
             * Construct with a future object
             *
             * /param fut is the future associated with this state. This future represents
             *        the task performed when the ComponentManager changes it's state from
             *        \c CMEnabledState to this object.
             */
            explicit CMDisabledState(std::shared_future<void> fut) : fut(std::move(fut)) {};
            ~CMDisabledState() override = default;
            CMDisabledState(CMDisabledState const&) = delete;
            CMDisabledState& operator=(CMDisabledState const&) = delete;
            CMDisabledState(CMDisabledState&&) = delete;
            CMDisabledState& operator=(CMDisabledState&&) = delete;

            /**
             * This method is used to change the state of the
             * {\code ComponentManagerImpl} from DISABLED to ENABLED. This method
             * returns immediately after switching the state of the manager object.
             * Any actions resulting from the state change happen asynchronously on a
             * separate thread.
             *
             * \param cm is the component manager which needs to be enabled
             * \param singleInvoke SingleInvokeTask object managing the async execution
             * \return a future object representing the actions performed due to the state change
             *
             */
            std::shared_future<void> Enable(ComponentManagerImpl& cm,
                                            std::shared_ptr<SingleInvokeTask> singleInvoke) override;

            /**
             * This method returns the stored future object. Since this object
             * represents disable state, there is no action performed by this method.
             * The stored future represents the actions performed due to the previous
             * change from ENABLED to DISABLED. For example, if two threads invoke
             * Disable on the {\code ComponentManagerImpl}, the first thread succeeds
             * in replacing the state with a new object of {\code CMDisableState}.
             * The second thread's call to disable is forwarded to the
             * {\code CMDisabledState} created by the first thread. The second thread
             * receives the same future that was created by the first thread.
             *
             * \param cm is the component manager which needs to be disabled
             * \param singleInvoke SingleInvokeTask object managing the async execution
             * \return a future object representing the actions performed due to the previous
             * change from ENABLED to DISABLED
             */
            std::shared_future<void> Disable(ComponentManagerImpl& cm,
                                             std::shared_ptr<SingleInvokeTask> singleInvoke) override;

            /**
             * Returns an empty vector because there are no configurations associated
             * with a disabled state
             */
            std::vector<std::shared_ptr<ComponentConfiguration>> GetConfigurations(
                ComponentManagerImpl const& cm) const override;

            /**
             * Returns false indicating this is not the ENABLED state
             */
            bool
            IsEnabled(ComponentManagerImpl const& /*cm*/) const override
            {
                return false;
            }

            /**
             * Returns a \c shared_future representing the asynchronous task
             * spawned to delete the component configuration objects created
             * when the component was enabled.
             */
            std::shared_future<void>
            GetFuture() const override
            {
                return fut;
            }

          private:
            FRIEND_TEST(CMDisabledStateTest, Ctor);
            std::shared_future<void> fut; ///< future associated with the transition task
        };
    } // namespace scrimpl
} // namespace cppmicroservices

#endif /* CMDisabledState_hpp */
