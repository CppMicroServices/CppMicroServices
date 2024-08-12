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

#ifndef CMEnabledState_hpp
#define CMEnabledState_hpp

#if defined(USING_GTEST)
#    include "gtest/gtest_prod.h"
#else
#    define FRIEND_TEST(x, y)
#endif
#include "../../ComponentRegistry.hpp"
#include "../../metadata/ComponentMetadata.hpp"
#include "../ConfigurationNotifier.hpp"
#include "ComponentManagerState.hpp"
#include "cppmicroservices/logservice/LogService.hpp"

namespace cppmicroservices
{
    namespace scrimpl
    {

        class ComponentConfigurationImpl;

        class CMEnabledState final : public ComponentManagerState
        {
          public:
            /**
             * Construct with a future object
             *
             * /param fut is the future associated with this state. This future represents
             *        the task performed when the ComponentManager changes it's state to
             *        this object. The transition task associated with the change from this
             *        state to the next \c CMDisabledState has to wait on this future.
             */
            explicit CMEnabledState(std::shared_future<void> fut) : fut(std::move(fut)) {}
            ~CMEnabledState() override = default;
            CMEnabledState(CMEnabledState const&) = delete;
            CMEnabledState& operator=(CMEnabledState const&) = delete;
            CMEnabledState(CMEnabledState&&) = delete;
            CMEnabledState& operator=(CMEnabledState&&) = delete;

            /**
             * This method simply returns the stored future. No state change takes place
             *
             * \param cm is the component manager which needs to be enabled
             * \param singleInvoke SingleInvokeTask object managing the async execution
             * \return a future object representing the actions performed due to the
             *         previous state change from \c DISABLED to \c ENABLED
             */
            std::shared_future<void> Enable(ComponentManagerImpl& /*cm*/,
                                            std::shared_ptr<SingleInvokeTask> /*singleInvoke*/) override;

            /**
             * This method changes the state of the {@link ComponentManagerImpl} object
             *
             * \param cm is the component manager which needs to be disabled
             * \param singleInvoke SingleInvokeTask object managing the async execution
             * \return a future object representing the actions performed due to the state change
             */
            std::shared_future<void> Disable(ComponentManagerImpl& cm,
                                             std::shared_ptr<SingleInvokeTask> singleInvoke) override;

            /**
             * This method waits for the configurations to be created and then returns the
             * list of configurations associated with the {@link ComponentManagerImpl} object
             *
             * \param cm is the component manager
             * \return a vector of configurations for the given component manager
             */
            std::vector<std::shared_ptr<ComponentConfiguration>> GetConfigurations(
                ComponentManagerImpl const& cm) const override;

            /**
             * Always returns true since this state represents an Enabled state
             */
            bool
            IsEnabled(ComponentManagerImpl const& /*cm*/) const override
            {
                return true;
            }

            /**
             * Returns the future associated with the transition into this state
             */
            std::shared_future<void>
            GetFuture() const override
            {
                return fut;
            }

            /**
             * Helper function used to create configuration objects for the
             * {@link ComponentManagerImpl} object. This method populates the
             * {@link #configurations} vector in this object
             *
             * \param compDesc is the component metadata
             * \param bundle which contains the component
             * \param registry is the runtime's component registry
             * \param logger is the runtime's logger
             */
            void CreateConfigurations(std::shared_ptr<metadata::ComponentMetadata const> compDesc,
                                      cppmicroservices::Bundle const& bundle,
                                      std::shared_ptr<ComponentRegistry> registry,
                                      std::shared_ptr<logservice::LogService> logger,
                                      std::shared_ptr<ConfigurationNotifier> configNotifier);

            /**
             * Helper function used to remove all the configuration objects created by this state.
             */
            void DeleteConfigurations();

            FRIEND_TEST(CMEnabledStateTest, TestCtor);
            FRIEND_TEST(CMEnabledStateTest, TestEnable);
            FRIEND_TEST(CMEnabledStateTest, TestDisable);
            FRIEND_TEST(CMEnabledStateTest, TestConcurrentEnable);
            FRIEND_TEST(CMEnabledStateTest, TestConcurrentDisable);
            FRIEND_TEST(CMEnabledStateTest, TestGetConfigurations);
            FRIEND_TEST(CMEnabledStateTest, TestCreateConfigurationsAsync);
            FRIEND_TEST(CMEnabledStateTest, TestCreateConfigurations);

            std::shared_future<void>
                fut; ///< future object to represent the transition task associated with this state.
            std::vector<std::shared_ptr<ComponentConfigurationImpl>>
                configurations; ///< configurations created by this state object
        };
    } // namespace scrimpl
} // namespace cppmicroservices

#endif /* CMEnabledState_hpp */
