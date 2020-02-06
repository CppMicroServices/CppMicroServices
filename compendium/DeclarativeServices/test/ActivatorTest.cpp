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

#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleImport.h"
#include "cppmicroservices/Constants.h"
#include "../src/SCRActivator.hpp"

#include "Mocks.hpp"

#define xstr(s) str(s)
#define str(s) #s

using cppmicroservices::scrimpl::SCRActivator;

namespace cppmicroservices{
namespace scrimpl {

// The fixture for testing class SCRActivator.
class ActivatorTest
  : public ::testing::Test
{
protected:
  ActivatorTest()
    : framework(cppmicroservices::FrameworkFactory().NewFramework())
  { }
      
  virtual ~ActivatorTest() = default;

  virtual void SetUp()
  {
    framework.Start();
  }

  virtual void TearDown()
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Framework& GetFramework() { return framework; }
private:
  cppmicroservices::Framework framework;
};

TEST_F(ActivatorTest, VerifySCRService)
{
  EXPECT_NO_THROW({
      auto bundleContext = GetFramework().GetBundleContext();
      auto allBundles = bundleContext.GetBundles();
      EXPECT_EQ(allBundles.size(), static_cast<size_t>(2));
      cppmicroservices::Bundle selfBundle;
      for (const auto& bundle : allBundles)
      {
        if (bundle.GetSymbolicName() == str(US_BUNDLE_NAME))
        {
          selfBundle = bundle;
          break;
        }
      }
      if (selfBundle)
      {
        selfBundle.Start();
        EXPECT_EQ(selfBundle.GetState(), cppmicroservices::Bundle::STATE_ACTIVE);
        auto selfContext = selfBundle.GetBundleContext();
        EXPECT_EQ(selfBundle.GetRegisteredServices().size(), static_cast<size_t>(1));
        cppmicroservices::ServiceReference<ServiceComponentRuntime> serviceRef = bundleContext.GetServiceReference<ServiceComponentRuntime>();
        std::shared_ptr<ServiceComponentRuntime> service = bundleContext.GetService<ServiceComponentRuntime>(serviceRef);
        EXPECT_NE(service.get(), nullptr);
      }
    });
}

TEST_F(ActivatorTest, VerifyConcurrentStartStop) {
  auto bundleContext = GetFramework().GetBundleContext();
  auto allBundles = bundleContext.GetBundles();
  EXPECT_EQ(allBundles.size(), static_cast<size_t>(2));
  cppmicroservices::Bundle selfBundle;
  for(auto bundle : allBundles)
  {
    if(bundle.GetSymbolicName() == xstr(US_BUNDLE_NAME))
    {
      selfBundle = bundle;
      break;
    }
  }
  std::promise<void> go;
  std::shared_future<void> ready(go.get_future());
  int numCalls = 50;
  std::vector<std::promise<void>> readies(numCalls);
  std::vector<std::future<void>> bundle_state_changes(numCalls);
  try {
    for(int i =0; i<numCalls; i++)
    {
      bundle_state_changes[i] = std::async(std::launch::async,
                                           [&selfBundle, ready, &readies, i]()
                                           {
                                             readies[i].set_value();
                                             ready.wait();
                                             ((i%2) ? selfBundle.Start() :selfBundle.Stop());
                                           });
    }

    for(int i =0; i< numCalls; i++)
    {
      readies[i].get_future().wait();
    }
    go.set_value();
    for(int i =0; i< numCalls; i++)
    {
      bundle_state_changes[i].wait();
    }
  }
  catch(const std::exception& e)
  {
    EXPECT_TRUE(false) << "Error: exception received ... " << e.what() << std::endl;
    go.set_value();
    throw std::current_exception();
  }
}
}
}

CPPMICROSERVICES_IMPORT_BUNDLE(US_BUNDLE_NAME);
