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

#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "../src/manager/states/CCUnsatisfiedReferenceState.hpp"
#include "Mocks.hpp"
#include "ConcurrencyTestUtil.hpp"

namespace cppmicroservices {
namespace scrimpl {

class CCUnsatisfiedReferenceStateTest
  : public ::testing::Test
{
protected:
  CCUnsatisfiedReferenceStateTest() : framework(cppmicroservices::FrameworkFactory().NewFramework())
  { }
  virtual ~CCUnsatisfiedReferenceStateTest() = default;

  virtual void SetUp() {
    framework.Start();
    auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
    mockMetadata->serviceMetadata.interfaces.push_back("Service::Interface");
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

TEST_F(CCUnsatisfiedReferenceStateTest, TestGetStateValue)
{
  auto state = std::make_shared<CCUnsatisfiedReferenceState>();
  EXPECT_EQ(state->GetValue(), ComponentState::UNSATISFIED_REFERENCE);
}

TEST_F(CCUnsatisfiedReferenceStateTest, TestActivate)
{
  auto state = std::make_shared<CCUnsatisfiedReferenceState>();
  mockCompConfig->SetState(state);
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  EXPECT_NO_THROW({
      auto inst = state->Activate(*mockCompConfig, framework);
      EXPECT_EQ(inst, nullptr);
    });
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  EXPECT_EQ(mockCompConfig->GetState(), state);
}

TEST_F(CCUnsatisfiedReferenceStateTest, TestDeactivate)
{
  auto state = std::make_shared<CCUnsatisfiedReferenceState>();
  mockCompConfig->SetState(state);
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  EXPECT_NO_THROW({
      state->Deactivate(*mockCompConfig);
    });
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  EXPECT_EQ(mockCompConfig->GetState(), state);
}

TEST_F(CCUnsatisfiedReferenceStateTest, TestRegister)
{
  auto state = std::make_shared<CCUnsatisfiedReferenceState>();
  mockCompConfig->SetState(state);
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  EXPECT_CALL(*mockCompConfig, GetFactory())
    .WillRepeatedly(testing::Return(std::make_shared<MockFactory>()));
  EXPECT_NO_THROW({
      state->Register(*mockCompConfig);
    });
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::SATISFIED);
  EXPECT_NE(mockCompConfig->GetState(), state);
  EXPECT_EQ(framework.GetBundleContext().GetServiceReferences("Service::Interface").size(), 1u);
}

TEST_F(CCUnsatisfiedReferenceStateTest, TestRegister_Failure)
{
  auto state = std::make_shared<CCUnsatisfiedReferenceState>();
  mockCompConfig->SetState(state);
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  EXPECT_CALL(*mockCompConfig, GetFactory())
    .WillRepeatedly(testing::Return(nullptr));
  EXPECT_NO_THROW({
      state->Register(*mockCompConfig);
    });
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  EXPECT_EQ(framework.GetBundleContext().GetServiceReferences("Service::Interface").size(), 0u);
}

TEST_F(CCUnsatisfiedReferenceStateTest, TestConcurrentRegister)
{
  auto state = std::make_shared<CCUnsatisfiedReferenceState>();
  mockCompConfig->SetState(state);
  EXPECT_EQ(mockCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  EXPECT_CALL(*mockCompConfig, GetFactory())
    .WillRepeatedly(testing::Return(std::make_shared<MockFactory>()));
  std::function<ComponentState()> func = [&state, this]() {
                                           EXPECT_NO_THROW({
                                               state->Register(*mockCompConfig);
                                             });
                                           return mockCompConfig->GetConfigState();
                                         };
  std::vector<ComponentState> resultVec = ConcurrentInvoke(func);
  for(auto result : resultVec)
  {
    EXPECT_EQ(result, ComponentState::SATISFIED);
  }
  EXPECT_NE(mockCompConfig->GetState(), state);
  EXPECT_EQ(framework.GetBundleContext().GetServiceReferences("Service::Interface").size(), 1u);
}
}
}
