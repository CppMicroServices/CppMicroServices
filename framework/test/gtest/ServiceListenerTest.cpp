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
#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/ServiceEvent.h"
#include "cppmicroservices/SharedLibrary.h"

#include "BundlePropsInterface.h"
#include "TestUtils.h"
#include "TestingConfig.h"

#include "gtest/gtest.h"

US_MSVC_PUSH_DISABLE_WARNING(4996)

using namespace cppmicroservices;
using namespace cppmicroservices::testing;

class ServiceListenerTest : public ::testing::Test
{
  protected:
    Framework framework;

  public:
    ServiceListenerTest() : framework(FrameworkFactory().NewFramework()) {};

    ~ServiceListenerTest() override = default;

    void
    SetUp() override
    {
        framework.Start();
    }

    void
    TearDown() override
    {
        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }
};

class TestServiceListener
{

  private:
    friend bool runStartStopTest(std::string const&,
                                 int cnt,
                                 Bundle const&,
                                 BundleContext const& context,
                                 std::vector<ServiceEvent::Type> const&);

    bool const checkUsingBundles;
    std::vector<ServiceEvent> events;

    bool teststatus;

    BundleContext context;

  public:
    TestServiceListener(BundleContext const& context, bool checkUsingBundles = true)
        : checkUsingBundles(checkUsingBundles)
        , events()
        , teststatus(true)
        , context(context)
    {
    }

    bool
    getTestStatus() const
    {
        return teststatus;
    }

    void
    clearEvents()
    {
        events.clear();
    }

    bool
    checkEvents(std::vector<ServiceEvent::Type> const& eventTypes)
    {
        if (events.size() != eventTypes.size())
        {
            return false;
        }

        for (std::size_t i = 0; i < eventTypes.size(); ++i)
        {
            if (eventTypes[i] != events[i].GetType())
            {
                return false;
            }
        }
        return true;
    }

    void
    serviceChanged(ServiceEvent const& evt)
    {
        events.push_back(evt);
        if (ServiceEvent::SERVICE_UNREGISTERING == evt.GetType())
        {
            ServiceReferenceU sr = evt.GetServiceReference();

            // Validate that no bundle is marked as using the service
            std::vector<Bundle> usingBundles = sr.GetUsingBundles();
            if (checkUsingBundles && !usingBundles.empty())
            {
                teststatus = false;
            }

            // Check if the service can be fetched
            InterfaceMapConstPtr service = context.GetService(sr);
            usingBundles = sr.GetUsingBundles();
            // if (UNREGISTERSERVICE_VALID_DURING_UNREGISTERING) {
            // In this mode the service shall be obtainable during
            // unregistration.
            if (!service || service->empty())
            {
                teststatus = false;
            }

            if (checkUsingBundles && usingBundles.size() != 1)
            {
                teststatus = false;
            }
            service.reset(); // results in UngetService

            // Check that the SERVICE_UNREGISTERING service can not be looked up
            // using the service registry.
            try
            {
                long sid = any_cast<long>(sr.GetProperty(Constants::SERVICE_ID));
                std::stringstream ss;
                ss << "(" << Constants::SERVICE_ID << "=" << sid << ")";
                std::vector<ServiceReferenceU> srs = context.GetServiceReferences("", ss.str());
                if (srs.empty()) {}
                else
                {
                    teststatus = false;
                }
            }
            catch (std::exception const&)
            {
                teststatus = false;
            }
        }
    }

}; // end of class ServiceListener

