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

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "TestUtilBundleListener.h"
#include "TestUtils.h"

#include "gtest/gtest.h"

using namespace cppmicroservices;

class BundleActivatorTest : public ::testing::Test
{
protected:
  BundleActivatorTest()
    : f(FrameworkFactory().NewFramework())
  {
    f.Start();
    ctx = f.GetBundleContext();
  }

  ~BundleActivatorTest() { 
    f.Stop();
    f.WaitForStop(std::chrono::milliseconds::zero());
  }

  void TearDown() override
  {
    f.Stop();
    f.WaitForStop(std::chrono::milliseconds::zero());
  }

  Framework f;
  BundleContext ctx;
};

// bundle.activator property not specified in the manifest and bundle has no activator class
TEST_F(BundleActivatorTest, NoPropertyNoClass)
{
  auto bundle = cppmicroservices::testing::InstallLib(ctx, "TestBundleR");
  EXPECT_TRUE(bundle);
  EXPECT_NO_THROW(bundle.Start());
}

// bundle.activator property not specified in the manifest and bundle has an activator class
TEST_F(BundleActivatorTest, NoPropertyWithClass)
{
  auto bundle = cppmicroservices::testing::InstallLib(ctx, "TestBundleBA_X1");
  EXPECT_TRUE(bundle);
  EXPECT_NO_THROW(bundle.Start());
  // verify bundle activator was not called => service not registered
  ServiceReferenceU ref = bundle.GetBundleContext().GetServiceReference(
    "cppmicroservices::TestBundleBA_X1Service");
  EXPECT_FALSE(ref);
}

// bundle.activator property set with wrong type in the manifest and bundle has an activator class
TEST_F(BundleActivatorTest, WrongPropertyTypeWithClass)
{
  auto bundle = cppmicroservices::testing::InstallLib(ctx, "TestBundleBA_S1");
  EXPECT_TRUE(bundle);

  // Add a framework listener and verify the FrameworkEvent
  bool receivedExpectedEvent = false;
  const FrameworkListener fl = [&](const FrameworkEvent& evt) {
    std::exception_ptr eptr = evt.GetThrowable();
    if ((evt.GetType() == FrameworkEvent::FRAMEWORK_WARNING) &&
        (eptr != nullptr)) {
      try {
        std::rethrow_exception(eptr);
      } catch (const BadAnyCastException& /*ex*/) {
        receivedExpectedEvent = true;
      }
    }
  };
  ctx.AddFrameworkListener(fl);
  EXPECT_NO_THROW(bundle.Start());
  EXPECT_TRUE(receivedExpectedEvent);
  ctx.RemoveFrameworkListener(fl);
  // verify bundle activator was not called => service not registered
  ServiceReferenceU ref = bundle.GetBundleContext().GetServiceReference(
    "cppmicroservices::TestBundleBA_S1Service");
  EXPECT_FALSE(ref);
}

// bundle.activator property set to false in the manifest and bundle has no activator class
TEST_F(BundleActivatorTest, PropertyFalseWithoutClass)
{
  auto bundle = cppmicroservices::testing::InstallLib(ctx, "TestBundleBA_00");
  EXPECT_TRUE(bundle);
  EXPECT_NO_THROW(bundle.Start());
}

// bundle.activator property set to false in the manifest and bundle has an activator class
TEST_F(BundleActivatorTest, PropertyFalseWithClass)
{
  auto bundle = cppmicroservices::testing::InstallLib(ctx, "TestBundleBA_01");
  EXPECT_TRUE(bundle);
  EXPECT_NO_THROW(bundle.Start());
  // verify bundle activator was not called => service not registered
  ServiceReferenceU ref = bundle.GetBundleContext().GetServiceReference(
    "cppmicroservices::TestBundleBA_01Service");
  EXPECT_FALSE(ref);
}

// bundle.activator property set to true in the manifest and bundle has no activator class
TEST_F(BundleActivatorTest, PropertyTrueWithoutClass)
{
  auto bundle = cppmicroservices::testing::InstallLib(ctx, "TestBundleBA_10");
  EXPECT_TRUE(bundle);
  EXPECT_THROW(bundle.Start(), std::runtime_error);
}

// bundle.activator property set to true in the manifest and bundle has an activator class
TEST_F(BundleActivatorTest, PropertyTrueWithClass)
{
  auto bundle = cppmicroservices::testing::InstallLib(ctx, "TestBundleA");
  EXPECT_TRUE(bundle);
  EXPECT_NO_THROW(bundle.Start());
  // verify bundle activator was called => service is registered
  ServiceReferenceU ref = bundle.GetBundleContext().GetServiceReference(
    "cppmicroservices::TestBundleAService");
  EXPECT_TRUE(ref);
}
