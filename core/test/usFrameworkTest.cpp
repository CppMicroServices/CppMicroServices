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

#include <usFrameworkFactory.h>
#include <usFramework.h>
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

        f->Start();

        // Default framework properties:
        //  - threading model: single
        //  - storage location: The current working directory
        //  - log level: 3 (us::ErrorMsg)
#ifdef US_ENABLE_THREADING_SUPPORT
        US_TEST_CONDITION(f->GetProperty(Framework::PROP_THREADING_SUPPORT).ToString() == "multi", "Test for default threading option")
#else
        US_TEST_CONDITION(f->GetProperty(Framework::PROP_THREADING_SUPPORT).ToString() == "single", "Test for default threading option")
#endif
        US_TEST_CONDITION(f->GetProperty(Framework::PROP_STORAGE_LOCATION).ToString() == testing::GetCurrentWorkingDirectory(), "Test for default base storage path")
        US_TEST_CONDITION(f->GetProperty(Framework::PROP_LOG_LEVEL).ToString() == "3", "Test for default logging level")

        US_TEST_CONDITION(Logger::instance().GetLogLevel() == ErrorMsg, "Test default log level")

#if defined (US_BUILD_SHARED_LIBS)
        US_TEST_CONDITION(f->GetBundleContext()->GetBundles().size() == 1, "Unexpected number of bundles")
