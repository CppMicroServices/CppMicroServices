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

#include <cstdio>
#include <iostream>
#include <algorithm>
#include <random>

#include "cppmicroservices/Any.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/BundleContext.h"

#include "../src/manager/ComponentManagerImpl.hpp"
#include "../src/manager/states/ComponentManagerState.hpp"
#include "../src/metadata/ComponentMetadata.hpp"

#include "Mocks.hpp"
#include "ConcurrencyTestUtil.hpp"

using cppmicroservices::Any;
using cppmicroservices::Bundle;

namespace cppmicroservices {
namespace scrimpl {

TEST(ComponentManagerImplTest, Ctor)
{
  auto framework = cppmicroservices::FrameworkFactory().NewFramework();
  framework.Start();
  auto bc = framework.GetBundleContext();
  auto fakeLogger = std::make_shared<FakeLogger>();
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  {
    EXPECT_THROW(
      {
        US_UNUSED(std::make_shared<ComponentManagerImpl>(
          nullptr, mockRegistry, bc, fakeLogger));
      },
      std::invalid_argument);
  }
  {
    EXPECT_THROW(
      {
        US_UNUSED(std::make_shared<ComponentManagerImpl>(
          mockMetadata, nullptr, bc, fakeLogger));
      },
      std::invalid_argument);
  }
  {
    EXPECT_THROW(
      {
        US_UNUSED(std::make_shared<ComponentManagerImpl>(
          mockMetadata, mockRegistry, BundleContext(), fakeLogger));
      },
      std::invalid_argument);
  }
  {
    EXPECT_THROW(
      {
        US_UNUSED(std::make_shared<ComponentManagerImpl>(
          mockMetadata, mockRegistry, bc, nullptr));
      },
      std::invalid_argument);
  }
  {
    EXPECT_NO_THROW({
      US_UNUSED(std::make_shared<ComponentManagerImpl>(
        mockMetadata, mockRegistry, bc, fakeLogger));
    });
  }
}

// The fixture for testing class ComponentManagerImpl.
class ComponentManagerImplParameterizedTest : public ::testing::TestWithParam<std::shared_ptr<metadata::ComponentMetadata>> {
protected:
  ComponentManagerImplParameterizedTest()
    : framework(cppmicroservices::FrameworkFactory().NewFramework())
  {
  }

  virtual ~ComponentManagerImplParameterizedTest() = default;

  virtual void SetUp() {
    framework.Start();
    fakeLogger = std::make_shared<FakeLogger>();
    mockRegistry = std::make_shared<MockComponentRegistry>();
  }

  virtual void TearDown() {
    fakeLogger.reset();
    mockRegistry.reset();
    framework.Stop();
    framework.WaitForStop(std::chrono::seconds::zero());
  }

