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

#include <gtest/gtest.h>

#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/SharedLibraryException.h"
#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleEvent.h>

#include "cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp"

#include "../src/manager/BundleLoader.hpp"

#include "TestInterfaces/Interfaces.hpp"
#include "TestUtils.hpp"

namespace scr = cppmicroservices::service::component::runtime;

class SharedLibraryExceptionTest : public ::testing::Test
{
protected:
  SharedLibraryExceptionTest()
    : framework(cppmicroservices::FrameworkFactory().NewFramework())
  {}

  virtual ~SharedLibraryExceptionTest() = default;

  virtual void SetUp()
  {
    framework.Start();
    auto context = framework.GetBundleContext();

    auto dsPluginPath = test::GetDSRuntimePluginFilePath();
    auto dsbundles = context.InstallBundles(dsPluginPath);
    for (auto& bundle : dsbundles) {
      bundle.Start();
    }

    auto sRef = context.GetServiceReference<scr::ServiceComponentRuntime>();
    ASSERT_TRUE(sRef);
    dsRuntimeService = context.GetService<scr::ServiceComponentRuntime>(sRef);
    ASSERT_TRUE(dsRuntimeService);
  }

  virtual void TearDown()
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Framework& GetFramework() { return framework; }
  std::shared_ptr<scr::ServiceComponentRuntime> dsRuntimeService;

private:
  cppmicroservices::Framework framework;
};

TEST_F(SharedLibraryExceptionTest, testDSBundleLoaderFailure)
{
  // Using the framework bundle here, as it passes the GetLocation validity test,
  // but fails at dlopen.
  ASSERT_THROW(cppmicroservices::scrimpl::GetComponentCreatorDeletors(
                 "invalid::name", GetFramework()),
               cppmicroservices::SharedLibraryException);
}

TEST_F(SharedLibraryExceptionTest, testDSBundleImmediateTrue)
{
  auto context = GetFramework().GetBundleContext();
  test::InstallLib(context, "TestBundleDSSLE1");

  auto bundles = context.GetBundles();
  auto bundle = std::find_if(
    bundles.begin(), bundles.end(), [](const cppmicroservices::Bundle& bundle) {
      return (bundle.GetSymbolicName() == "TestBundleDSSLE1");
    });

  ASSERT_NE(bundle, bundles.end()) << "TestBundleDSSLE1 not found";

  try {
    bundle->Start(); // should throw cppmicroservices::SharedLibraryException
    FAIL() << "Exception should have been caught from bundle->Start()";
  } catch (cppmicroservices::SharedLibraryException& ex) {
    // origin bundle captured by SharedLibraryException should
    // point to the bundle that threw during Start()
    ASSERT_TRUE(*bundle == ex.GetBundle());
  } catch (...) {
    FAIL()
      << "SharedLibraryException expected, but a different exception caught.";
  }
}

TEST_F(SharedLibraryExceptionTest, testDSBundleImmediateFalse)
{
  auto context = GetFramework().GetBundleContext();
  test::InstallLib(context, "TestBundleDSSLE2");

  auto bundles = context.GetBundles();
  auto bundle = std::find_if(
    bundles.begin(), bundles.end(), [](const cppmicroservices::Bundle& bundle) {
      return (bundle.GetSymbolicName() == "TestBundleDSSLE2");
    });

  ASSERT_NE(bundle, bundles.end()) << "TestBundleDSSLE2 not found";
  ASSERT_NO_THROW(bundle->Start());

  EXPECT_EQ(bundle->GetRegisteredServices().size(), 1ul)
    << "Service from TestBundleDSSLE2 must be registered by DS runtime";

  cppmicroservices::ServiceReference<test::Interface1> serviceRef =
    context.GetServiceReference<test::Interface1>();
  if (serviceRef) {
    try {
      context.GetService<test::Interface1>(
        serviceRef); // should throw cppmicroservices::SharedLibraryException
      FAIL() << "Exception should have been caught from GetService";
    } catch (const cppmicroservices::SharedLibraryException& ex) {
      // origin bundle captured by SharedLibraryException should
      // point to the bundle that threw during Start()
      ASSERT_TRUE(*bundle == ex.GetBundle());
    } catch (...) {
      FAIL()
        << "SharedLibraryException expected, but a different exception caught.";
    }
  } else {
    FAIL() << "Failed to get a valid service reference.";
  }
}
