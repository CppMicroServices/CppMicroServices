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

#include <future>
#include <iostream>
#include <memory>
#include <random>

#include "../../src/SCRAsyncWorkService.hpp"
#include "../../src/SCRExtensionRegistry.hpp"
#include "../../src/manager/states/CCActiveState.hpp"
#include "ConcurrencyTestUtil.hpp"
#include "Mocks.hpp"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

namespace cppmicroservices
{
    namespace scrimpl
    {

        class CCActiveStateTest : public ::testing::Test
        {
          protected:
            CCActiveStateTest() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {}
            virtual ~CCActiveStateTest() = default;

            virtual void
            SetUp()
            {
                framework.Start();
                auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
                auto mockRegistry = std::make_shared<MockComponentRegistry>();
                auto fakeLogger = std::make_shared<FakeLogger>();
                auto logger = std::make_shared<SCRLogger>(framework.GetBundleContext());
                auto asyncWorkService
                    = std::make_shared<cppmicroservices::scrimpl::SCRAsyncWorkService>(framework.GetBundleContext(),
                                                                                       logger);
                auto extRegistry = std::make_shared<SCRExtensionRegistry>(logger);
                auto notifier = std::make_shared<ConfigurationNotifier>(framework.GetBundleContext(),
                                                                        fakeLogger,
                                                                        asyncWorkService,
                                                                        extRegistry);
 
                mockCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                                  framework,
                                                                                  mockRegistry,
                                                                                  fakeLogger,
                                                                                  notifier);
             }

            virtual void
            TearDown()
            {
                mockCompConfig.reset();
                framework.Stop();
                framework.WaitForStop(std::chrono::milliseconds::zero());
            }

            cppmicroservices::Framework framework;
            std::shared_ptr<MockComponentConfigurationImpl> mockCompConfig;
        };

        TEST_F(CCActiveStateTest, TestGetStateValue)
        {
            auto state = std::make_shared<CCActiveState>();
            EXPECT_EQ(state->GetValue(), ComponentState::ACTIVE);
        }

        TEST_F(CCActiveStateTest, TestRegister)
        {
            auto state = std::make_shared<CCActiveState>();
            mockCompConfig->SetState(state);
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
            EXPECT_NO_THROW({ state->Register(*mockCompConfig); });
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
            EXPECT_EQ(mockCompConfig->GetState(), state);
        }

        TEST_F(CCActiveStateTest, TestActivate)
        {
            auto state = std::make_shared<CCActiveState>();
            mockCompConfig->SetState(state);
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
            EXPECT_CALL(*mockCompConfig, CreateAndActivateComponentInstance(testing::_))
                .Times(1)
                .WillOnce(testing::Return(std::make_shared<MockComponentInstance>()));
            EXPECT_NO_THROW({
                auto inst = state->Activate(*mockCompConfig, framework);
                EXPECT_NE(inst, nullptr);
            });
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
            EXPECT_EQ(mockCompConfig->GetState(), state);
        }

        TEST_F(CCActiveStateTest, TestActivateWithInvalidLatch)
        {
            auto state = std::make_shared<CCActiveState>();
            mockCompConfig->SetState(state);
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
            EXPECT_NO_THROW({
                state->WaitForTransitionTask();
                auto inst = state->Activate(*mockCompConfig, framework);
                EXPECT_EQ(inst, nullptr);
            });
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
            EXPECT_EQ(mockCompConfig->GetState(), state);
        }

        TEST_F(CCActiveStateTest, TestConcurrentActivate)
        {
            auto state = std::make_shared<CCActiveState>();
            mockCompConfig->SetState(state);
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
            EXPECT_CALL(*mockCompConfig, CreateAndActivateComponentInstance(testing::_))
                .WillRepeatedly(
                    testing::Invoke([](Bundle const&) { return std::make_shared<MockComponentInstance>(); }));
            std::function<std::shared_ptr<ComponentInstance>()> func
                = [&]() { return state->Activate(*mockCompConfig, framework); };
            auto results = ConcurrentInvoke(func);
            auto resultSize = results.size();
            // eliminate duplicates
            auto it = std::unique(results.begin(), results.end());
            results.resize(std::distance(results.begin(), it));
            EXPECT_EQ(resultSize, results.size());

            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
            EXPECT_EQ(mockCompConfig->GetState(), state);
        }

