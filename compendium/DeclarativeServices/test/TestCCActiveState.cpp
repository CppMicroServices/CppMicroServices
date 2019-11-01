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
#include <random>

#include "Mocks.hpp"
#include "ConcurrencyTestUtil.hpp"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "../src/manager/states/CCActiveState.hpp"

typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

namespace cppmicroservices {
namespace scrimpl {

class CCActiveStateTest
  : public ::testing::Test
{
protected:
  CCActiveStateTest() : framework(cppmicroservices::FrameworkFactory().NewFramework())
  { }
  virtual ~CCActiveStateTest() = default;

  virtual void SetUp()
  {
    framework.Start();
    auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
    auto mockRegistry = std::make_shared<MockComponentRegistry>();
    auto fakeLogger = std::make_shared<FakeLogger>();
    mockCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                      framework,
                                                                      mockRegistry,
                                                                      fakeLogger);
  }

  virtual void TearDown()
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
  EXPECT_NO_THROW({
      state->Register(*mockCompConfig);
    });
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
    .WillRepeatedly(testing::Invoke([](const Bundle&) {
                                      return std::make_shared<MockComponentInstance>();
                                    }));
  std::function<std::shared_ptr<ComponentInstance>()> func = [&](){
                                                               return state->Activate(*mockCompConfig, framework);
                                                             };
  auto results = ConcurrentInvoke(func);
  auto resultSize = results.size();
  // eliminate duplicates
  auto it = std::unique (results.begin(), results.end());
  results.resize(std::distance(results.begin(),it));
  EXPECT_EQ(resultSize, results.size());

  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
  EXPECT_EQ(mockCompConfig->GetState(), state);
}

TEST_F(CCActiveStateTest, TestDeactivate)
{
  auto state = std::make_shared<CCActiveState>();
  mockCompConfig->SetState(state);
  EXPECT_CALL(*mockCompConfig, DestroyComponentInstances())
    .Times(1);
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
  EXPECT_NO_THROW({
      state->Deactivate(*mockCompConfig);
    });
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  EXPECT_NE(mockCompConfig->GetState(), state);
}

TEST_F(CCActiveStateTest, TestConcurrentDeactivate)
{
  auto state = std::make_shared<CCActiveState>();
  mockCompConfig->SetState(state);
  EXPECT_CALL(*mockCompConfig, DestroyComponentInstances())
    .Times(1);
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
  std::function<bool()> func = [&](){
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
    .WillOnce(testing::Invoke([&]() {
                                activeInstanceCount = 0;
                              }));
  EXPECT_CALL(*mockCompConfig, CreateAndActivateComponentInstance(testing::_))
    .WillRepeatedly(testing::Invoke([&](const Bundle&) {
                                      activeInstanceCount++;
                                      return std::make_shared<MockComponentInstance>();
                                    }));
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::ACTIVE);
  std::function<std::pair<TimePoint,bool>()> func = [&](){
                                                      std::random_device rd;
                                                      std::mt19937 gen(rd());
                                                      std::uniform_int_distribution<unsigned int> dis;
                                                      int randVal = dis(gen);
                                                      if(randVal & 0x1)
                                                      {
                                                        auto inst = state->Activate(*mockCompConfig, Bundle());
                                                        return std::make_pair(std::chrono::system_clock::now(), inst ? true : false);
                                                      }
                                                      else
                                                      {
                                                        state->Deactivate(*mockCompConfig);
                                                        return std::make_pair(std::chrono::system_clock::now(),false);
                                                      }
                                                    };
  auto results = ConcurrentInvoke(func);
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  EXPECT_NE(mockCompConfig->GetState(), state);
  EXPECT_EQ(activeInstanceCount, 0);
}
}
}
