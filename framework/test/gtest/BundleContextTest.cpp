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

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/LDAPFilter.h"
#include "cppmicroservices/LDAPProp.h"
#include "cppmicroservices/ServiceFactory.h"
#include "cppmicroservices/ServiceObjects.h"
#include "cppmicroservices/ServiceReference.h"

#include "TestUtils.h"
#include "TestingConfig.h"

#include <chrono>
#include <future>
#include <thread>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace cppmicroservices;
using namespace cppmicroservices::testing;

namespace bc_tests
{
    struct TestService
    {
        virtual ~TestService() {}
    };
} // namespace bc_tests

namespace cppmicroservices
{
    struct TestBundleH
    {
        virtual ~TestBundleH() {}
    };
} // namespace cppmicroservices

TEST(BundleContextTest, BundleContextThrowWhenInvalid)
{
    cppmicroservices::Framework framework = cppmicroservices::FrameworkFactory().NewFramework();
    ASSERT_TRUE(framework) << "The framework was not created successfully.";
    framework.Start();

    auto context = framework.GetBundleContext();
    ASSERT_TRUE(context) << "The bundle context is not valid.";

    // Register a service and get a service reference for testing
    // GetService() later.
    (void)context.RegisterService<bc_tests::TestService>(std::make_shared<bc_tests::TestService>());
    auto sRef = context.GetServiceReference<bc_tests::TestService>();
    ASSERT_TRUE(sRef) << "The service reference is not valid.";

    auto frameworkListenerToken = context.AddFrameworkListener([](cppmicroservices::FrameworkEvent const&) {});

    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());

    auto context2 = framework.GetBundleContext();
    ASSERT_FALSE(context2) << "The bundle context is valid when it should not be.";

    EXPECT_THROW({ (void)context2.GetProperty("bonjour"); }, std::runtime_error)
        << "GetProperty() on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.GetProperties(); }, std::runtime_error)
        << "GetProperties() on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.GetBundle(); }, std::runtime_error)
        << "GetBundle() on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.GetBundle(0); }, std::runtime_error)
        << "GetBundle(int) on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.GetBundles("fake/path"); }, std::runtime_error)
        << "GetBundles(string) on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.GetBundles(); }, std::runtime_error)
        << "GetBundles() on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.RegisterService<bc_tests::TestService>(std::make_shared<bc_tests::TestService>()); },
                 std::runtime_error)
        << "RegisterService() on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.GetServiceReferences<bc_tests::TestService>(); }, std::runtime_error)
        << "GetServiceReferences(string) on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.GetServiceReferences<bc_tests::TestService>(); }, std::runtime_error)
        << "GetServiceReferences() on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.GetServiceReference("hola"); }, std::runtime_error)
        << "GetServiceReference(string) on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.GetServiceReference<bc_tests::TestService>(); }, std::runtime_error)
        << "GetServiceReference() on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.GetService<bc_tests::TestService>(sRef); }, std::invalid_argument)
        << "GetService() on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.GetServiceObjects<bc_tests::TestService>(sRef); }, std::runtime_error)
        << "GetServiceObjects() on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.AddServiceListener([](const cppmicroservices::ServiceEvent&) {}); },
                 std::runtime_error)
        << "AddServiceListener() on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.AddBundleListener([](const cppmicroservices::BundleEvent&) {}); }, std::runtime_error)
        << "AddBundleListener() on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.AddFrameworkListener([](const cppmicroservices::FrameworkEvent&) {}); },
                 std::runtime_error)
        << "AddFrameworkListener() on invalid BundleContext did not throw.";
    EXPECT_THROW({ context2.RemoveListener(std::move(frameworkListenerToken)); }, std::runtime_error)
        << "RemoveListener() on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.GetDataFile("definitely/not/a/real/file/name.txt"); }, std::runtime_error)
        << "GetDataFile() on invalid BundleContext did not throw.";
    EXPECT_THROW({ (void)context2.InstallBundles("this/doesnt/matter"); }, std::runtime_error)
        << "InstallBundles() on invalid BundleContext did not throw.";
}

#if defined(US_ENABLE_THREADING_SUPPORT)
TEST(BundleContextTest, NoSegfaultWithRegisterServiceShutdownRace)
{
    cppmicroservices::Framework framework = cppmicroservices::FrameworkFactory().NewFramework();
    ASSERT_TRUE(framework) << "The framework was not created successfully.";
    framework.Start();

    auto context = framework.GetBundleContext();
    ASSERT_TRUE(context) << "The bundle context is not valid.";

    std::thread thread(
        [&framework]()
        {
            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        });

    try
    {
        (void)context.RegisterService<bc_tests::TestService>(std::make_shared<bc_tests::TestService>());
    }
    catch (...)
    {
        // The above can throw, and should, in the case where it would previously segfault
        // due to the check-then-act race. In the event it throws, we will ignore it as this
        // is desired.
    }

    thread.join();
}

TEST(BundleContextTest, NoSegfaultWithServiceFactory)
{
    cppmicroservices::Framework framework = FrameworkFactory().NewFramework();
    framework.Start();
    auto context = framework.GetBundleContext();

    InstallLib(context, "TestBundleH").Start();

    std::string getServiceThrowsFilter(LDAPProp("getservice_exception") == true);
    auto svcGetServiceThrowsRefs(context.GetServiceReferences("cppmicroservices::TestBundleH", getServiceThrowsFilter));

    ASSERT_EQ(svcGetServiceThrowsRefs.size(), 1);
    ASSERT_EQ("1", svcGetServiceThrowsRefs[0].GetProperty(std::string("getservice_exception")).ToString());

    std::thread thread(
        [&framework]()
        {
            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        });

    ASSERT_EQ(nullptr, context.GetService(svcGetServiceThrowsRefs[0]));

    thread.join();
}
#endif
