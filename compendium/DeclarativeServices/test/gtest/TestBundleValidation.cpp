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
#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/SecurityException.h"

#include "cppmicroservices/cm/Configuration.hpp"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"

#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp"

#include "../TestUtils.hpp"
#include "TestInterfaces/Interfaces.hpp"

#include "gtest/gtest.h"

#include <unordered_set>

TEST(TestBundleValidation, BundleValidationFailure)
{
    using validationFuncType = std::function<bool(cppmicroservices::Bundle const&)>;

    validationFuncType validationFunc = [](cppmicroservices::Bundle const& b) -> bool
    {
        if (b.GetSymbolicName() == "declarative_services" || b.GetSymbolicName() == "configuration_admin")
        {
            return true;
        }
        return false;
    };
    cppmicroservices::FrameworkConfiguration configuration {
        { cppmicroservices::Constants::FRAMEWORK_BUNDLE_VALIDATION_FUNC, validationFunc }
    };

    auto f = cppmicroservices::FrameworkFactory().NewFramework(std::move(configuration));
    ASSERT_NO_THROW(f.Start());

    test::InstallAndStartDS(f.GetBundleContext());

    auto sDSSvcRef = f.GetBundleContext()
                         .GetServiceReference<cppmicroservices::service::component::runtime::ServiceComponentRuntime>();
    ASSERT_TRUE(sDSSvcRef);
    auto dsRuntimeService
        = f.GetBundleContext().GetService<cppmicroservices::service::component::runtime::ServiceComponentRuntime>(
            sDSSvcRef);
    ASSERT_TRUE(dsRuntimeService);

    // test starting an "immediate" ds component
    // in this case, starting the bundle causes the shared library to load
    test::InstallLib(f.GetBundleContext(), "TestBundleDSTBV1");
    auto bundles = f.GetBundleContext().GetBundles();
    auto bundleIter
        = std::find_if(bundles.begin(),
                       bundles.end(),
                       [](cppmicroservices::Bundle const& b) { return (b.GetSymbolicName() == "TestBundleDSTBV1"); });

    ASSERT_THROW(bundleIter->Start(), cppmicroservices::SecurityException);
    // a bundle validation function which returns false must cause the
    // Framework not to start the bundle and it should not be loaded
    // into the process.
    EXPECT_EQ(bundleIter->GetState(), cppmicroservices::Bundle::State::STATE_RESOLVED);

    // test starting a delayed activation ds component with a service dependency
    // in this case, starting the bundle does not cause the shared library to load
    // the shared library is loaded on the first call to "GetService"
    test::InstallLib(f.GetBundleContext(), "TestBundleDSTBV1_1");
    bundles = f.GetBundleContext().GetBundles();
    bundleIter
        = std::find_if(bundles.begin(),
                       bundles.end(),
                       [](cppmicroservices::Bundle const& b) { return (b.GetSymbolicName() == "TestBundleDSTBV1_1"); });

    ASSERT_NO_THROW(bundleIter->Start());

    struct Interface1Impl final : public test::Interface1
    {
        std::string
        Description() override
        {
            return "foo";
        }
    };
    auto Interface1SvcReg = f.GetBundleContext().RegisterService<test::Interface1>(std::make_shared<Interface1Impl>());

    auto svcRef = f.GetBundleContext().GetServiceReference<test::Interface2>();
    ASSERT_TRUE(svcRef);
    ASSERT_THROW(auto svcObj = f.GetBundleContext().GetService(svcRef), cppmicroservices::SecurityException);

    // a bundle validation function which returns false must cause the
    // service component not to be enabled
    auto compDesc = dsRuntimeService->GetComponentDescriptionDTO(*bundleIter, "sample::ServiceComponentBV1_1");
    ASSERT_FALSE(dsRuntimeService->IsComponentEnabled(compDesc));

    // delayed components won't throw when enabled. they should throw on first
    // request of the service
    auto enabledFuture = dsRuntimeService->EnableComponent(compDesc);
    ASSERT_NO_THROW(enabledFuture.get());

    svcRef = f.GetBundleContext().GetServiceReference<test::Interface2>();
    ASSERT_TRUE(svcRef);
    ASSERT_THROW(auto svcObj = f.GetBundleContext().GetService(svcRef), cppmicroservices::SecurityException);

    compDesc = dsRuntimeService->GetComponentDescriptionDTO(*bundleIter, "sample::ServiceComponentBV1_1");
    ASSERT_FALSE(dsRuntimeService->IsComponentEnabled(compDesc));
    Interface1SvcReg.Unregister();

    // test starting an immediate activation ds component with a service reference
    test::InstallLib(f.GetBundleContext(), "TestBundleDSTBV1_2");
    bundles = f.GetBundleContext().GetBundles();
    bundleIter
        = std::find_if(bundles.begin(),
                       bundles.end(),
                       [](cppmicroservices::Bundle const& b) { return (b.GetSymbolicName() == "TestBundleDSTBV1_2"); });

    // on bundle start, the ds component will not be activated immediately since
    // it's service reference is unsatisfied. Registering a service which satisfies
    // the reference should cause an exception and no service should be registered.
    ASSERT_NO_THROW(bundleIter->Start());
    compDesc = dsRuntimeService->GetComponentDescriptionDTO(*bundleIter, "sample::ServiceComponentBV1_2");
    ASSERT_TRUE(dsRuntimeService->IsComponentEnabled(compDesc));
    // trying to enable the component will result in an exception from
    // the future since the ds component was immediately activated
    Interface1SvcReg = f.GetBundleContext().RegisterService<test::Interface1>(std::make_shared<Interface1Impl>());
    svcRef = f.GetBundleContext().GetServiceReference<test::Interface2>();
    ASSERT_TRUE(svcRef);
    ASSERT_THROW(auto svcObj = f.GetBundleContext().GetService(svcRef), cppmicroservices::SecurityException);
    ASSERT_FALSE(dsRuntimeService->IsComponentEnabled(compDesc));
    auto enableCompFuture = dsRuntimeService->EnableComponent(compDesc);
    ASSERT_THROW(enableCompFuture.get(), cppmicroservices::SecurityException);
    Interface1SvcReg.Unregister();

    // test starting a prototype scope service component
    test::InstallLib(f.GetBundleContext(), "TestBundleDSTBV1_3");
    bundles = f.GetBundleContext().GetBundles();
    bundleIter
        = std::find_if(bundles.begin(),
                       bundles.end(),
                       [](cppmicroservices::Bundle const& b) { return (b.GetSymbolicName() == "TestBundleDSTBV1_3"); });
    ASSERT_NO_THROW(bundleIter->Start());
    compDesc = dsRuntimeService->GetComponentDescriptionDTO(*bundleIter, "sample::ServiceComponentBV1_3");
    ASSERT_TRUE(dsRuntimeService->IsComponentEnabled(compDesc));
    svcRef = f.GetBundleContext().GetServiceReference<test::Interface1>();
    ASSERT_TRUE(svcRef);
    ASSERT_THROW(auto svcObj = f.GetBundleContext().GetService(svcRef), cppmicroservices::SecurityException);
    ASSERT_FALSE(dsRuntimeService->IsComponentEnabled(compDesc));

    // test starting a delayed activation ds component with a required configuration policy
    test::InstallLib(f.GetBundleContext(), "TestBundleDSTBV1_4");
    bundles = f.GetBundleContext().GetBundles();
    bundleIter
        = std::find_if(bundles.begin(),
                       bundles.end(),
                       [](cppmicroservices::Bundle const& b) { return (b.GetSymbolicName() == "TestBundleDSTBV1_4"); });
    ASSERT_NO_THROW(bundleIter->Start());
    compDesc = dsRuntimeService->GetComponentDescriptionDTO(*bundleIter, "sample::ServiceComponentBV1_4");
    ASSERT_TRUE(dsRuntimeService->IsComponentEnabled(compDesc));

    auto cmBundlePath = test::GetConfigAdminRuntimePluginFilePath();
    auto cmBundle = f.GetBundleContext().InstallBundles(cmBundlePath);
    ASSERT_TRUE(cmBundle[0]);
    ASSERT_NO_THROW(cmBundle[0].Start());

    auto sCMSvcRef = f.GetBundleContext().GetServiceReference<cppmicroservices::service::cm::ConfigurationAdmin>();
    ASSERT_TRUE(sCMSvcRef);
    auto cmRuntimeService
        = f.GetBundleContext().GetService<cppmicroservices::service::cm::ConfigurationAdmin>(sCMSvcRef);
    ASSERT_TRUE(cmRuntimeService);

    auto config = cmRuntimeService->GetConfiguration("sample::ServiceComponentBV1_4");
    cppmicroservices::AnyMap configObj(cppmicroservices::AnyMap::UNORDERED_MAP);
    configObj["foo"] = std::string("bar");
    auto updateFuture = config->Update(configObj);
    ASSERT_NO_THROW(updateFuture.get());

    configObj["foo"] = std::string("baz");
    auto updateIfDifferentFuture = config->UpdateIfDifferent(configObj);
    ASSERT_NO_THROW(updateIfDifferentFuture.second.get());

    svcRef = f.GetBundleContext().GetServiceReference<test::CAInterface>();
    ASSERT_TRUE(svcRef);
    ASSERT_THROW(auto svcObj = f.GetBundleContext().GetService(svcRef), cppmicroservices::SecurityException);

    compDesc = dsRuntimeService->GetComponentDescriptionDTO(*bundleIter, "sample::ServiceComponentBV1_4");
    ASSERT_FALSE(dsRuntimeService->IsComponentEnabled(compDesc));
    config->Remove().get();

    // test starting an immediate activation ds component with a required configuration policy
    test::InstallLib(f.GetBundleContext(), "TestBundleDSTBV1_5");
    bundles = f.GetBundleContext().GetBundles();
    bundleIter
        = std::find_if(bundles.begin(),
                       bundles.end(),
                       [](cppmicroservices::Bundle const& b) { return (b.GetSymbolicName() == "TestBundleDSTBV1_5"); });
    ASSERT_NO_THROW(bundleIter->Start());
    compDesc = dsRuntimeService->GetComponentDescriptionDTO(*bundleIter, "sample::ServiceComponentBV1_5");
    ASSERT_TRUE(dsRuntimeService->IsComponentEnabled(compDesc));
    config = cmRuntimeService->GetConfiguration("sample::ServiceComponentBV1_5");
    configObj.clear();
    configObj["foo"] = std::string("bar");
    ASSERT_THROW(config->Update(configObj).get(), cppmicroservices::SecurityException);

    configObj["foo"] = std::string("baz");
    ASSERT_THROW(config->UpdateIfDifferent(configObj).second.get(), cppmicroservices::SecurityException);

    svcRef = f.GetBundleContext().GetServiceReference<test::CAInterface>();
    ASSERT_TRUE(svcRef);
    ASSERT_THROW(auto svcObj = f.GetBundleContext().GetService(svcRef), cppmicroservices::SecurityException);

    compDesc = dsRuntimeService->GetComponentDescriptionDTO(*bundleIter, "sample::ServiceComponentBV1_5");
    ASSERT_FALSE(dsRuntimeService->IsComponentEnabled(compDesc));

    f.Stop();
    f.WaitForStop(std::chrono::milliseconds::zero());
}

