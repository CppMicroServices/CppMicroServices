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

#include "cppmicroservices/Any.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/SecurityException.h"

#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp"

#include "TestInterfaces/Interfaces.hpp"
#include "TestUtils.hpp"

#include "gtest/gtest.h"

TEST(TestBundleValidation, BundleValidationFailure)
{
  using validationFuncType = std::function<bool(const std::string&)>;

  validationFuncType validationFunc = [](const std::string& path) -> bool {
    if (std::string::npos != path.find("DeclarativeServices")) {
      return true;
    }
    return false;
  };
  cppmicroservices::FrameworkConfiguration configuration{
    { cppmicroservices::Constants::FRAMEWORK_BUNDLE_VALIDATION_FUNC,
      validationFunc }
  };

  auto f =
    cppmicroservices::FrameworkFactory().NewFramework(std::move(configuration));
  ASSERT_NO_THROW(f.Start());

  test::InstallAndStartDS(f.GetBundleContext());

  auto sDSSvcRef =
    f.GetBundleContext().GetServiceReference<cppmicroservices::service::component::runtime::ServiceComponentRuntime>();
  ASSERT_TRUE(sDSSvcRef);
  auto dsRuntimeService =
    f.GetBundleContext()
      .GetService<
        cppmicroservices::service::component::runtime::ServiceComponentRuntime>(
        sDSSvcRef);
  ASSERT_TRUE(dsRuntimeService);

  // test starting an "immediate" ds component
  // in this case, starting the bundle causes the shared library to load
  test::InstallLib(f.GetBundleContext(), "TestBundleDSTOI1");
  auto bundles = f.GetBundleContext().GetBundles();
  auto bundleIter = std::find_if(
    bundles.begin(), bundles.end(), [](const cppmicroservices::Bundle& b) {
      return (b.GetSymbolicName() == "TestBundleDSTOI1");
    });

  ASSERT_THROW(bundleIter->Start(), cppmicroservices::SecurityException);
  // a bundle validation function which returns false must cause the
  // Framework not to start the bundle and it should not be loaded
  // into the process.
  EXPECT_EQ(bundleIter->GetState(),
            cppmicroservices::Bundle::State::STATE_RESOLVED);

  // test starting a delayed activation ds component
  // in this case, starting the bundle does not cause the shared library to load
  // the shared library is loaded on the first call to "GetService"
  test::InstallLib(f.GetBundleContext(), "TestBundleDSTOI6");
  bundles = f.GetBundleContext().GetBundles();
  bundleIter = std::find_if(
    bundles.begin(), bundles.end(), [](const cppmicroservices::Bundle& b) {
      return (b.GetSymbolicName() == "TestBundleDSTOI6");
    });

  ASSERT_NO_THROW(bundleIter->Start());
  
  struct Interface1Impl final : public test::Interface1
  {
    std::string Description() override { return "foo"; }
  };
  f.GetBundleContext().RegisterService<test::Interface1>(std::make_shared<Interface1Impl>());

  auto svcRef = f.GetBundleContext().GetServiceReference<test::Interface2>();
  ASSERT_TRUE(svcRef);
  ASSERT_THROW(auto svcObj = f.GetBundleContext().GetService(svcRef), cppmicroservices::SecurityException);

  // a bundle validation function which returns false must cause the
  // service component not to be enabled
  auto compDesc = dsRuntimeService->GetComponentDescriptionDTO(*bundleIter, "sample::ServiceComponent6");
  ASSERT_FALSE(dsRuntimeService->IsComponentEnabled(compDesc));

  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());
}

TEST(TestBundleValidation, BundleValidationSuccess)
{
  using validationFuncType = std::function<bool(const std::string&)>;

  validationFuncType validationFunc = [](const std::string&) -> bool {
    return true;
  };
  cppmicroservices::FrameworkConfiguration configuration{
    { cppmicroservices::Constants::FRAMEWORK_BUNDLE_VALIDATION_FUNC,
      validationFunc }
  };

  auto f =
    cppmicroservices::FrameworkFactory().NewFramework(std::move(configuration));
  ASSERT_NO_THROW(f.Start());

  test::InstallAndStartDS(f.GetBundleContext());

  test::InstallLib(f.GetBundleContext(), "TestBundleDSTOI1");
  auto bundles = f.GetBundleContext().GetBundles();
  auto bundleIter = std::find_if(
    bundles.begin(), bundles.end(), [](const cppmicroservices::Bundle& b) {
      return (b.GetSymbolicName() == "TestBundleDSTOI1");
    });

  ASSERT_NO_THROW(bundleIter->Start());
  // a bundle validation function which returns true must cause the
  // Framework to start the bundle and it should be loaded
  // into the process.
  ASSERT_EQ(bundleIter->GetState(), cppmicroservices::Bundle::State::STATE_ACTIVE);

  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());
}

