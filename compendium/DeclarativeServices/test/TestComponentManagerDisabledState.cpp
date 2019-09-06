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

#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "../src/manager/states/CMDisabledState.hpp"
#include "Mocks.hpp"
#include "ConcurrencyTestUtil.hpp"

namespace cppmicroservices {
namespace scrimpl {

class CMDisabledStateTest
  : public ::testing::Test
{
protected:
  CMDisabledStateTest() : framework(cppmicroservices::FrameworkFactory().NewFramework())
  { }
  virtual ~CMDisabledStateTest() = default;

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

TEST_F(CMDisabledStateTest, Ctor)
{
  // default contructor
  EXPECT_NO_THROW({
      auto disabledState = std::make_shared<CMDisabledState>();
      EXPECT_TRUE(disabledState->fut.valid());
      disabledState->fut.get();
    });

  // constructor with a future
  std::promise<void> prom;
  auto enabledState = std::make_shared<CMDisabledState>(prom.get_future().share());
  EXPECT_TRUE(enabledState->fut.valid());
  EXPECT_EQ(enabledState->fut.wait_for(std::chrono::microseconds(1)), std::future_status::timeout) << "The future member of CMDisabledState must not be ready until the corresponding promise is set";
  prom.set_value();
  enabledState->fut.get();
}

TEST_F(CMDisabledStateTest, TestIsEnabled)
{
  auto enabledState = std::make_shared<CMDisabledState>();
  EXPECT_FALSE(enabledState->IsEnabled(*compMgr)) << "CMDisabledState must always return false for IsEnabled";
}

TEST_F(CMDisabledStateTest, GetConfigurations)
{
  EXPECT_NO_THROW({
      auto disabledState = std::make_shared<CMDisabledState>();
      auto configs = disabledState->GetConfigurations(*compMgr);
      EXPECT_TRUE(configs.empty()) << "Component Configurations must not exist in a disabled state.";
    });
}

TEST_F(CMDisabledStateTest, TestDisable)
{
  auto disabledState = std::make_shared<CMDisabledState>();
  compMgr->SetState(disabledState);
  EXPECT_NO_THROW({
      auto fut = disabledState->Disable(*compMgr);
      EXPECT_TRUE(fut.valid()) << "A call to ComponentManager::Disable must always return a valid future";
      EXPECT_FALSE(compMgr->IsEnabled()) << "ComponentManager must be DISABLED after a call to Disable";
    });
}

TEST_F(CMDisabledStateTest, TestConcurrentDisable)
{
  auto disabledState = std::make_shared<CMDisabledState>();
  compMgr->SetState(disabledState);
  // Invoke "Disable" from multiple threads
  std::function<std::shared_future<void>()> func = [&disabledState, this]() {
                                                     return disabledState->Disable(*compMgr);
                                                   };
  std::vector<std::shared_future<void>> futVec = ConcurrentInvoke<std::shared_future<void>>(func);
  EXPECT_FALSE(compMgr->IsEnabled()) << "ComponentManager state must be DISABLED after concurrent calls to Disable";
  // check if the futures returned from concurrent invocation are all valid
  for(auto fut : futVec)
  {
    EXPECT_TRUE(fut.valid()) << "All futures returned from concurrent calls to Disable must be valid";
    EXPECT_NO_THROW({
        fut.get();
      });
  }
}

TEST_F(CMDisabledStateTest, TestEnable)
{
  auto disabledState = std::make_shared<CMDisabledState>();
  compMgr->SetState(disabledState);
  auto fut = disabledState->Enable(*compMgr);
  EXPECT_TRUE(fut.valid()) << "A call to ComponentManager::Enable must always return a valid future";
  EXPECT_NO_THROW({
      fut.get();
    });
  EXPECT_TRUE(compMgr->IsEnabled()) << "ComponentManager must be ENABLED after a call to Enable";
}

TEST_F(CMDisabledStateTest, TestConcurrentEnable)
{
  auto disabledState = std::make_shared<CMDisabledState>();
  compMgr->SetState(disabledState);
  // Invoke "Enable" from multiple threads
  std::function<std::shared_future<void>()> func = [&disabledState, this]() {
                                                     return disabledState->Enable(*(this->compMgr));
                                                   };
  std::vector<std::shared_future<void>> futVec = ConcurrentInvoke<std::shared_future<void>>(func);
  EXPECT_TRUE(compMgr->IsEnabled()) << "ComponentManager state must be ENABLED after concurrent calls to Enable";
  // check if the futures returned from concurrent invocation are all valid
  for(auto fut : futVec)
  {
    EXPECT_TRUE(fut.valid()) << "All futures returned from concurrent calls to Enable must be valid";
    EXPECT_NO_THROW({
        fut.get();
      });
  }
}
}
}