TEST(TestBundleValidation, BundleValidationSuccess)
{
    using validationFuncType = std::function<bool(cppmicroservices::Bundle const&)>;

    validationFuncType validationFunc = [](cppmicroservices::Bundle const&) -> bool { return true; };
    cppmicroservices::FrameworkConfiguration configuration {
        { cppmicroservices::Constants::FRAMEWORK_BUNDLE_VALIDATION_FUNC, validationFunc }
    };

    auto f = cppmicroservices::FrameworkFactory().NewFramework(std::move(configuration));
    ASSERT_NO_THROW(f.Start());

    test::InstallAndStartDS(f.GetBundleContext());

    test::InstallLib(f.GetBundleContext(), "TestBundleDSTBV2");
    auto bundles = f.GetBundleContext().GetBundles();
    auto bundleIter
        = std::find_if(bundles.begin(),
                       bundles.end(),
                       [](cppmicroservices::Bundle const& b) { return (b.GetSymbolicName() == "TestBundleDSTBV2"); });

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
    using validationFuncType = std::function<bool(cppmicroservices::Bundle const&)>;

    validationFuncType validationFunc = [](cppmicroservices::Bundle const& b) -> bool
    {
        if (b.GetSymbolicName() == "declarative_services")
        {
            return true;
        }
        throw std::runtime_error("foobar");
    };
    cppmicroservices::FrameworkConfiguration configuration {
        { cppmicroservices::Constants::FRAMEWORK_BUNDLE_VALIDATION_FUNC, validationFunc }
    };

    auto f = cppmicroservices::FrameworkFactory().NewFramework(std::move(configuration));
    ASSERT_NO_THROW(f.Start());

    bool receivedBundleValidationErrorEvent { false };
    bool receivedSecondBundleValidationErrorEvent { false };
    auto token = f.GetBundleContext().AddFrameworkListener(
        [&receivedBundleValidationErrorEvent,
         &receivedSecondBundleValidationErrorEvent](cppmicroservices::FrameworkEvent const& evt)
        {
            if (evt.GetType() == cppmicroservices::FrameworkEvent::Type::FRAMEWORK_ERROR
                && evt.GetBundle().GetSymbolicName() == "TestBundleDSTBV3")
            {
                receivedBundleValidationErrorEvent = true;
            }

            if (evt.GetType() == cppmicroservices::FrameworkEvent::Type::FRAMEWORK_ERROR
                && evt.GetBundle().GetSymbolicName() == "TestBundleDSTBV3_1")
            {
                receivedSecondBundleValidationErrorEvent = true;
            }
        });

    test::InstallAndStartDS(f.GetBundleContext());

    auto sDSSvcRef = f.GetBundleContext()
                         .GetServiceReference<cppmicroservices::service::component::runtime::ServiceComponentRuntime>();
    ASSERT_TRUE(sDSSvcRef);
    auto dsRuntimeService
        = f.GetBundleContext().GetService<cppmicroservices::service::component::runtime::ServiceComponentRuntime>(
            sDSSvcRef);
    ASSERT_TRUE(dsRuntimeService);

    test::InstallLib(f.GetBundleContext(), "TestBundleDSTBV3");
    auto bundles = f.GetBundleContext().GetBundles();
    auto bundleIter
        = std::find_if(bundles.begin(),
                       bundles.end(),
                       [](cppmicroservices::Bundle const& b) { return (b.GetSymbolicName() == "TestBundleDSTBV3"); });

    ASSERT_THROW(bundleIter->Start(), cppmicroservices::SecurityException);
    // a bundle validation function which returns false must cause the
    // Framework not to start the bundle and it should not be loaded
    // into the process.
    EXPECT_EQ(bundleIter->GetState(), cppmicroservices::Bundle::State::STATE_RESOLVED);
    ASSERT_TRUE(receivedBundleValidationErrorEvent);

    // test starting a delayed activation ds component
    // in this case, starting the bundle does not cause the shared library to load
    // the shared library is loaded on the first call to "GetService"
    test::InstallLib(f.GetBundleContext(), "TestBundleDSTBV3_1");
    bundles = f.GetBundleContext().GetBundles();
    bundleIter
        = std::find_if(bundles.begin(),
                       bundles.end(),
                       [](cppmicroservices::Bundle const& b) { return (b.GetSymbolicName() == "TestBundleDSTBV3_1"); });

    ASSERT_NO_THROW(bundleIter->Start());

    struct Interface1Impl final : public test::Interface1
    {
        std::string
        Description() override
        {
            return "foo";
        }
    };
    f.GetBundleContext().RegisterService<test::Interface1>(std::make_shared<Interface1Impl>());

    auto svcRef = f.GetBundleContext().GetServiceReference<test::Interface2>();
    ASSERT_TRUE(svcRef);
    ASSERT_THROW(auto svcObj = f.GetBundleContext().GetService(svcRef), cppmicroservices::SecurityException);

    // a bundle validation function which returns false must cause the
    // service component not to be enabled
    auto compDesc = dsRuntimeService->GetComponentDescriptionDTO(*bundleIter, "sample::ServiceComponentBV3_1");
    ASSERT_FALSE(dsRuntimeService->IsComponentEnabled(compDesc));
    ASSERT_TRUE(receivedSecondBundleValidationErrorEvent);

    f.GetBundleContext().RemoveListener(std::move(token));
    f.Stop();
    f.WaitForStop(std::chrono::milliseconds::zero());
}

