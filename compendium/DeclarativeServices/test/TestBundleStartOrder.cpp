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

#include <gtest/gtest.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/ServiceInterface.h>
#include <cppmicroservices/ServiceTracker.h>
#include <cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp>
#include <cppmicroservices/servicecomponent/ComponentConstants.hpp>
#include <TestInterfaces/Interfaces.hpp>

#include "TestFixture.hpp"
#include "TestUtils.hpp"

namespace scr = cppmicroservices::service::component::runtime;

namespace test {

class TestBundleStartOrder
  : public ::testing::Test
{
protected:
  TestBundleStartOrder()
    : ::testing::Test()
    , framework(cppmicroservices::FrameworkFactory().NewFramework())
  { }
  virtual ~TestBundleStartOrder() = default;

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

class BundleName final
{
public:
  BundleName(std::string name)
    : lookingFor(std::move(name))
  {}

  ~BundleName() = default;
  
  bool operator()(const cppmicroservices::Bundle& b)
  {
    auto symbolicName = b.GetSymbolicName();
    return (lookingFor == symbolicName);
  }
private:
  std::string lookingFor;
};
  

/**
 * Test if DS detects already active bundles with component descriptions
 */
TEST_F(TestBundleStartOrder, testRuntimeWithAlreadyStartedBundles) // DS_TOI_55
{
  auto context = framework.GetBundleContext();
  ASSERT_EQ(context.GetBundles().size(), 2ul) << "Framework+executable must be the only bundles in the installed bundles list";
  // install & start some test bundles
  test::InstallLib(context, "TestBundleDSTOI1");
  test::InstallLib(context, "TestBundleDSTOI10");
  auto bundles = context.GetBundles();
  EXPECT_EQ(bundles.size(), 4ul) << "Test bundles must be in installed bundles list";
  
  auto bundle1 = std::find_if(std::begin(bundles), std::end(bundles), BundleName("TestBundleDSTOI1"));
  ASSERT_NE(bundle1, bundles.end()) << "TestBundleDSTOI1 not found";
  bundle1->Start();
  EXPECT_TRUE(bundle1->GetRegisteredServices().empty()) << "Service from TestBundleDS_TOI_1 must not be available since the DS runtime is not active yet";
  
  auto bundle2 = std::find_if(std::begin(bundles), std::end(bundles), BundleName("TestBundleDSTOI10"));
  ASSERT_NE(bundle2, bundles.end()) << "TestBundleDSTOI10 not found";
  bundle2->Start();
  EXPECT_TRUE(bundle2->GetRegisteredServices().empty()) << "Service from TestBundleDS_TOI_10 must not be available since the DS runtime is not actiev yet";

  auto dsPluginPath = test::GetDSRuntimePluginFilePath();
  auto dsbundles = context.InstallBundles(dsPluginPath);
  for (auto& bundle : dsbundles)
  {
    bundle.Start();
  }

  // Verify that already started DS bundles are recognized by DS Runtime
  auto sRef = context.GetServiceReference<scr::ServiceComponentRuntime>();
  if (sRef)
  {
    auto service = context.GetService<scr::ServiceComponentRuntime>(sRef);
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->GetComponentDescriptionDTOs().size(), 2ul) << "Components from active bundles must be loaded by DS";
  }
  
  EXPECT_EQ(bundle1->GetRegisteredServices().size(), 1ul) << "Service from TestBundleDS_TOI_1 must be registered by DS runtime";
  EXPECT_EQ(bundle2->GetRegisteredServices().size(), 1ul) << "Service from TestBundleDS_TOI_10 must be registered by DS runtime";
}

/**
 * Test if DS detects already newly started bundles with component descriptions
 */
TEST_F(TestBundleStartOrder, testRuntimeWithNewlyStartedBundles) // DS_TOI_56
{
  // install & start some test bundles
  auto context = framework.GetBundleContext();
  auto dsPluginPath = test::GetDSRuntimePluginFilePath();
  auto dsbundles = context.InstallBundles(dsPluginPath);
  for (auto& bundle : dsbundles)
  {
    bundle.Start();
  }

  test::InstallLib(context, "TestBundleDSTOI1");
  test::InstallLib(context, "TestBundleDSTOI10");
  auto bundles = context.GetBundles();
  EXPECT_NE(bundles.size(), 0ul) << "No bundles installed";
  
  auto bundle1 = std::find_if(std::begin(bundles), std::end(bundles), BundleName("TestBundleDSTOI1"));
  ASSERT_NE(bundle1, bundles.end()) << "TestBundleDSTOI1 not found";
  bundle1->Start();
  EXPECT_EQ(bundle1->GetRegisteredServices().size(), 1ul) << "Service from TestBundleDS_TOI_1 must be registered by DS runtime";
  
  auto bundle2 = std::find_if(std::begin(bundles), std::end(bundles), BundleName("TestBundleDSTOI10"));
  ASSERT_NE(bundle2, bundles.end()) << "TestBundleDSTOI10 not found";
  bundle2->Start();
  EXPECT_EQ(bundle2->GetRegisteredServices().size(), 1ul) << "Service from TestBundleDS_TOI_10 must be registered by DS runtime";

  // Verify that already started DS bundles are recognized by DS Runtime
  auto sRef = context.GetServiceReference<scr::ServiceComponentRuntime>();
  ASSERT_TRUE(sRef) << "No ServiceComponentRuntime found";
  auto service = context.GetService<scr::ServiceComponentRuntime>(sRef);
  ASSERT_NE(service, nullptr);
  EXPECT_EQ(service->GetComponentDescriptionDTOs().size(), 2ul) << "Components from active bundles must be loaded by DS";
}

TEST_F(TestBundleStartOrder, testDSBundleResolution)
{
  auto context = framework.GetBundleContext();
  ASSERT_EQ(context.GetBundles().size(), 2ul) << "Framework+executable must be the only bundles in the installed bundles list";
  
  // install & start some test bundles
  test::InstallLib(context, "DSGraph01");
  test::InstallLib(context, "DSGraph02");
  test::InstallLib(context, "DSGraph03");
  test::InstallLib(context, "DSGraph04");
  test::InstallLib(context, "DSGraph05");
  test::InstallLib(context, "DSGraph06");
  test::InstallLib(context, "DSGraph07");
  ASSERT_EQ(context.GetBundles().size(), 9ul) << "Framework+executable must be the only bundles in the installed bundles list";
  auto dsPluginPath = test::GetDSRuntimePluginFilePath();
  context.InstallBundles(dsPluginPath);
  ASSERT_EQ(context.GetBundles().size(), 10ul) << "Framework+executable must be the only bundles in the installed bundles list";

  auto bundles = context.GetBundles();
  for (auto& bundle : bundles)
  {
    bundle.Start();
  }

  auto bundle = std::find_if(std::begin(bundles), std::end(bundles), BundleName("DSGraph01"));
  ASSERT_NE(bundle, bundles.end()) << "DSGraph01 not found";

  auto const ref = context.GetServiceReference<test::DSGraph01>();
  EXPECT_TRUE(ref);
  auto const service = context.GetService(ref);
  EXPECT_TRUE(service);
}

} // end of anonymous namespace