bool
runStartStopTest(std::string const& /*name*/,
                 int cnt,
                 Bundle& bundle,
                 BundleContext context,
                 std::vector<ServiceEvent::Type> const& events)
{
    bool teststatus = true;

    for (int i = 0; i < cnt && teststatus; ++i)
    {
        TestServiceListener sListen(context);
        try
        {
            context.AddServiceListener(&sListen, &TestServiceListener::serviceChanged);
        }
        catch (std::logic_error const&)
        {
            teststatus = false;
        }

        // Start the test target to get a service published.
        try
        {
            bundle.Start();
        }
        catch (std::exception const&)
        {
            teststatus = false;
        }

        // Stop the test target to get a service unpublished.
        try
        {
            bundle.Stop();
        }
        catch (std::exception const&)
        {
            teststatus = false;
        }

        if (teststatus && !sListen.checkEvents(events))
        {
            teststatus = false;
        }

        try
        {
            context.RemoveServiceListener(&sListen, &TestServiceListener::serviceChanged);
            teststatus &= sListen.getTestStatus();
            sListen.clearEvents();
        }
        catch (std::logic_error const&)
        {
            teststatus = false;
        }
    }
    return teststatus;
}

TEST_F(ServiceListenerTest, frameSL02a)
{
    auto context = framework.GetBundleContext();

    TestServiceListener listener1(context);
    TestServiceListener listener2(context);

    context.RemoveServiceListener(&listener1, &TestServiceListener::serviceChanged);
    context.AddServiceListener(&listener1, &TestServiceListener::serviceChanged);
    context.RemoveServiceListener(&listener2, &TestServiceListener::serviceChanged);
    context.AddServiceListener(&listener2, &TestServiceListener::serviceChanged);

    auto bundle = InstallLib(context, "TestBundleA");
    bundle.Start();

    std::vector<ServiceEvent::Type> events;
    events.push_back(ServiceEvent::SERVICE_REGISTERED);

    // Check first service listener
    ASSERT_TRUE(listener1.checkEvents(events));
    // Check second service listener
    ASSERT_TRUE(listener2.checkEvents(events));

    context.RemoveServiceListener(&listener1, &TestServiceListener::serviceChanged);
    context.RemoveServiceListener(&listener2, &TestServiceListener::serviceChanged);
}

TEST_F(ServiceListenerTest, frameSL05a)
{
    std::vector<ServiceEvent::Type> events;
    events.push_back(ServiceEvent::SERVICE_REGISTERED);
    events.push_back(ServiceEvent::SERVICE_UNREGISTERING);

    auto bundle = InstallLib(framework.GetBundleContext(), "TestBundleA");

    bool testStatus = runStartStopTest("FrameSL05a", 1, bundle, framework.GetBundleContext(), events);

    ASSERT_TRUE(testStatus);
}

TEST_F(ServiceListenerTest, frameSL10a)
{
    std::vector<ServiceEvent::Type> events;
    events.push_back(ServiceEvent::SERVICE_REGISTERED);
    events.push_back(ServiceEvent::SERVICE_UNREGISTERING);

    auto bundle = InstallLib(framework.GetBundleContext(), "TestBundleA2");

    bool testStatus = runStartStopTest("FrameSL10a", 1, bundle, framework.GetBundleContext(), events);
    ASSERT_TRUE(testStatus);
}