/*
 * Verify the absence of redundant bundle validation checks by installing a
 * bundle with two services.
 */
TEST(TestBundleValidation, BundleValidationMultipleSrc)
{
    using validationFuncType = std::function<bool(cppmicroservices::Bundle const&)>;

    // Check for multiple calls for the same bundle
    validationFuncType validationFunc = [](cppmicroservices::Bundle const& bundle) -> bool
    {
        static std::unordered_set<std::string> validBundles;
        auto const bundleLoc = bundle.GetLocation();

        if (validBundles.count(bundleLoc) == 1)
        {
            return false;
        }
        else
        {
            validBundles.emplace(bundleLoc);
            return true;
        }
    };
    cppmicroservices::FrameworkConfiguration frameworkConfiguration {
        { cppmicroservices::Constants::FRAMEWORK_BUNDLE_VALIDATION_FUNC, validationFunc }
    };
    auto framework = cppmicroservices::FrameworkFactory().NewFramework(std::move(frameworkConfiguration));

    ASSERT_NO_THROW(framework.Start());

    test::InstallAndStartDS(framework.GetBundleContext());
    test::InstallLib(framework.GetBundleContext(), "TestBundleDSTBV4");

    auto bundles = framework.GetBundleContext().GetBundles();
    auto bundleIter
        = std::find_if(bundles.begin(),
                       bundles.end(),
                       [](cppmicroservices::Bundle const& b) { return (b.GetSymbolicName() == "TestBundleDSTBV4"); });

    ASSERT_NO_THROW(bundleIter->Start());
    ASSERT_EQ(bundleIter->GetState(), cppmicroservices::Bundle::State::STATE_ACTIVE);

    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
}

