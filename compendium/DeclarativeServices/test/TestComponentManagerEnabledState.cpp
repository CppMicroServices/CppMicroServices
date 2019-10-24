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
#include "../src/manager/states/CMEnabledState.hpp"
#include "Mocks.hpp"
#include "ConcurrencyTestUtil.hpp"

namespace cppmicroservices {
namespace scrimpl {

class CMEnabledStateTest
  : public ::testing::Test
{
protected:
  CMEnabledStateTest() : framework(cppmicroservices::FrameworkFactory().NewFramework())
  { }
  virtual ~CMEnabledStateTest() = default;

  virtual void SetUp() {
    framework.Start();
    auto fakeLogger = std::make_shared<FakeLogger>();
    auto compDesc = std::make_shared<metadata::ComponentMetadata>();
    auto mockRegistry = std::make_shared<MockComponentRegistry>();
    compMgr = std::make_shared<MockComponentManagerImpl>(compDesc,
                                                         mockRegistry,
                                                         framework.GetBundleContext(),
                                                         fakeLogger);
  }

  virtual void TearDown() {
    compMgr.reset();
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Framework framework;
  std::shared_ptr<MockComponentManagerImpl> compMgr;
};

TEST_F(CMEnabledStateTest, TestCtor)
{
  EXPECT_NO_THROW({
      std::promise<void> prom;
      auto enabledState = std::make_shared<CMEnabledState>(prom.get_future().share());
      EXPECT_TRUE(enabledState->fut.valid()) << "The future member of CMEnabledState must be valid";
      EXPECT_EQ(enabledState->fut.wait_for(std::chrono::microseconds(1)), std::future_status::timeout) << "The future member of CMEnabledState must not be ready until the corresponding promise is set";
      prom.set_value();
      enabledState->fut.get();
    });
}

TEST_F(CMEnabledStateTest, TestIsEnabled)
{
  std::promise<void> prom;
  auto enabledState = std::make_shared<CMEnabledState>(prom.get_future().share());
  EXPECT_TRUE(enabledState->IsEnabled(*compMgr)) << "CMEnabledState must always return true for IsEnabled";
}

TEST_F(CMEnabledStateTest, TestGetConfigurations)
{
  std::promise<void> prom;
  auto enabledState = std::make_shared<CMEnabledState>(prom.get_future().share());
  enabledState->configurations = { std::make_shared<MockComponentConfigurationImpl>(compMgr->GetMetadata(),
                                                                                    framework,
                                                                                    compMgr->GetRegistry(),
                                                                                    compMgr->GetLogger()) };
  auto fut = std::async(std::launch::async, [&](){
                                              return enabledState->GetConfigurations(*compMgr);
                                            });
  EXPECT_NE(fut.wait_for(std::chrono::milliseconds::zero()), std::future_status::ready) << "The call to GetConfigurations must not return until the promise is set";
  prom.set_value();
  auto configs = fut.get();
  EXPECT_EQ(configs.size(), 1ul) << "GetConfigurations must return the stored configuration";
  enabledState->configurations.clear(); // remove the inserted mock configuration
}

/**
 * This test point checks the case when a call to Enable on one thread
 * succeeds in swapping the state and is responsible for setting a promise
 * and a call to Enable on another thread has to wait until the first
 * thread sets the promise
 */
TEST_F(CMEnabledStateTest, TestEnable)
{
  std::promise<void> prom;
  auto enabledState = std::make_shared<CMEnabledState>(prom.get_future().share());
  compMgr->SetState(enabledState);
  auto fut = enabledState->Enable(*compMgr);
  EXPECT_NE(fut.wait_for(std::chrono::milliseconds::zero()), std::future_status::ready) << "Future returned from Enable must not be ready until the promise is set";
  prom.set_value();
  EXPECT_NO_THROW(fut.get());
}

TEST_F(CMEnabledStateTest, TestConcurrentEnable)
{
  std::promise<void> prom;
  auto enabledState = std::make_shared<CMEnabledState>(prom.get_future().share());
  compMgr->SetState(enabledState);
  prom.set_value();
  EXPECT_TRUE(compMgr->IsEnabled());
  // Invoke "Enable" from multiple threads
  std::function<std::shared_future<void>()> func = [enabledState, this]() {
                                                     return enabledState->Enable(*(this->compMgr));
                                                   };
  std::vector<std::shared_future<void>> futVec = ConcurrentInvoke(func);
  EXPECT_TRUE(compMgr->IsEnabled()) << "ComponentManager must still be ENABLED after concurrent calls to Enable";
  // check if the futures returned from concurrent invocation are all valid
  for(auto& fut : futVec)
  {
    EXPECT_TRUE(fut.valid()) << "All futures returned from concurrent calls to Enable must be valid";
    EXPECT_NO_THROW(fut.get());
  }
}

TEST_F(CMEnabledStateTest, TestDisable)
{
  std::promise<void> prom;
  auto enabledState = std::make_shared<CMEnabledState>(prom.get_future().share());
  prom.set_value();
  compMgr->SetState(enabledState);
  EXPECT_TRUE(compMgr->IsEnabled());
  auto fut = enabledState->Disable(*compMgr);
  EXPECT_TRUE(fut.valid()) << "A call to ComponentManager::Enable must always return a valid future";
  EXPECT_NO_THROW(fut.get());
  EXPECT_FALSE(compMgr->IsEnabled()) << "ComponentManager must be DISABLED after a call to Disable";
}

TEST_F(CMEnabledStateTest, TestConcurrentDisable)
{
  std::promise<void> prom;
  auto enabledState = std::make_shared<CMEnabledState>(prom.get_future().share());
  compMgr->SetState(enabledState);
  prom.set_value();
  EXPECT_TRUE(compMgr->IsEnabled());
  // Invoke "Disable" from multiple threads
  std::function<std::shared_future<void>()> func = [enabledState, this]() {
                                                     return enabledState->Disable(*(this->compMgr));
                                                   };
  std::vector<std::shared_future<void>> futVec = ConcurrentInvoke(func);
  EXPECT_FALSE(compMgr->IsEnabled()) << "ComponentManager must be DISABLED after concurrent calls to Disable";
  // check if the futures returned from concurrent invocation are all valid
  for(auto& fut : futVec)
  {
    EXPECT_TRUE(fut.valid()) << "All futures returned from concurrent calls to Disable must be valid";
    EXPECT_NO_THROW(fut.get());
  }
}

TEST_F(CMEnabledStateTest, TestCreateConfigurations)
{
  std::promise<void> prom;
  auto enabledState = std::make_shared<CMEnabledState>(prom.get_future().share());
  compMgr->SetState(enabledState);
  prom.set_value();
  EXPECT_EQ(enabledState->configurations.size(), 0ul) << "Initial number of configurations is zero";
  EXPECT_NO_THROW({
      enabledState->CreateConfigurations(compMgr->GetMetadata(),
                                         compMgr->GetBundle(),
                                         compMgr->GetRegistry(),
                                         compMgr->GetLogger());
    });
  EXPECT_EQ(enabledState->configurations.size(), 1ul) << "Must have a configuration created after call to CreateConfigurations";
  enabledState->configurations.clear(); // remove configs due to the call to the private method.
}
}
}
