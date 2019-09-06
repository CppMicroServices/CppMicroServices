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

#include <memory>
#include <future>
#include <iostream>

#include "Mocks.hpp"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "../src/manager/states/CCRegisteredState.hpp"
#include "ConcurrencyTestUtil.hpp"

namespace cppmicroservices {
namespace scrimpl {

class CCRegisteredStateTest
  : public ::testing::Test
{
protected:
  CCRegisteredStateTest() : framework(cppmicroservices::FrameworkFactory().NewFramework())
  { }
  virtual ~CCRegisteredStateTest() = default;

  virtual void SetUp() {
    framework.Start();
    auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
    auto mockRegistry = std::make_shared<MockComponentRegistry>();
    auto fakeLogger = std::make_shared<FakeLogger>();
    mockCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                      framework,
                                                                      mockRegistry,
                                                                      fakeLogger);
  }

  virtual void TearDown() {
    mockCompConfig.reset();
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Framework framework;
  std::shared_ptr<MockComponentConfigurationImpl> mockCompConfig;
};

TEST_F(CCRegisteredStateTest, TestGetStateValue)
{
  auto state = std::make_shared<CCRegisteredState>();
  EXPECT_EQ(state->GetValue(), ComponentState::SATISFIED);
}

TEST_F(CCRegisteredStateTest, TestRegister)
{
  auto state = std::make_shared<CCRegisteredState>();
  mockCompConfig->SetState(state);
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::SATISFIED);
  EXPECT_NO_THROW({
      state->Register(*mockCompConfig);
    });
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::SATISFIED);
  EXPECT_EQ(mockCompConfig->GetState(), state);
}

TEST_F(CCRegisteredStateTest, TestActivate_Success)
{
  auto state = std::make_shared<CCRegisteredState>();
  auto mockCompInstance = std::make_shared<MockComponentInstance>();
  mockCompConfig->SetState(state);
  EXPECT_CALL(*mockCompConfig, CreateAndActivateComponentInstance(testing::_))
    .Times(1)
    .WillRepeatedly(testing::Return(mockCompInstance));
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::SATISFIED);
  EXPECT_NO_THROW({
      state->Activate(*mockCompConfig, framework);
    });
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
  EXPECT_NE(mockCompConfig->GetState(), state);
}

TEST_F(CCRegisteredStateTest, TestActivate_Failure)
{
  auto state = std::make_shared<CCRegisteredState>();
  mockCompConfig->SetState(state);
  EXPECT_CALL(*mockCompConfig, CreateAndActivateComponentInstance(testing::_))
    .Times(1)
    .WillRepeatedly(testing::Return(nullptr));
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::SATISFIED);
  EXPECT_NO_THROW({
      state->Activate(*mockCompConfig, framework);
    });
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::SATISFIED);
  EXPECT_NE(mockCompConfig->GetState(), state);
}

TEST_F(CCRegisteredStateTest, TestConcurrentActivate)
{
  auto state = std::make_shared<CCRegisteredState>();
  auto mockCompInstance = std::make_shared<MockComponentInstance>();
  mockCompConfig->SetState(state);
  EXPECT_CALL(*mockCompConfig, CreateAndActivateComponentInstance(testing::_))
    .WillRepeatedly(testing::Return(mockCompInstance));
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::SATISFIED);
  std::function<bool()> func = [&]() {
                                 state->Activate(*mockCompConfig, framework);
                                 return (mockCompConfig->GetConfigState() == ComponentState::ACTIVE);
                               };
  std::vector<bool> resultVec = ConcurrentInvoke<bool>(func);
  for(auto result : resultVec)
  {
    EXPECT_TRUE(result);
  }
  EXPECT_NE(mockCompConfig->GetState(), state);
}

TEST_F(CCRegisteredStateTest, TestDeactivate)
{
  auto state = std::make_shared<CCRegisteredState>();
  mockCompConfig->SetState(state);
  EXPECT_CALL(*mockCompConfig, DestroyComponentInstances())
    .Times(1);
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::SATISFIED);
  EXPECT_NO_THROW({
      state->Deactivate(*mockCompConfig);
    });
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  EXPECT_NE(mockCompConfig->GetState(), state);
}

TEST_F(CCRegisteredStateTest, TestConcurrentDeactivate)
{
  auto state = std::make_shared<CCRegisteredState>();
  mockCompConfig->SetState(state);
  EXPECT_CALL(*mockCompConfig, DestroyComponentInstances())
    .Times(1);
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::SATISFIED);
  std::function<ComponentState()> func = [&](){
                                           state->Deactivate(*mockCompConfig);
                                           return mockCompConfig->GetConfigState();
                                         };
  auto results = ConcurrentInvoke(func);
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  EXPECT_NE(mockCompConfig->GetState(), state);
}
}
}
