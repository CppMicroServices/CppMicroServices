/*=============================================================================

Library: CppMicroServices

Copyright (c) The CppMicroServices developers. See the COPYRIGHT
file at the top-level directory of this distribution and at
https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

#include <usConstants.h>
#include <usFrameworkFactory.h>
#include <usFramework.h>
#include <usFrameworkEvent.h>
#include <usLog.h>

#include "usTestUtils.h"
#include "usTestUtilBundleListener.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

using namespace us;

namespace
{
    void TestDefaultConfig()
    {
        FrameworkFactory factory;

        auto f = factory.NewFramework();
        US_TEST_CONDITION(f, "Test Framework instantiation")

        f.Start();

        // Default framework properties:
        //  - threading model: single
        //  - storage location: The current working directory
        //  - log level: 3 (us::ErrorMsg)
#ifdef US_ENABLE_THREADING_SUPPORT
        US_TEST_CONDITION(f.GetProperty(Constants::FRAMEWORK_THREADING_SUPPORT).ToString() == "multi", "Test for default threading option")
#else
        US_TEST_CONDITION(f.GetProperty(Constants::FRAMEWORK_THREADING_SUPPORT).ToString() == "single", "Test for default threading option")
#endif
        US_TEST_CONDITION(f.GetProperty(Constants::FRAMEWORK_STORAGE).Empty(), "Test for empty default base storage property")
        US_TEST_CONDITION(f.GetProperty(Constants::FRAMEWORK_LOG_LEVEL).ToString() == "3", "Test for default logging level")

        US_TEST_CONDITION(Logger::instance().GetLogLevel() == ErrorMsg, "Test default log level")
    }

    void TestCustomConfig()
    {
        std::map < std::string, Any > configuration;
        configuration["org.osgi.framework.security"] = std::string("osgi");
        configuration["org.osgi.framework.startlevel.beginning"] = 0;
        configuration["org.osgi.framework.bsnversion"] = std::string("single");
        configuration["org.osgi.framework.custom1"] = std::string("foo");
        configuration["org.osgi.framework.custom2"] = std::string("bar");
        configuration[Constants::FRAMEWORK_LOG_LEVEL] = 0;

        // the threading model framework property is set at compile time and read-only at runtime. Test that this
        // is always the case.
#ifdef US_ENABLE_THREADING_SUPPORT
        configuration[Constants::FRAMEWORK_THREADING_SUPPORT] = std::string("single");
#else
        configuration[Constants::FRAMEWORK_THREADING_SUPPORT] = std::string("multi");
#endif

        FrameworkFactory factory;

        auto f = factory.NewFramework(configuration);
        US_TEST_CONDITION(f, "Test Framework instantiation with custom configuration")

        f.Start();
        US_TEST_CONDITION("osgi" == f.GetProperty("org.osgi.framework.security").ToString(), "Test Framework custom launch properties")
        US_TEST_CONDITION(0 == any_cast<int>(f.GetProperty("org.osgi.framework.startlevel.beginning")), "Test Framework custom launch properties")
        US_TEST_CONDITION("single" == any_cast<std::string>(f.GetProperty("org.osgi.framework.bsnversion")), "Test Framework custom launch properties")
        US_TEST_CONDITION("foo" == any_cast<std::string>(f.GetProperty("org.osgi.framework.custom1")), "Test Framework custom launch properties")
        US_TEST_CONDITION("bar" == any_cast<std::string>(f.GetProperty("org.osgi.framework.custom2")), "Test Framework custom launch properties")
        US_TEST_CONDITION(any_cast<int>(f.GetProperty(Constants::FRAMEWORK_LOG_LEVEL)) == 0, "Test for custom logging level")

        US_TEST_CONDITION(Logger::instance().GetLogLevel() == DebugMsg, "Test custom log level")

#ifdef US_ENABLE_THREADING_SUPPORT
        US_TEST_CONDITION(f.GetProperty(Constants::FRAMEWORK_THREADING_SUPPORT).ToString() == "multi", "Test for attempt to change threading option")
#else
        US_TEST_CONDITION(f.GetProperty(Constants::FRAMEWORK_THREADING_SUPPORT).ToString() == "single", "Test for attempt to change threading option")
#endif
    }

    void TestProperties()
    {
        FrameworkFactory factory;

        auto f = factory.NewFramework();
        f.Start();
        US_TEST_CONDITION(f.GetLocation() == "System Bundle", "Test Framework Bundle Location");
        US_TEST_CONDITION(f.GetSymbolicName() == Constants::SYSTEM_BUNDLE_SYMBOLICNAME, "Test Framework Bundle Name");
        US_TEST_CONDITION(f.GetBundleId() == 0, "Test Framework Bundle Id");
    }

    void TestLifeCycle()
    {
        TestBundleListener listener;
        FrameworkFactory factory;
        std::vector<BundleEvent> pEvts;

        std::map < std::string, Any > configuration;
        //configuration[Constants::FRAMEWORK_LOG_LEVEL] = 0;
        auto f = factory.NewFramework(configuration);

        US_TEST_CONDITION(f.GetState() == Bundle::STATE_INSTALLED, "Check framework is installed")

        f.Init();

        US_TEST_CONDITION(f.GetState() == Bundle::STATE_STARTING, "Check framework is starting")

        f.Start();

        US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Check framework bundle event listener")

        US_TEST_CONDITION(f.GetState() == Bundle::STATE_ACTIVE, "Check framework is active")

        f.GetBundleContext().AddBundleListener(&listener, &TestBundleListener::BundleChanged);

        f.Stop();
        f.WaitForStop(std::chrono::milliseconds(0));
        US_TEST_CONDITION(!(f.GetState() & Bundle::STATE_ACTIVE), "Check framework is in the Stop state")

        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, f));

        US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Check framework bundle event listener")

        // Test that uninstalling a framework throws an exception
        try
        {
            f.Uninstall();
            US_TEST_FAILED_MSG(<< "Failed to throw uninstall exception")
        }
        catch (const std::runtime_error&)
        {

        }
        catch (...)
        {
            US_TEST_FAILED_MSG(<< "Failed to throw correct exception type")
        }

        // Test that all bundles in the Start state are stopped when the framework is stopped.
        f.Start();
        auto bundle = testing::InstallLib(f.GetBundleContext(), "TestBundleA");
        US_TEST_CONDITION_REQUIRED(bundle, "Non-null bundle")
        bundle.Start();

        // Stopping the framework stops all active bundles.
        f.Stop();
        auto ev = f.WaitForStop(std::chrono::seconds(30));
        if (!ev.IsNull() && ev.GetType() == FrameworkEvent::FRAMEWORK_ERROR) std::rethrow_exception(ev.GetException());
        US_TEST_CONDITION(ev.IsNull() || ev.GetType() == FrameworkEvent::FRAMEWORK_STOPPED, "Check framework stopped");

        US_TEST_CONDITION(bundle.GetState() != Bundle::STATE_ACTIVE, "Check that TestBundleA is in the Stop state")
        US_TEST_CONDITION(f.GetState() != Bundle::STATE_ACTIVE, "Check framework is in the Stop state")
    }

    void TestEvents()
    {
        TestBundleListener listener;
        std::vector<BundleEvent> pEvts;
        std::vector<BundleEvent> pStopEvts;
        FrameworkFactory factory;

        std::map < std::string, Any > configuration;
        //configuration[Constants::FRAMEWORK_LOG_LEVEL] = 0;
        auto f = factory.NewFramework(configuration);

        f.Start();

        auto fmc = f.GetBundleContext();
        fmc.AddBundleListener(&listener, &TestBundleListener::BundleChanged);

        auto install = [&pEvts, &fmc](const std::string& libName)
        {
          auto bundle = testing::InstallLib(fmc, libName);
          pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundle));
#ifdef US_BUILD_SHARED_LIBS
          if (bundle.GetSymbolicName() == "TestBundleB")
          {
            // This is an additional install event from the bundle
            // that is statically imported by TestBundleB.
            pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, testing::GetBundle("TestBundleImportedByB", fmc)));
          }
#endif
        };

        // The bundles used to test bundle events when stopping the framework.
        // For static builds, the order of the "install" calls is imported.
        install("TestBundleA");
        install("TestBundleA2");
        install("TestBundleB");
#ifdef US_ENABLE_THREADING_SUPPORT
        install("TestBundleC1");
#endif
        install("TestBundleH");
#ifndef US_BUILD_SHARED_LIBS
        install("TestBundleImportedByB");
#endif
        install("TestBundleLQ");
        install("TestBundleM");
        install("TestBundleR");
        install("TestBundleRA");
        install("TestBundleRL");
        install("TestBundleS");
        install("TestBundleSL1");
        install("TestBundleSL3");
        install("TestBundleSL4");
#ifndef US_BUILD_SHARED_LIBS
        install("main");
#endif


        auto bundles(fmc.GetBundles());
        for (auto& bundle : bundles)
        {
          bundle.Start();
          // no events will be fired for the framework, its already active at this point
          if (bundle != f)
          {
            pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundle));
            pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundle));
            pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundle));

            // bundles will be stopped in the reverse order in which they were started.
            // It is easier to maintain this test if the stop events are setup in the
            // right order here, while the bundles are starting, instead of hard coding
            // the order of events somewhere else.
            // Doing it this way also tests the order in which starting and stopping
            // bundles occurs and when their events are fired.
            pStopEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundle));
            pStopEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundle));
          }
        }
        // Remember, the framework is stopped first, before all bundles are stopped.
        pStopEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, f));
        std::reverse(pStopEvts.begin(), pStopEvts.end());


        US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Check for bundle start events")

        // Stopping the framework stops all active bundles.
        f.Stop();
        f.WaitForStop(std::chrono::milliseconds(0));

        US_TEST_CONDITION(listener.CheckListenerEvents(pStopEvts), "Check for bundle stop events")
    }
}

int usFrameworkTest(int /*argc*/, char* /*argv*/[])
{
    US_TEST_BEGIN("FrameworkTest");

    TestDefaultConfig();
    TestCustomConfig();
    TestProperties();
    TestLifeCycle();
    TestEvents();

    US_TEST_END()
}
