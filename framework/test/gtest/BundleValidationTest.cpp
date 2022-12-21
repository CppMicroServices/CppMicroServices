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

#include "TestUtils.h"

#include "gtest/gtest.h"

#ifdef US_BUILD_SHARED_LIBS

TEST(BundleValidationTest, BundleValidationFailure)
{
    using validationFuncType = std::function<bool(cppmicroservices::Bundle const&)>;

    validationFuncType validationFunc = [](cppmicroservices::Bundle const&) -> bool { return false; };
    cppmicroservices::FrameworkConfiguration configuration {
        {cppmicroservices::Constants::FRAMEWORK_BUNDLE_VALIDATION_FUNC, validationFunc}
    };

    auto f = cppmicroservices::FrameworkFactory().NewFramework(std::move(configuration));
    ASSERT_NO_THROW(f.Start());

    auto bundleA = cppmicroservices::testing::InstallLib(f.GetBundleContext(), "TestBundleA");
    ASSERT_TRUE(bundleA);

    ASSERT_THROW(bundleA.Start(), cppmicroservices::SecurityException);
    // a bundle validation function which returns false must cause the
    // Framework not to start the bundle and it should not be loaded
    // into the process.
    ASSERT_EQ(bundleA.GetState(), cppmicroservices::Bundle::State::STATE_RESOLVED);

    f.Stop();
    f.WaitForStop(std::chrono::milliseconds::zero());
}

TEST(BundleValidationTest, BundleValidationSuccess)
{
    using validationFuncType = std::function<bool(cppmicroservices::Bundle const&)>;

    validationFuncType validationFunc = [](cppmicroservices::Bundle const&) -> bool { return true; };
    cppmicroservices::FrameworkConfiguration configuration {
        {cppmicroservices::Constants::FRAMEWORK_BUNDLE_VALIDATION_FUNC, validationFunc}
    };

    auto f = cppmicroservices::FrameworkFactory().NewFramework(std::move(configuration));
    ASSERT_NO_THROW(f.Start());

    auto bundleA = cppmicroservices::testing::InstallLib(f.GetBundleContext(), "TestBundleA");
    ASSERT_TRUE(bundleA);

    ASSERT_NO_THROW(bundleA.Start());
    // a bundle validation function which returns true must cause the
    // Framework to start the bundle and it should be loaded
    // into the process.
    ASSERT_EQ(bundleA.GetState(), cppmicroservices::Bundle::State::STATE_ACTIVE);

    f.Stop();
    f.WaitForStop(std::chrono::milliseconds::zero());
}

TEST(BundleValidationTest, BundleValidationFunctionException)
{
    using validationFuncType = std::function<bool(cppmicroservices::Bundle const&)>;

    validationFuncType validationFunc
        = [](cppmicroservices::Bundle const&) -> bool { throw std::runtime_error("foobar"); };
    cppmicroservices::FrameworkConfiguration configuration {
        {cppmicroservices::Constants::FRAMEWORK_BUNDLE_VALIDATION_FUNC, validationFunc}
    };

    auto f = cppmicroservices::FrameworkFactory().NewFramework(std::move(configuration));
    ASSERT_NO_THROW(f.Start());

    bool receivedBundleValidationErrorEvent { false };
    auto token = f.GetBundleContext().AddFrameworkListener(
        [&receivedBundleValidationErrorEvent](cppmicroservices::FrameworkEvent const& evt)
        {
            if (evt.GetType() == cppmicroservices::FrameworkEvent::Type::FRAMEWORK_WARNING
                && evt.GetBundle().GetSymbolicName() == "TestBundleA")
            {
                receivedBundleValidationErrorEvent = true;
            }
        });

    auto bundleA = cppmicroservices::testing::InstallLib(f.GetBundleContext(), "TestBundleA");
    ASSERT_TRUE(bundleA);

    ASSERT_THROW(bundleA.Start(), cppmicroservices::SecurityException);
    // a bundle validation function which returns false must cause the
    // Framework not to start the bundle and it should not be loaded
    // into the process.
    ASSERT_EQ(bundleA.GetState(), cppmicroservices::Bundle::State::STATE_RESOLVED);
    ASSERT_TRUE(receivedBundleValidationErrorEvent);

    f.GetBundleContext().RemoveListener(std::move(token));
    f.Stop();
    f.WaitForStop(std::chrono::milliseconds::zero());
}

#endif
