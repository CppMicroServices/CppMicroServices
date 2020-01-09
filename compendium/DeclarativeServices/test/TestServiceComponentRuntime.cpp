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

#include <iostream>
#include <gtest/gtest.h>

#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>

#include "cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "TestUtils.hpp"

namespace scr = cppmicroservices::service::component::runtime;

namespace test
{
class TestServiceComponentRuntime
  : public ::testing::Test
{
protected:
  TestServiceComponentRuntime()
    : ::testing::Test()
    , framework(cppmicroservices::FrameworkFactory().NewFramework())
  { }
  virtual ~TestServiceComponentRuntime() = default;

  virtual void SetUp()
  {
    framework.Start();
  }

  virtual void TearDown()
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Framework framework;
};

/**
 * Test if the declarative services runtime publishes a service with interface
 * cppmicroservices::service::component::runtime::ServiceComponentRuntime.
 */
TEST_F(TestServiceComponentRuntime, testServiceAvailability)
{
  auto dsPluginPath = GetDSRuntimePluginFilePath();
  auto bundles = framework.GetBundleContext().InstallBundles(dsPluginPath);
  EXPECT_EQ(bundles.size(), 1ul) << "DS Runtime bundle found at" << dsPluginPath;
  for (auto bundle : bundles) {
    bundle.Start();
  }

  // verify that the DS runtime bundle publishes the expected servcie
  auto context = framework.GetBundleContext();
  auto sRef = context.GetServiceReference<scr::ServiceComponentRuntime>();
  ASSERT_TRUE(sRef);
  auto service = context.GetService<scr::ServiceComponentRuntime>(sRef);
  ASSERT_NE(service, nullptr);
}

} // end of anonymous namespace