/*
 * Verfiy the absence of redundant bundle validation on reinstalling a bundle.
 */
TEST(TestBundleValidation, BundleValidationReinstall)
{
    using validationFuncType = std::function<bool(cppmicroservices::Bundle const&)>;

    // Check for multiple calls for the same bundle
    validationFuncType validationFunc = [](cppmicroservices::Bundle const& bundle) -> bool
    {
        static std::unordered_set<std::string> validBundles;
        auto const bundleLoc = bundle.GetLocation();

        if (validBundles.count(bundleLoc) == 1)
        {
            return false;
        }
        else
        {
            validBundles.emplace(bundleLoc);
            return true;
        }
    };
    cppmicroservices::FrameworkConfiguration frameworkConfiguration {
        { cppmicroservices::Constants::FRAMEWORK_BUNDLE_VALIDATION_FUNC, validationFunc }
    };
    auto framework = cppmicroservices::FrameworkFactory().NewFramework(std::move(frameworkConfiguration));

    ASSERT_NO_THROW(framework.Start());

    test::InstallAndStartDS(framework.GetBundleContext());
    test::InstallLib(framework.GetBundleContext(), "TestBundleDSTBV5");

    auto bundles = framework.GetBundleContext().GetBundles();
    auto bundleIter
        = std::find_if(bundles.begin(),
                       bundles.end(),
                       [](cppmicroservices::Bundle const& b) { return (b.GetSymbolicName() == "TestBundleDSTBV5"); });

    ASSERT_NO_THROW(bundleIter->Start());
    ASSERT_EQ(bundleIter->GetState(), cppmicroservices::Bundle::State::STATE_ACTIVE);
    // Uninstalling Bundle
    ASSERT_NO_THROW(bundleIter->Uninstall());
    ASSERT_EQ(bundleIter->GetState(), cppmicroservices::Bundle::State::STATE_UNINSTALLED);

    test::InstallLib(framework.GetBundleContext(), "TestBundleDSTBV5");

    bundles = framework.GetBundleContext().GetBundles();
    bundleIter
        = std::find_if(bundles.begin(),
                       bundles.end(),
                       [](cppmicroservices::Bundle const& b) { return (b.GetSymbolicName() == "TestBundleDSTBV5"); });

    // Reinstalling Bundle
    ASSERT_NO_THROW(bundleIter->Start());
    ASSERT_EQ(bundleIter->GetState(), cppmicroservices::Bundle::State::STATE_ACTIVE);

    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
}