TEST(TestBundleValidation, BundleValidationFunctionException)
{
  using validationFuncType = std::function<bool(const std::string&)>;

  validationFuncType validationFunc = [](const std::string& path) -> bool {
    if (std::string::npos != path.find("DeclarativeServices")) {
      return true;
    }
    throw std::runtime_error("foobar");
  };
  cppmicroservices::FrameworkConfiguration configuration{
    { cppmicroservices::Constants::FRAMEWORK_BUNDLE_VALIDATION_FUNC,
      validationFunc }
  };

  auto f =
    cppmicroservices::FrameworkFactory().NewFramework(std::move(configuration));
  ASSERT_NO_THROW(f.Start());

  bool receivedBundleValidationErrorEvent{ false };
  bool receivedSecondBundleValidationErrorEvent{ false };
  auto token = f.GetBundleContext().AddFrameworkListener(
    [&receivedBundleValidationErrorEvent, &receivedSecondBundleValidationErrorEvent](
      const cppmicroservices::FrameworkEvent& evt) {
      if (evt.GetType() ==
            cppmicroservices::FrameworkEvent::Type::FRAMEWORK_ERROR &&
          evt.GetBundle().GetSymbolicName() == "TestBundleDSTOI1") {
        receivedBundleValidationErrorEvent = true;
      }

      if (evt.GetType() ==
            cppmicroservices::FrameworkEvent::Type::FRAMEWORK_ERROR &&
          evt.GetBundle().GetSymbolicName() == "TestBundleDSTOI6") {
        receivedSecondBundleValidationErrorEvent = true;
      }
    });

  test::InstallAndStartDS(f.GetBundleContext());

  auto sDSSvcRef = f.GetBundleContext()
                     .GetServiceReference<cppmicroservices::service::component::
                                            runtime::ServiceComponentRuntime>();
  ASSERT_TRUE(sDSSvcRef);
  auto dsRuntimeService =
    f.GetBundleContext()
      .GetService<
        cppmicroservices::service::component::runtime::ServiceComponentRuntime>(
        sDSSvcRef);
  ASSERT_TRUE(dsRuntimeService);

  test::InstallLib(f.GetBundleContext(), "TestBundleDSTOI1");
  auto bundles = f.GetBundleContext().GetBundles();
  auto bundleIter = std::find_if(
    bundles.begin(), bundles.end(), [](const cppmicroservices::Bundle& b) {
      return (b.GetSymbolicName() == "TestBundleDSTOI1");
    });

  ASSERT_THROW(bundleIter->Start(), cppmicroservices::SecurityException);
  // a bundle validation function which returns false must cause the
  // Framework not to start the bundle and it should not be loaded
  // into the process.
  EXPECT_EQ(bundleIter->GetState(),
            cppmicroservices::Bundle::State::STATE_RESOLVED);
  ASSERT_TRUE(receivedBundleValidationErrorEvent);

  // test starting a delayed activation ds component
  // in this case, starting the bundle does not cause the shared library to load
  // the shared library is loaded on the first call to "GetService"
  test::InstallLib(f.GetBundleContext(), "TestBundleDSTOI6");
  bundles = f.GetBundleContext().GetBundles();
  bundleIter = std::find_if(
    bundles.begin(), bundles.end(), [](const cppmicroservices::Bundle& b) {
      return (b.GetSymbolicName() == "TestBundleDSTOI6");
    });

  ASSERT_NO_THROW(bundleIter->Start());

  struct Interface1Impl final : public test::Interface1
  {
    std::string Description() override { return "foo"; }
  };
  f.GetBundleContext().RegisterService<test::Interface1>(
    std::make_shared<Interface1Impl>());

  auto svcRef = f.GetBundleContext().GetServiceReference<test::Interface2>();
  ASSERT_TRUE(svcRef);
  ASSERT_THROW(auto svcObj = f.GetBundleContext().GetService(svcRef),
               cppmicroservices::SecurityException);

  // a bundle validation function which returns false must cause the
  // service component not to be enabled
  auto compDesc = dsRuntimeService->GetComponentDescriptionDTO(
    *bundleIter, "sample::ServiceComponent6");
  ASSERT_FALSE(dsRuntimeService->IsComponentEnabled(compDesc));
  ASSERT_TRUE(receivedSecondBundleValidationErrorEvent);

  f.GetBundleContext().RemoveListener(std::move(token));
  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());
}