  cppmicroservices::Framework framework;
  std::shared_ptr<logservice::LogService> fakeLogger;
  std::shared_ptr<MockComponentRegistry> mockRegistry;
};

TEST_P(ComponentManagerImplParameterizedTest, VerifyInitialize)
{
  auto compDesc = GetParam();
  auto compMgr = std::make_shared<ComponentManagerImpl>(compDesc,
                                                        mockRegistry,
                                                        framework.GetBundleContext(),
                                                        fakeLogger);
  EXPECT_EQ(compMgr->IsEnabled(), false) << "Illegal state before Initialization";
  compMgr->Initialize();
  EXPECT_EQ(compMgr->IsEnabled(), compMgr->GetMetadata()->enabled) << "Illegal state after Initialization";
}

TEST_P(ComponentManagerImplParameterizedTest, VerifyEnable)
{
  auto compDesc = GetParam();
  auto compMgr = std::make_shared<ComponentManagerImpl>(compDesc,
                                                        mockRegistry,
                                                        framework.GetBundleContext(),
                                                        fakeLogger);
  EXPECT_NO_THROW({
      compMgr->Initialize();
      compMgr->Enable();
      EXPECT_EQ(compMgr->IsEnabled(), true) << "State expected to be ENABLED";
      compMgr->Enable(); // enabling an already enabled component results in no state change
      EXPECT_EQ(compMgr->IsEnabled(), true) << "State expected to stay as ENABLED";
    });
}

TEST_P(ComponentManagerImplParameterizedTest, VerifyDisable)
{
  auto compDesc = GetParam();
  auto compMgr = std::make_shared<ComponentManagerImpl>(compDesc,
                                                        mockRegistry,
                                                        framework.GetBundleContext(),
                                                        fakeLogger);
  EXPECT_NO_THROW({
      compMgr->Initialize();
      compMgr->Disable();
      EXPECT_EQ(compMgr->IsEnabled(), false) << "State expected to be DISABLED";
      compMgr->Disable(); // Disabling an already disabled component results in no state change
      EXPECT_EQ(compMgr->IsEnabled(), false) << "State expected to stay as DISABLED";
    });
}

TEST_P(ComponentManagerImplParameterizedTest, VerifyStateChangeCount)
{
  auto compDesc = GetParam();
  auto compMgr = std::make_shared<MockComponentManagerImpl>(compDesc,
                                                            mockRegistry,
                                                            framework.GetBundleContext(),
                                                            fakeLogger);
  EXPECT_NO_THROW({
      compMgr->Initialize();
      compMgr->ResetCounter();
      auto wasEnabled = compMgr->IsEnabled();
      compMgr->Disable();
      EXPECT_EQ(compMgr->statechangecount, wasEnabled ? 1 : 0) << "Unexpected number of state changes during a call to Disable a ComponentManager";
      EXPECT_EQ(compMgr->IsEnabled(), false) << "ComponentManager must be in DISABLED state after a call to Disable";
    });
}

TEST_P(ComponentManagerImplParameterizedTest, VerifySequentialStateChange)
{
  auto compDesc = GetParam();
  auto compMgr = std::make_shared<MockComponentManagerImpl>(compDesc,
                                                            mockRegistry,
                                                            framework.GetBundleContext(),
                                                            fakeLogger);
  EXPECT_NO_THROW({

      auto prevState = compMgr->IsEnabled();
      compMgr->Initialize();
      // Initialize will swicth to enabled only if "enabled" is set in the comp description
      // two atomic swaps per state change.
      EXPECT_EQ(compMgr->statechangecount, compMgr->GetMetadata()->enabled ? 1 : 0) << "Unexpected number of state changes during a call to Initialize a ComponentManager";
      prevState = compMgr->IsEnabled();
      compMgr->ResetCounter();
      compMgr->Disable();
      EXPECT_EQ(compMgr->IsEnabled(), false) << "ComponentManager must be in DISABLED state after a call to Disable";
      EXPECT_EQ(compMgr->statechangecount, prevState ? 1 : 0) << "Unexpected number of state changes during a call to Disable a ComponentManager";
      compMgr->ResetCounter();
      prevState = compMgr->IsEnabled();
      compMgr->Enable();
      EXPECT_EQ(compMgr->statechangecount, !prevState ? 1 : 0) << "Unexpected number of state changes during a call to Enable a ComponentManager";
      EXPECT_EQ(compMgr->IsEnabled(), true) << "ComponentManager must be in ENABLED state after a call to Enable";
      compMgr->ResetCounter();
    });
}

TEST_P(ComponentManagerImplParameterizedTest, VerifyConcurrentEnable)
{
  auto compDesc = GetParam();
  auto compMgr = std::make_shared<MockComponentManagerImpl>(compDesc,
                                                            mockRegistry,
                                                            framework.GetBundleContext(),
                                                            fakeLogger);

  compMgr->Initialize();
  compMgr->Disable(); // ensure the component is in DISABLED state
  compMgr->ResetCounter();

  // test concurrent calls to "enable" from multiple threads
  std::function<std::shared_future<void>()> func = [compMgr]() {
                                                     return compMgr->Enable();
                                                   };
  std::vector<std::shared_future<void>> results = ConcurrentInvoke(func);

  // verify component manager is disabled and the manager has performed two atomic state change operations for the disable operation.
  EXPECT_EQ(compMgr->IsEnabled(), true) << "ComponentManager must be in ENABLED state after a call to Enable";
  EXPECT_EQ(compMgr->statechangecount, 1) << "Unexpected number of state changes after concurrent calls to Enable a ComponentManager";
  for(auto& fut : results)
  {
    EXPECT_EQ(fut.valid(), true) << "A valid future is expected as a return value from ComponentManager::Enable";
    fut.wait();
  }
}

TEST_P(ComponentManagerImplParameterizedTest, VerifyConcurrentDisable)
{
  auto compDesc = GetParam();
  auto compMgr = std::make_shared<MockComponentManagerImpl>(compDesc,
                                                            mockRegistry,
                                                            framework.GetBundleContext(),
                                                            fakeLogger);

  compMgr->Initialize();
  compMgr->Enable(); // ensure the component is in ENABLED state
  compMgr->ResetCounter();

  // test concurrent calls to "disable" from multiple threads
  std::function<std::shared_future<void>()> func = [compMgr]() {
                                                     return compMgr->Disable();
                                                   };
  std::vector<std::shared_future<void>> results = ConcurrentInvoke(func);

  // verify component manager is disabled and the manager has performed two atomic state change operations for the disable operation.
  EXPECT_EQ(compMgr->IsEnabled(), false) << "ComponentManager must be in DISABLED state after a call to Disable";
  EXPECT_EQ(compMgr->statechangecount, 1) << "Unexpected number of state changes after concurrent calls to Disable a ComponentManager";
  for(auto& fut : results)
  {
    EXPECT_EQ(fut.valid(), true) << "A valid future is expected as a return value from ComponentManager::Disable";
    fut.wait();
  }
}

TEST_P(ComponentManagerImplParameterizedTest, VerifyConcurrentEnableDisable)
{
  auto compDesc = GetParam();
  auto compMgr = std::make_shared<MockComponentManagerImpl>(compDesc,
                                                            mockRegistry,
                                                            framework.GetBundleContext(),
                                                            fakeLogger);
  compMgr->Initialize();
  // test concurrent calls to enable and disable from multiple threads
  std::function<std::shared_future<void>()> func = [compMgr]() mutable {
                                                     std::vector<std::shared_future<void>> futVec;
                                                     std::random_device rd;
                                                     std::mt19937 gen(rd());
                                                     std::uniform_int_distribution<unsigned int> dis(20,50);
                                                     int randVal = dis(gen); // random number in range [20, 50)
                                                     for(int i =0; i < randVal; ++i)
                                                     {
                                                       futVec.push_back(((i & 0x1) ? compMgr->Disable() : compMgr->Enable()));
                                                     }
                                                     return futVec.back();
                                                   };
  std::vector<std::shared_future<void>> results = ConcurrentInvoke(func);

  for(auto& fut : results)
  {
    EXPECT_EQ(fut.valid(), true);
    fut.get();
  }
}

TEST_P(ComponentManagerImplParameterizedTest, TestAccumulateFutures)
{
  auto compDesc = GetParam();
  auto compMgr = std::make_shared<MockComponentManagerImpl>(compDesc,
                                                            mockRegistry,
                                                            framework.GetBundleContext(),
                                                            fakeLogger);

  EXPECT_EQ(compMgr->disableFutures.size(), 0ul) << "Disabled futures list must be empty before any calls to AccumulateFuture method";
  std::promise<void> p1;
  compMgr->AccumulateFuture(p1.get_future().share());
  EXPECT_EQ(compMgr->disableFutures.size(), 1ul);
  std::promise<void> p2;
  compMgr->AccumulateFuture(p2.get_future().share());
  EXPECT_EQ(compMgr->disableFutures.size(), 2ul);
  std::promise<void> p3;
  compMgr->AccumulateFuture(p3.get_future().share());
  EXPECT_EQ(compMgr->disableFutures.size(), 3ul);
  p1.set_value();
  std::promise<void> p4;
  compMgr->AccumulateFuture(p4.get_future().share());
  EXPECT_EQ(compMgr->disableFutures.size(), 3ul);
  p4.set_value();
  std::promise<void> p5;
  compMgr->AccumulateFuture(p5.get_future().share());
  EXPECT_EQ(compMgr->disableFutures.size(), 3ul);
  p2.set_value();
  p3.set_value();
  p5.set_value();
}

/**
 * Util method to create component descriptions used for parametrized tests for ComponentManagerImpl
 */
std::shared_ptr<metadata::ComponentMetadata> CreateComponentMetadata(const std::string& implClassName,
                                                                     bool defaultEnabled)
{
  auto compDesc = std::make_shared<metadata::ComponentMetadata>();
  compDesc->name = compDesc->implClassName = implClassName;
  compDesc->enabled = defaultEnabled;
  compDesc->immediate = false;
  return compDesc;
}

INSTANTIATE_TEST_SUITE_P(ComponentManagerParameterized, ComponentManagerImplParameterizedTest,
                        testing::Values(CreateComponentMetadata("foo", false)/* default disabled */,
                                        CreateComponentMetadata("bar", true) /* default enabled */));
}
}