TEST_F(ServiceListenerTest, frameSL25a)
{
    auto context = framework.GetBundleContext();

    TestServiceListener sListen(context, false);
    EXPECT_NO_THROW(context.AddServiceListener(&sListen, &TestServiceListener::serviceChanged))
        << "service listener registration failed ";

    auto libSL1 = InstallLib(context, "TestBundleSL1");
    auto libSL3 = InstallLib(context, "TestBundleSL3");
    auto libSL4 = InstallLib(context, "TestBundleSL4");

    std::vector<ServiceEvent::Type> expectedServiceEventTypes;

    // Startup
    expectedServiceEventTypes.push_back(ServiceEvent::SERVICE_REGISTERED); // at start of libSL1
    expectedServiceEventTypes.push_back(ServiceEvent::SERVICE_REGISTERED); // FooService at start of libSL4
    expectedServiceEventTypes.push_back(ServiceEvent::SERVICE_REGISTERED); // at start of libSL3

    // Stop libSL4
    expectedServiceEventTypes.push_back(ServiceEvent::SERVICE_UNREGISTERING); // FooService at first stop of libSL4

#ifdef US_BUILD_SHARED_LIBS
    // Shutdown
    expectedServiceEventTypes.push_back(ServiceEvent::SERVICE_UNREGISTERING); // at stop of libSL1
    expectedServiceEventTypes.push_back(ServiceEvent::SERVICE_UNREGISTERING); // at stop of libSL3
#endif

    // Start libBundleTestSL1 to ensure that the Service interface is available.
    libSL1.Start();

    // Start libBundleTestSL4 that will require the serivce interface and publish
    // cppmicroservices::FooService
    libSL4.Start();

    // Start libBundleTestSL3 that will require the serivce interface and get the service
    libSL3.Start();

    // Check that libSL3 has been notified about the FooService.
    ServiceReferenceU libSL3SR = context.GetServiceReference("ActivatorSL3");
    InterfaceMapConstPtr libSL3Activator = context.GetService(libSL3SR);
    // Validate that ActivatorSL3 service != 0
    ASSERT_TRUE(libSL3Activator && !libSL3Activator->empty());

    ServiceReference<BundlePropsInterface> libSL3PropsI(libSL3SR);
    auto propsInterface = context.GetService(libSL3PropsI);
    ASSERT_TRUE(propsInterface);

    BundlePropsInterface::Properties::const_iterator i = propsInterface->GetProperties().find("serviceAdded");
    ASSERT_NE(i, propsInterface->GetProperties().end());
    Any serviceAddedField3 = i->second;
    // Test libSL3 notified about presence of FooService
    ASSERT_FALSE(serviceAddedField3.Empty());
    ASSERT_TRUE(any_cast<bool>(serviceAddedField3));

    // Check that libSL1 has been notified about the FooService.

    ServiceReferenceU libSL1SR = context.GetServiceReference("ActivatorSL1");
    auto libSL1Activator = context.GetService(libSL1SR);
    ASSERT_TRUE(libSL1Activator);
    ASSERT_FALSE(libSL1Activator->empty());

    ServiceReference<BundlePropsInterface> libSL1PropsI(libSL1SR);
    propsInterface = context.GetService(libSL1PropsI);
    // Cast to BundlePropsInterface
    ASSERT_TRUE(propsInterface);

    i = propsInterface->GetProperties().find("serviceAdded");
    ASSERT_NE(i, propsInterface->GetProperties().end());
    Any serviceAddedField1 = i->second;
    // Test libSL1 notified about presence of FooService
    ASSERT_FALSE(serviceAddedField1.Empty());
    ASSERT_TRUE(any_cast<bool>(serviceAddedField1));

    // Stop the service provider: libSL4
    libSL4.Stop();

    // Check that libSL3 has been notified about the removal of FooService.
    libSL3SR = context.GetServiceReference("ActivatorSL3");
    libSL3Activator = context.GetService(libSL3SR);
    ASSERT_TRUE(libSL3Activator);
    ASSERT_FALSE(libSL3Activator->empty());

    propsInterface = context.GetService(libSL3PropsI);
    ASSERT_TRUE(propsInterface);

    i = propsInterface->GetProperties().find("serviceRemoved");
    ASSERT_NE(i, propsInterface->GetProperties().end());

    Any serviceRemovedField3 = i->second;
    ASSERT_FALSE(serviceRemovedField3.Empty());
    // test libSL3 notified about removal of FooService
    ASSERT_TRUE(any_cast<bool>(serviceRemovedField3));

    // Stop libSL1
    libSL1.Stop();

    // Stop pSL3
    libSL3.Stop();

    // Check service events seen by this class
    ASSERT_TRUE(sListen.checkEvents(expectedServiceEventTypes)) << "Service listener event notification error";

    // Service listener checks
    ASSERT_TRUE(sListen.getTestStatus());

    context.RemoveServiceListener(&sListen, &TestServiceListener::serviceChanged);
    sListen.clearEvents();
}

US_MSVC_POP_WARNING