        TEST_F(CCActiveStateTest, TestDeactivate)
        {
            auto state = std::make_shared<CCActiveState>();
            mockCompConfig->SetState(state);
            EXPECT_CALL(*mockCompConfig, DestroyComponentInstances()).Times(1);
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
            EXPECT_NO_THROW({ state->Deactivate(*mockCompConfig); });
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
            EXPECT_NE(mockCompConfig->GetState(), state);
        }

        TEST_F(CCActiveStateTest, TestConcurrentDeactivate)
        {
            auto state = std::make_shared<CCActiveState>();
            mockCompConfig->SetState(state);
            EXPECT_CALL(*mockCompConfig, DestroyComponentInstances()).Times(1);
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
            std::function<bool()> func = [&]()
            {
                state->Deactivate(*mockCompConfig);
                return true;
            };
            auto results = ConcurrentInvoke(func);
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
            EXPECT_NE(mockCompConfig->GetState(), state);
        }

        TEST_F(CCActiveStateTest, TestConcurrentActivateDeactivate)
        {
            auto state = std::make_shared<CCActiveState>();
            mockCompConfig->SetState(state);
            std::atomic<int> activeInstanceCount;
            EXPECT_CALL(*mockCompConfig, DestroyComponentInstances())
                .Times(1)
                .WillOnce(testing::Invoke([&]() { activeInstanceCount = 0; }));
            EXPECT_CALL(*mockCompConfig, CreateAndActivateComponentInstance(testing::_))
                .WillRepeatedly(testing::Invoke(
                    [&](Bundle const&)
                    {
                        activeInstanceCount++;
                        return std::make_shared<MockComponentInstance>();
                    }));
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
            std::function<std::pair<TimePoint, bool>()> func = [&]()
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<unsigned int> dis;
                int randVal = dis(gen);
                if (randVal & 0x1)
                {
                    auto inst = state->Activate(*mockCompConfig, Bundle());
                    return std::make_pair(std::chrono::system_clock::now(), inst ? true : false);
                }
                else
                {
                    state->Deactivate(*mockCompConfig);
                    return std::make_pair(std::chrono::system_clock::now(), false);
                }
            };
            auto results = ConcurrentInvoke(func);
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
            EXPECT_NE(mockCompConfig->GetState(), state);
            EXPECT_EQ(activeInstanceCount, 0);
        }
        // Test that DS completes Modification before Deactivation
        // The Deactivate method is called immediately following the Modified method
        // This test verifies that the Modified method processing is allowed to complete
        // before the Deactivate method processing begins.
        // The Modified method does some processing and eventually calls ModifyComponentInstanceProperties
        // The Deactivate method calls DestroyComponentInstances.
        // If the Modified method is allowed to complete then the ModifyComponentInstanceProperties method
        // will be called before the DestroyComponentInstances.

        TEST_F(CCActiveStateTest, TestModifiedWithDeactivate)
        {
            // testing::InSequence says that any EXPECT_CALLS that occur in this block must happen
            // in the order in which they appear in the block.
            testing::InSequence expectCallsInOrder;

            // Create a mockComponentConfigurationImpl object and set the state to Active.
            auto state = std::make_shared<CCActiveState>();
            mockCompConfig->SetState(state);
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);

            // Expect ModifyComponentInstanceProperties to happen before DestroyComponentInstances.
            // Also, ModifyComponentInstanceProperties has an action specified so that it will return
            // true when called. If it returned the default (false) then DestroyComponentInstances would
            // be called by the code that calls ModifyComponentInstanceProperties
            // and would not be a valid test.
            EXPECT_CALL(*mockCompConfig, ModifyComponentInstanceProperties).Times(1).WillOnce(testing::Return(true));
            EXPECT_CALL(*mockCompConfig, DestroyComponentInstances()).Times(1);
            EXPECT_NO_THROW({ state->Modified(*mockCompConfig); });
            EXPECT_NO_THROW({ state->Deactivate(*mockCompConfig); });

            // Verify that the Deactivate operation completed successfully and the state is changed to
            // UNSATISFIED_REFERENCE.
            EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
            EXPECT_NE(mockCompConfig->GetState(), state);
        }
    } // namespace scrimpl
} // namespace cppmicroservices
