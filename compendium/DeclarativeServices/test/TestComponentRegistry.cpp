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

#include <chrono>
#include <algorithm>
#include <vector>
#include <memory>
#include "gmock/gmock.h"
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/BundleContext.h>
#include "../src/ComponentRegistry.hpp"
#include "../src/manager/ComponentManager.hpp"
#include "../src/manager/ComponentConfiguration.hpp"
#include "../src/metadata/ComponentMetadata.hpp"
#include "Mocks.hpp"
using cppmicroservices::Bundle;
using cppmicroservices::scrimpl::ComponentManager;
using cppmicroservices::scrimpl::ComponentConfiguration;
using cppmicroservices::scrimpl::ComponentRegistry;
using cppmicroservices::scrimpl::metadata::ComponentMetadata;

namespace cppmicroservices {
namespace scrimpl {

std::string GenRandomString()
{
  static const std::string name("com::servicecomponentimpl.testcompname");
  std::string tempName = name;
  std::random_shuffle(tempName.begin(), tempName.end());
  return tempName;
}

class FakeComponentManager
  : public ComponentManager
{
public:
  FakeComponentManager() : ComponentManager()
                         ,mBundleId(++id)
                         ,mName(GenRandomString()) { }
  ~FakeComponentManager() {};
  unsigned long GetBundleId() const override { return mBundleId; }
  std::string GetName() const override { return mName; }
  MOCK_CONST_METHOD0(GetBundle, Bundle(void));
  MOCK_METHOD0(Initialize, void(void));
  MOCK_CONST_METHOD0(IsEnabled, bool(void));
  MOCK_METHOD0(Enable, std::shared_future<void>(void));
  MOCK_METHOD0(Disable, std::shared_future<void>(void));
  MOCK_CONST_METHOD0(GetComponentConfigurations,std::vector<std::shared_ptr<ComponentConfiguration>>(void));
  MOCK_CONST_METHOD0(GetMetadata, std::shared_ptr<const ComponentMetadata>(void));
private:
  long mBundleId;
  std::string mName;
  static std::atomic<long> id;
};

std::atomic<long> FakeComponentManager::id(0);

// The fixture for testing class ComponentRegistry.
class ComponentRegistryTest
  : public ::testing::Test
{
protected:
  ComponentRegistryTest()
    : framework(cppmicroservices::FrameworkFactory().NewFramework())
    , registry(std::make_shared<ComponentRegistry>())
  { }
  virtual ~ComponentRegistryTest() = default;

  virtual void SetUp()
  {
    framework.Start();
    auto bundleContext = framework.GetBundleContext();
    auto allBundles = bundleContext.GetBundles();
    EXPECT_EQ(allBundles.size(), static_cast<size_t>(2));
  }

  virtual void TearDown()
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Framework& GetFramework() { return framework; }
  std::shared_ptr<ComponentRegistry> GetRegistry() { return registry; }
private:
  cppmicroservices::Framework framework;
  std::shared_ptr<ComponentRegistry> registry;
};

TEST_F(ComponentRegistryTest, VerifyAddComponentManager)
{
  auto registry = GetRegistry();
  auto mockCompMgr = std::make_shared<MockComponentManager>();
  EXPECT_CALL(*mockCompMgr, GetBundleId()).Times(1).WillOnce(testing::Return(121));
  EXPECT_CALL(*mockCompMgr, GetName()).Times(1).WillOnce(testing::Return(std::string("Foo")));
  EXPECT_EQ(registry->Count(), 0ul);
  registry->AddComponentManager(mockCompMgr);
  EXPECT_EQ(registry->Count(), 1ul);
}

TEST_F(ComponentRegistryTest, VerifyGetComponentManager)
{
  auto registry = GetRegistry();
  auto mockCompMgr = std::make_shared<MockComponentManager>();
  EXPECT_CALL(*mockCompMgr, GetBundleId()).Times(1).WillOnce(testing::Return(121));
  EXPECT_CALL(*mockCompMgr, GetName()).Times(1).WillOnce(testing::Return(std::string("Foo")));
  auto mockCompMgr1 = std::make_shared<MockComponentManager>();
  EXPECT_CALL(*mockCompMgr1, GetBundleId()).Times(1).WillOnce(testing::Return(121));
  EXPECT_CALL(*mockCompMgr1, GetName()).Times(1).WillOnce(testing::Return(std::string("Bar")));
  auto mockCompMgr2 = std::make_shared<MockComponentManager>();
  EXPECT_CALL(*mockCompMgr2, GetBundleId()).Times(1).WillOnce(testing::Return(122));
  EXPECT_CALL(*mockCompMgr2, GetName()).Times(1).WillOnce(testing::Return(std::string("Foo")));
  EXPECT_EQ(registry->Count(), 0ul);
  registry->AddComponentManager(mockCompMgr);
  registry->AddComponentManager(mockCompMgr1);
  registry->AddComponentManager(mockCompMgr2);
  EXPECT_EQ(registry->Count(), 3ul);
  EXPECT_EQ(registry->GetComponentManager(121, "Foo"), mockCompMgr);
  EXPECT_EQ(registry->GetComponentManagers().size(), 3ul);
  EXPECT_EQ(registry->GetComponentManagers(121).size(), 2ul);
}

TEST_F(ComponentRegistryTest, VerifyRemoveComponentManager)
{
  auto registry = GetRegistry();
  auto mockCompMgr = std::make_shared<MockComponentManager>();
  EXPECT_CALL(*mockCompMgr, GetBundleId()).Times(2).WillRepeatedly(testing::Return(121));
  EXPECT_CALL(*mockCompMgr, GetName()).Times(2).WillRepeatedly(testing::Return(std::string("Foo")));
  registry->AddComponentManager(mockCompMgr);
  EXPECT_EQ(registry->Count(), 1ul);
  registry->RemoveComponentManager(mockCompMgr);
  EXPECT_EQ(registry->Count(), 0ul);
}

TEST_F(ComponentRegistryTest, VerifyRemoveComponentManagerByIdName)
{
  auto registry = GetRegistry();
  auto mockCompMgr = std::make_shared<MockComponentManager>();
  EXPECT_CALL(*mockCompMgr, GetBundleId()).Times(1).WillOnce(testing::Return(121));
  EXPECT_CALL(*mockCompMgr, GetName()).Times(1).WillOnce(testing::Return(std::string("Foo")));
  registry->AddComponentManager(mockCompMgr);
  EXPECT_EQ(registry->Count(), 1ul);
  registry->RemoveComponentManager(121, "Foo");
  EXPECT_EQ(registry->Count(), 0ul);
}

TEST_F(ComponentRegistryTest, VerifyConcurrentAddsRemoves)
{
  auto registry = GetRegistry();
  size_t expected_count(0);
  std::set<std::pair<long, std::string>> randomComps;
  std::mutex compNameMutex; // protects changes to randomComps & expected_count
      
  {
    std::promise<void> go;
    try
    { // Add Elements concurrently
      std::shared_future<void> ready(go.get_future());
      std::size_t numCalls = 20;
      std::vector<std::promise<void>> readies(numCalls);
      std::vector<std::future<void>> registry_adds(numCalls);
      for(std::size_t i =0; i<numCalls; i++)
      {
        registry_adds[i] = std::async(std::launch::async,
                                      [registry, ready, &readies, i, &expected_count, &compNameMutex, &randomComps]()
                                      {
                                        readies[i].set_value();
                                        std::shared_ptr<ComponentManager> cm(std::make_shared<FakeComponentManager>());
                                        ready.wait();
                                        if(registry->AddComponentManager(cm))
                                        {
                                          std::lock_guard<std::mutex> lock(compNameMutex);
                                          randomComps.insert(std::make_pair(cm->GetBundleId(),cm->GetName()));
                                          expected_count++;
                                        }
                                      });
      }

      for(std::size_t i =0; i< numCalls; i++)
      {
        readies[i].get_future().wait();
      }

      go.set_value();

      for(std::size_t i =0; i< numCalls; i++)
      {
        registry_adds[i].get();
      }
    }
    catch(...)
    {
      go.set_value();
      throw;
    }

    std::promise<void> go2;
    try
    { // Remove elements concurrently
      std::shared_future<void> ready2(go2.get_future());
      std::size_t numCalls = randomComps.size();
      std::vector<std::promise<void>> readies2(numCalls);
      std::vector<std::future<void>> registry_removes(numCalls);
      std::size_t i =0;
      for(auto pair : randomComps)
      {
        registry_removes[i] = std::async(std::launch::async,
                                         [registry, ready2, &readies2, i, pair]()
                                         {
                                           readies2[i].set_value();
                                           ready2.wait();
                                           registry->RemoveComponentManager(pair.first, pair.second);
                                         });
        i++;
      }
      for(std::size_t i =0; i< numCalls; i++)
      {
        readies2[i].get_future().wait();
      }

      go2.set_value();

      for(std::size_t i =0; i< numCalls; i++)
      {
        registry_removes[i].get();
      }
      EXPECT_EQ(registry->Count(), 0ul);
    }
    catch(...)
    {
      go2.set_value();
      throw;
    }
  }
}
} //scrimpl
} // cppmicroservices