#endif
        f->Stop();
    }

    void TestCustomConfig()
    {
        std::map < std::string, Any > configuration;
        configuration["org.osgi.framework.security"] = std::string("osgi");
        configuration["org.osgi.framework.startlevel.beginning"] = 0;
        configuration["org.osgi.framework.bsnversion"] = std::string("single");
        configuration["org.osgi.framework.custom1"] = std::string("foo");
        configuration["org.osgi.framework.custom2"] = std::string("bar");
        configuration[Framework::PROP_STORAGE_LOCATION] = std::string("/foo");
        configuration[Framework::PROP_LOG_LEVEL] = 0;

        // the threading model framework property is set at compile time and read-only at runtime. Test that this
        // is always the case.
#ifdef US_ENABLE_THREADING_SUPPORT
        configuration[Framework::PROP_THREADING_SUPPORT] = std::string("single");
#else
        configuration[Framework::PROP_THREADING_SUPPORT] = std::string("multi");
#endif
        std::string installPaths;
#if defined (US_BUILD_SHARED_LIBS)
        installPaths = LIB_PATH + ";" + LIB_PATH + "/dummydir";
#endif
        configuration.insert(std::pair<std::string, std::string>(Framework::PROP_INSTALL_PATHS, installPaths));

        FrameworkFactory factory;

        auto f = factory.NewFramework(configuration);
        US_TEST_CONDITION(f, "Test Framework instantiation with custom configuration")

        f->Start();
        US_TEST_CONDITION("osgi" == f->GetProperty("org.osgi.framework.security").ToString(), "Test Framework custom launch properties")
        US_TEST_CONDITION(0 == any_cast<int>(f->GetProperty("org.osgi.framework.startlevel.beginning")), "Test Framework custom launch properties")
        US_TEST_CONDITION("single" == any_cast<std::string>(f->GetProperty("org.osgi.framework.bsnversion")), "Test Framework custom launch properties")
        US_TEST_CONDITION("foo" == any_cast<std::string>(f->GetProperty("org.osgi.framework.custom1")), "Test Framework custom launch properties")
        US_TEST_CONDITION("bar" == any_cast<std::string>(f->GetProperty("org.osgi.framework.custom2")), "Test Framework custom launch properties")
        US_TEST_CONDITION(f->GetProperty(Framework::PROP_STORAGE_LOCATION).ToString() == "/foo", "Test for custom base storage path")
        US_TEST_CONDITION(any_cast<int>(f->GetProperty(Framework::PROP_LOG_LEVEL)) == 0, "Test for custom logging level")

        US_TEST_CONDITION(Logger::instance().GetLogLevel() == DebugMsg, "Test custom log level")

        US_TEST_CONDITION(f->GetProperty(Framework::PROP_INSTALL_PATHS).ToString() == installPaths, "Test auto-install config setting")
#ifdef US_ENABLE_THREADING_SUPPORT
        US_TEST_CONDITION(f->GetProperty(Framework::PROP_THREADING_SUPPORT).ToString() == "multi", "Test for attempt to change threading option")
#else
        US_TEST_CONDITION(f->GetProperty(Framework::PROP_THREADING_SUPPORT).ToString() == "single", "Test for attempt to change threading option")
#endif
#if defined (US_BUILD_SHARED_LIBS)
        US_TEST_CONDITION(f->GetBundleContext()->GetBundles().size() > 1, "Unexpected number of bundles")
#endif
        f->Stop();
    }

    void TestProperties()
    {
        FrameworkFactory factory;

        auto f = factory.NewFramework();
        f->Start();
        US_TEST_CONDITION(f->GetLocation() == "System Bundle", "Test Framework Bundle Location");
        US_TEST_CONDITION(f->GetName() == US_CORE_FRAMEWORK_NAME, "Test Framework Bundle Name");
        US_TEST_CONDITION(f->GetBundleId() == 0, "Test Framework Bundle Id");
        f->Stop();
    }

    void TestLifeCycle()
    {
        TestBundleListener listener;
        FrameworkFactory factory;
        std::vector<BundleEvent> pEvts;

        auto f = factory.NewFramework();
        f->Start();

        US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Check framework bundle event listener")

        US_TEST_CONDITION(f->IsStarted(), "Check framework is in the Start state")

        f->GetBundleContext()->AddBundleListener(&listener, &TestBundleListener::BundleChanged);

        f->Stop();
        US_TEST_CONDITION(!f->IsStarted(), "Check framework is in the Stop state")

        pEvts.push_back(BundleEvent(BundleEvent::STOPPING, f));

        US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Check framework bundle event listener")

        // Test that uninstalling a framework throws an exception
        try
        {
            f->Uninstall();
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
        f->Start();
        InstallTestBundle(f->GetBundleContext(), "TestBundleA");

        auto bundleA = f->GetBundleContext()->GetBundle("TestBundleA");
        bundleA->Start();

        // Stopping the framework stops all active bundles.
        f->Stop();

        US_TEST_CONDITION(!bundleA->IsStarted(), "Check that TestBundleA is in the Stop state")
        US_TEST_CONDITION(!f->IsStarted(), "Check framework is in the Stop state")
    }

    void TestEvents()
    {
        TestBundleListener listener;
        std::vector<BundleEvent> pEvts;
        std::vector<BundleEvent> pStopEvts;
        FrameworkFactory factory;

        auto f = factory.NewFramework();

        f->Start();

        BundleContext* fmc = f->GetBundleContext();
        fmc->AddBundleListener(&listener, &TestBundleListener::BundleChanged);

        // The bundles used to test bundle events when stopping the framework
        auto bundle = InstallTestBundle(fmc, "TestBundleA");
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
        bundle = InstallTestBundle(fmc, "TestBundleA2");
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
        bundle = InstallTestBundle(fmc, "TestBundleB");
#if defined (US_BUILD_SHARED_LIBS)
        // static-linked bundles are auto-installed
        auto bundleImportedByB = fmc->GetBundle("TestBundleImportedByB");
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundleImportedByB));
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
#else
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
        auto bundleImportedByB = InstallTestBundle(fmc, "TestBundleImportedByB");
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundleImportedByB));
#endif
        bundle = InstallTestBundle(fmc, "TestBundleH");
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
        bundle = InstallTestBundle(fmc, "TestBundleM");
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
        bundle = InstallTestBundle(fmc, "TestBundleR");
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
        bundle = InstallTestBundle(fmc, "TestBundleRA");
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
#ifndef US_DISABLE_TESTING_LINKED_RESOURCES // FIXME: TestBundleRL uses linker to embed manifest.json on Windows
        bundle = InstallTestBundle(fmc, "TestBundleRL");
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
#endif
        bundle = InstallTestBundle(fmc, "TestBundleS");
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
        bundle = InstallTestBundle(fmc, "TestBundleSL1");
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
        bundle = InstallTestBundle(fmc, "TestBundleSL3");
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
        bundle = InstallTestBundle(fmc, "TestBundleSL4");
        pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));

        auto bundles(fmc->GetBundles());
        for (auto& bundle : bundles)
        {
            bundle->Start();
            // no events will be fired for the framework, its already active at this point
            if (bundle != f)
            {
                pEvts.push_back(BundleEvent(BundleEvent::STARTING, bundle));
                pEvts.push_back(BundleEvent(BundleEvent::STARTED, bundle));

                // bundles will be stopped in the same order in which they were started.
                // It is easier to maintain this test if the stop events are setup in the
                // right order here, while the bundles are starting, instead of hard coding
                // the order of events somewhere else.
                // Doing it this way also tests the order in which starting and stopping
                // bundles occurs and when their events are fired.
                pStopEvts.push_back(BundleEvent(BundleEvent::STOPPING, bundle));
                pStopEvts.push_back(BundleEvent(BundleEvent::STOPPED, bundle));
            }
        }

        US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Check for bundle start events")

        // Remember, the framework is stopped last, after all bundles are stopped.
        pStopEvts.push_back(BundleEvent(BundleEvent::STOPPING, f));

        // Stopping the framework stops all active bundles.
        f->Stop();

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