/*
 * Verfiy bundle validation happens per-framework
 */
TEST(TestBundleValidation, BundleValidationPerFramework)
{
    using validationFuncType = std::function<bool(cppmicroservices::Bundle const&)>;

    validationFuncType validationFuncTrue = [](cppmicroservices::Bundle const& b) -> bool
    {
        return true;
    };
    validationFuncType validationFuncFalse = [](cppmicroservices::Bundle const& b) -> bool
    {
        if (b.GetSymbolicName() == "declarative_services" || b.GetSymbolicName() == "configuration_admin")
        {
            return true;
        }
        return false;
    };

    cppmicroservices::FrameworkConfiguration frameworkConfigurationValidTrue {
        { cppmicroservices::Constants::FRAMEWORK_BUNDLE_VALIDATION_FUNC, validationFuncTrue }
    };
    cppmicroservices::FrameworkConfiguration frameworkConfigurationValidFalse {
        { cppmicroservices::Constants::FRAMEWORK_BUNDLE_VALIDATION_FUNC, validationFuncFalse }
    };
    auto framework1 = cppmicroservices::FrameworkFactory().NewFramework(std::move(frameworkConfigurationValidTrue));
    ASSERT_NO_THROW(framework1.Start());

    test::InstallAndStartDS(framework1.GetBundleContext());
    test::InstallLib(framework1.GetBundleContext(), "TestBundleDSTBV5");

    auto bundles1 = framework1.GetBundleContext().GetBundles();
    auto bundleIter1
        = std::find_if(bundles1.begin(),
                       bundles1.end(),
                       [](cppmicroservices::Bundle const& b) { return (b.GetSymbolicName() == "TestBundleDSTBV5"); });

    ASSERT_NO_THROW(bundleIter1->Start());
    ASSERT_EQ(bundleIter1->GetState(), cppmicroservices::Bundle::State::STATE_ACTIVE);

    // new framework with a validation func that should fail for all bundles other than DS and CA
    auto framework2 = cppmicroservices::FrameworkFactory().NewFramework(std::move(frameworkConfigurationValidFalse));
    ASSERT_NO_THROW(framework2.Start());

    // install the bundles
    test::InstallAndStartDS(framework2.GetBundleContext());
    test::InstallLib(framework2.GetBundleContext(), "TestBundleDSTBV5");

    // find the bundle in question (already installed in framework1 succesfully)
    auto bundles2 = framework2.GetBundleContext().GetBundles();
    auto bundleIter2
        = std::find_if(bundles2.begin(),
                       bundles2.end(),
                       [](cppmicroservices::Bundle const& b) { return (b.GetSymbolicName() == "TestBundleDSTBV5"); });

    // starting it should throw even though it was valid in framework1
    ASSERT_THROW(bundleIter2->Start(), cppmicroservices::SecurityException);
    ASSERT_EQ(bundleIter2->GetState(), cppmicroservices::Bundle::State::STATE_RESOLVED);

    framework1.Stop();
    framework1.WaitForStop(std::chrono::milliseconds::zero());
    framework2.Stop();
    framework2.WaitForStop(std::chrono::milliseconds::zero());
}
