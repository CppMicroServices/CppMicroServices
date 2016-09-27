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

#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#ifdef US_BUILD_SHARED_LIBS
#include "singletonframework/singletonframework.h"
#endif

#include "TestingConfig.h"
#include "TestingMacros.h"
#include "TestUtilBundleListener.h"
#include "TestUtils.h"

#include <exception>
#include <mutex>
#include <thread>

using namespace cppmicroservices;

namespace
{
    void TestDefaultConfig()
    {
        auto f = FrameworkFactory().NewFramework();
		US_TEST_CONDITION(f, "Test Framework instantiation");

        f.Start();

        // Default framework properties:
        //  - threading model: multi
        //  - persistent storage location: The current working directory
        //  - diagnostic logging: off
        //  - diagnostic logger: std::clog
#ifdef US_ENABLE_THREADING_SUPPORT
        US_TEST_CONDITION(f.GetProperty(Constants::FRAMEWORK_THREADING_SUPPORT).ToString() == "multi", "Test for default threading option")
#else
        US_TEST_CONDITION(f.GetProperty(Constants::FRAMEWORK_THREADING_SUPPORT).ToString() == "single", "Test for default threading option")
#endif
        US_TEST_CONDITION(f.GetProperty(Constants::FRAMEWORK_STORAGE).Empty(), "Test for empty default base storage property")
        US_TEST_CONDITION(any_cast<bool>(f.GetProperty(Constants::FRAMEWORK_LOG)) == false, "Test default diagnostic logging")

    }

    void TestCustomConfig()
    {
        std::map < std::string, Any > configuration;
        configuration["org.osgi.framework.security"] = std::string("osgi");
        configuration["org.osgi.framework.startlevel.beginning"] = 0;
        configuration["org.osgi.framework.bsnversion"] = std::string("single");
        configuration["org.osgi.framework.custom1"] = std::string("foo");
        configuration["org.osgi.framework.custom2"] = std::string("bar");
        configuration[Constants::FRAMEWORK_LOG] = true;
        configuration[Constants::FRAMEWORK_STORAGE] = testing::GetTempDirectory();

        // the threading model framework property is set at compile time and read-only at runtime. Test that this
        // is always the case.
#ifdef US_ENABLE_THREADING_SUPPORT
        configuration[Constants::FRAMEWORK_THREADING_SUPPORT] = std::string("single");
#else
        configuration[Constants::FRAMEWORK_THREADING_SUPPORT] = std::string("multi");
#endif

        auto f = FrameworkFactory().NewFramework(configuration);
		US_TEST_CONDITION(f, "Test Framework instantiation with custom configuration");

        try
        {
          f.Start();
        }
        catch (const std::exception& e)
        {
          US_TEST_FAILED_MSG(<< "Exception during framework start. " << e.what());
        }
        catch (...)
        {
          US_TEST_FAILED_MSG(<< "Unknown exception during framework start. ");
        }

        US_TEST_CONDITION("osgi" == f.GetProperty("org.osgi.framework.security").ToString(), "Test Framework custom launch properties");
        US_TEST_CONDITION(0 == any_cast<int>(f.GetProperty("org.osgi.framework.startlevel.beginning")), "Test Framework custom launch properties");
        US_TEST_CONDITION("single" == any_cast<std::string>(f.GetProperty("org.osgi.framework.bsnversion")), "Test Framework custom launch properties");
        US_TEST_CONDITION("foo" == any_cast<std::string>(f.GetProperty("org.osgi.framework.custom1")), "Test Framework custom launch properties");
        US_TEST_CONDITION("bar" == any_cast<std::string>(f.GetProperty("org.osgi.framework.custom2")), "Test Framework custom launch properties");
        US_TEST_CONDITION(any_cast<bool>(f.GetProperty(Constants::FRAMEWORK_LOG)) == true, "Test for enabled diagnostic logging");
		US_TEST_CONDITION(f.GetProperty(Constants::FRAMEWORK_STORAGE).ToString() == testing::GetTempDirectory(), "Test for custom base storage path");

#ifdef US_ENABLE_THREADING_SUPPORT
        US_TEST_CONDITION(f.GetProperty(Constants::FRAMEWORK_THREADING_SUPPORT).ToString() == "multi", "Test for attempt to change threading option")
#else
        US_TEST_CONDITION(f.GetProperty(Constants::FRAMEWORK_THREADING_SUPPORT).ToString() == "single", "Test for attempt to change threading option")
#endif
    }

    //void TestDefaultLogSink()
    //{
    //    std::map<std::string, Any> configuration;
    //    // turn on diagnostic logging
    //    configuration[Constants::FRAMEWORK_LOG] = true;

    //    // Needs improvement to guarantee that log messages are generated,
    //    // however for the moment do some operations using the framework
    //    // which would create diagnostic log messages.
    //    std::stringstream temp_buf;
    //    auto clog_buf = std::clog.rdbuf();
    //    std::clog.rdbuf(temp_buf.rdbuf());

    //    auto f = FrameworkFactory().NewFramework(configuration);
    //    US_TEST_CONDITION(f, "Test Framework instantiation with custom diagnostic logger");

    //    f.Start();
    //    testing::InstallLib(f.GetBundleContext(), "TestBundleA");

    //    f.Stop();
    //    f.WaitForStop(std::chrono::milliseconds::zero());

    //    US_TEST_OUTPUT(<< "Diagnostic log messages: " << temp_buf.str());
    //    US_TEST_CONDITION(!temp_buf.str().empty(), "Test that the default logger captured data.");

    //    std::clog.rdbuf(clog_buf);
    //}

	void TestCustomLogSink()
	{
		std::map<std::string, Any> configuration;
		// turn on diagnostic logging
		configuration[Constants::FRAMEWORK_LOG] = true;

		std::ostream custom_log_sink(std::cerr.rdbuf());

		auto f = FrameworkFactory().NewFramework(configuration, &custom_log_sink);
		US_TEST_CONDITION(f, "Test Framework instantiation with custom diagnostic logger");

		f.Start();
		f.Stop();
	}

    void TestProperties()
    {
        auto f = FrameworkFactory().NewFramework();
        f.Init();
        US_TEST_CONDITION(f.GetLocation() == "System Bundle", "Test Framework Bundle Location");
        US_TEST_CONDITION(f.GetSymbolicName() == Constants::SYSTEM_BUNDLE_SYMBOLICNAME, "Test Framework Bundle Name");
        US_TEST_CONDITION(f.GetBundleId() == 0, "Test Framework Bundle Id");
    }

//    void TestLifeCycle()
//    {
//        TestBundleListener listener;
//        std::vector<BundleEvent> pEvts;
//
//        auto f = FrameworkFactory().NewFramework();
//
//        US_TEST_CONDITION(f.GetState() == Bundle::STATE_INSTALLED, "Check framework is installed")
//
//        // make sure WaitForStop returns immediately when the Framework's state is "Installed"
//        auto fNoWaitEvent = f.WaitForStop(std::chrono::milliseconds::zero());
//        US_TEST_CONDITION(fNoWaitEvent, "Check for valid framework event");
//        US_TEST_CONDITION(fNoWaitEvent.GetType() == FrameworkEvent::Type::FRAMEWORK_ERROR, "Check for correct framework event type");
//        US_TEST_CONDITION(fNoWaitEvent.GetThrowable() == nullptr, "Check that no exception was thrown");
//
//        f.Init();
//
//        US_TEST_CONDITION(f.GetState() == Bundle::STATE_STARTING, "Check framework is starting");
//        US_TEST_CONDITION(f.GetBundleContext(), "Check for a valid bundle context");
//
//        f.Start();
//
//        US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Check framework bundle event listener");
//
//        US_TEST_CONDITION(f.GetState() == Bundle::STATE_ACTIVE, "Check framework is active");
//
//        f.GetBundleContext().AddBundleListener(&listener, &TestBundleListener::BundleChanged);
//
//#ifdef US_ENABLE_THREADING_SUPPORT
//        // To test that the correct FrameworkEvent is returned, we must guarantee that the Framework
//        // is in a STARTING, ACTIVE, or STOPPING state.
//        std::thread waitForStopThread([&f]() {
//            auto fStopEvent = f.WaitForStop(std::chrono::milliseconds::zero());
//            US_TEST_CONDITION(!(f.GetState() & Bundle::STATE_ACTIVE), "Check framework is in the Stop state")
//            US_TEST_CONDITION(fStopEvent, "Check for valid framework event");
//            US_TEST_CONDITION(fStopEvent.GetType() == FrameworkEvent::Type::FRAMEWORK_STOPPED, "Check for correct framework event type");
//            US_TEST_CONDITION(fStopEvent.GetThrowable() == nullptr, "Check that no exception was thrown");
//        });
//
//        f.Stop();
//        waitForStopThread.join();
//#else
//        f.Stop();
//#endif
//
//        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, f));
//
//        US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Check framework bundle event listener")
//
//        // Test that uninstalling a framework throws an exception
//        try
//        {
//            f.Uninstall();
//            US_TEST_FAILED_MSG(<< "Failed to throw uninstall exception")
//        }
//        catch (const std::runtime_error&)
//        {
//
//        }
//        catch (...)
//        {
//            US_TEST_FAILED_MSG(<< "Failed to throw correct exception type")
//        }
//
//        // Test that all bundles in the Start state are stopped when the framework is stopped.
//        f.Start();
//#if defined(US_BUILD_SHARED_LIBS)
//        auto bundle = testing::InstallLib(f.GetBundleContext(), "TestBundleA");
//#else
//        auto bundle = testing::GetBundle("TestBundleA", f.GetBundleContext());
//#endif
//        US_TEST_CONDITION_REQUIRED(bundle, "Non-null bundle")
//        bundle.Start();
//        US_TEST_CONDITION(bundle.GetState() == Bundle::STATE_ACTIVE, "Check that TestBundleA is in an Active state");
//
//#ifdef US_ENABLE_THREADING_SUPPORT
//        // To test that the correct FrameworkEvent is returned, we must guarantee that the Framework
//        // is in a STARTING, ACTIVE, or STOPPING state.
//        waitForStopThread = std::thread([&f]() {
//            auto ev = f.WaitForStop(std::chrono::seconds(10));
//            US_TEST_CONDITION_REQUIRED(ev, "Test for valid framework event returned by Framework::WaitForStop()");
//            if (!ev && ev.GetType() == FrameworkEvent::Type::FRAMEWORK_ERROR) std::rethrow_exception(ev.GetThrowable());
//            US_TEST_CONDITION(ev.GetType() == FrameworkEvent::Type::FRAMEWORK_STOPPED, "Check that framework event is stopped");
//        });
//
//        // Stopping the framework stops all active bundles.
//        f.Stop();
//        waitForStopThread.join();
//#else
//        f.Stop();
//#endif
//        US_TEST_CONDITION(bundle.GetState() != Bundle::STATE_ACTIVE, "Check that TestBundleA is not active");
//        US_TEST_CONDITION(f.GetState() != Bundle::STATE_ACTIVE, "Check framework is not active");
//    }
//
//#ifdef US_ENABLE_THREADING_SUPPORT
//
//    void TestConcurrentFrameworkStart()
//    {
//      // test concurrent Framework starts.
//      auto f = FrameworkFactory().NewFramework();
//      f.Init();
//      int start_count{ 0 };
//      f.GetBundleContext().AddFrameworkListener([&start_count](const FrameworkEvent& ev) 
//                            { if (FrameworkEvent::Type::FRAMEWORK_STARTED == ev.GetType()) ++start_count; });
//      size_t num_threads{ 100 };
//      std::vector<std::thread> threads;
//      for (size_t i = 0; i < num_threads; ++i)
//      {
//          threads.push_back(std::thread{ [&f]()
//          {
//              f.Start(); 
//          } });
//      }
//
//      for (auto& t : threads) t.join();
//
//      // To test that the correct FrameworkEvent is returned, we must guarantee that the Framework
//      // is in a STARTING, ACTIVE, or STOPPING state.
//      std::thread waitForStopThread([&f]() {
//          auto fEvent = f.WaitForStop(std::chrono::milliseconds::zero());
//          US_TEST_CONDITION_REQUIRED(fEvent, "Check for a valid framework event returned from WaitForStop");
//          US_TEST_CONDITION(fEvent.GetType() == FrameworkEvent::Type::FRAMEWORK_STOPPED, "Check that framework event is stopped");
//          US_TEST_CONDITION(fEvent.GetThrowable() == nullptr, "Check that no exception was thrown");
//      });
//
//      f.Stop();
//      waitForStopThread.join();
//
//      // Its somewhat ambiguous in the OSGi spec whether or not multiple Framework STARTED events should be sent
//      // when repeated calls to Framework::Start() are made on the same Framework instance once its in the 
//      // ACTIVE bundle state.
//      // Felix and Knopflerfish OSGi implementations take two different stances.
//      // Lock down the behavior that only one Framework STARTED event is sent.
//      US_TEST_CONDITION_REQUIRED(1 == start_count, "Multiple Framework::Start() calls only produce one Framework STARTED event.");
//    }
//
//    void TestConcurrentFrameworkStop()
//    {
//        // test concurrent Framework stops.
//        auto f = FrameworkFactory().NewFramework();
//        f.Start();
//
//        // To test that the correct FrameworkEvent is returned, we must guarantee that the Framework
//        // is in a STARTING, ACTIVE, or STOPPING state.
//        std::thread waitForStopThread([&f]() {
//            auto fEvent = f.WaitForStop(std::chrono::milliseconds::zero());
//            US_TEST_CONDITION_REQUIRED(fEvent, "Check for a valid framework event returned from WaitForStop");
//            US_TEST_CONDITION(fEvent.GetType() == FrameworkEvent::Type::FRAMEWORK_STOPPED, "Check that framework event is stopped");
//            US_TEST_CONDITION(fEvent.GetThrowable() == nullptr, "Check that no exception was thrown");
//        });
//
//        size_t num_threads{ 100 };
//        std::vector<std::thread> threads;
//        for (size_t i = 0; i < num_threads; ++i)
//        {
//            threads.push_back(std::thread{ [&f]()
//            {
//                f.Stop();
//            } });
//        }
//
//        for (auto& t : threads) t.join();
//        waitForStopThread.join();
//    }
//
//    void TestConcurrentFrameworkWaitForStop()
//    {
//        // test concurrent Framework stops.
//        auto f = FrameworkFactory().NewFramework();
//        f.Start();
//
//        std::mutex m;
//        size_t num_threads{ 100 };
//        std::vector<std::thread> threads;
//        for (size_t i = 0; i < num_threads; ++i)
//        {
//            // To test that the correct FrameworkEvent is returned, we must guarantee that the Framework
//            // is in a STARTING, ACTIVE, or STOPPING state.
//            threads.push_back(std::thread([&f, &m]() {
//                auto fEvent = f.WaitForStop(std::chrono::milliseconds::zero());
//                std::unique_lock<std::mutex> lock(m);
//                US_TEST_CONDITION_REQUIRED(fEvent, "Check for a valid framework event returned from WaitForStop");
//                US_TEST_CONDITION(fEvent.GetType() == FrameworkEvent::Type::FRAMEWORK_STOPPED, "Check that framework event is stopped");
//                US_TEST_CONDITION(fEvent.GetThrowable() == nullptr, "Check that no exception was thrown");
//            }));
//        }
//
//        f.Stop();
//        for (auto& t : threads) t.join();
//    }
//
//#endif
//
//    void TestEvents()
//    {
//        TestBundleListener listener;
//        std::vector<BundleEvent> pEvts;
//        std::vector<BundleEvent> pStopEvts;
//
//        auto f = FrameworkFactory().NewFramework();
//
//        f.Start();
//
//        auto fmc = f.GetBundleContext();
//        fmc.AddBundleListener(&listener, &TestBundleListener::BundleChanged);
//#ifdef US_BUILD_SHARED_LIBS
//        auto install = [&pEvts, &fmc](const std::string& libName)
//        {
//          auto bundle = testing::InstallLib(fmc, libName);
//          pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundle));
//          if (bundle.GetSymbolicName() == "TestBundleB")
//          {
//            // This is an additional install event from the bundle
//            // that is statically imported by TestBundleB.
//            pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, testing::GetBundle("TestBundleImportedByB", fmc)));
//          }
//        };
//
//        // The bundles used to test bundle events when stopping the framework.
//        // For static builds, the order of the "install" calls is imported.
//
//        install("TestBundleA");
//        install("TestBundleA2");
//        install("TestBundleB");
//#ifdef US_ENABLE_THREADING_SUPPORT
//        install("TestBundleC1");
//#endif
//        install("TestBundleH");
//        install("TestBundleLQ");
//        install("TestBundleM");
//        install("TestBundleR");
//        install("TestBundleRA");
//        install("TestBundleRL");
//        install("TestBundleS");
//        install("TestBundleSL1");
//        install("TestBundleSL3");
//        install("TestBundleSL4");
//        auto bundles(fmc.GetBundles());
//#else
//        // since all bundles are embedded in the main executable, all bundles are
//        // installed at framework start. simply check for start and stop events
//        std::vector<cppmicroservices::Bundle> bundles;
//        bundles.push_back(testing::GetBundle("TestBundleA", fmc));
//        bundles.push_back(testing::GetBundle("TestBundleA2", fmc));
//        bundles.push_back(testing::GetBundle("TestBundleB", fmc));
//#ifdef US_ENABLE_THREADING_SUPPORT
//        bundles.push_back(testing::GetBundle("TestBundleC1", fmc));
//#endif
//        bundles.push_back(testing::GetBundle("TestBundleH", fmc));
//        bundles.push_back(testing::GetBundle("TestBundleImportedByB", fmc));
//        bundles.push_back(testing::GetBundle("TestBundleLQ", fmc));
//        bundles.push_back(testing::GetBundle("TestBundleM", fmc));
//        bundles.push_back(testing::GetBundle("TestBundleR", fmc));
//        bundles.push_back(testing::GetBundle("TestBundleRA", fmc));
//        bundles.push_back(testing::GetBundle("TestBundleRL", fmc));
//        bundles.push_back(testing::GetBundle("TestBundleS", fmc));
//        bundles.push_back(testing::GetBundle("TestBundleSL1", fmc));
//        bundles.push_back(testing::GetBundle("TestBundleSL3", fmc));
//        bundles.push_back(testing::GetBundle("TestBundleSL4", fmc));
//        bundles.push_back(testing::GetBundle("main", fmc));
//#endif
//
//
//      
//        for (auto& bundle : bundles)
//        {
//          bundle.Start();
//          // no events will be fired for the framework, its already active at this point
//          if (bundle != f)
//          {
//            pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundle));
//            pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundle));
//            pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundle));
//
//            // bundles will be stopped in the reverse order in which they were started.
//            // It is easier to maintain this test if the stop events are setup in the
//            // right order here, while the bundles are starting, instead of hard coding
//            // the order of events somewhere else.
//            // Doing it this way also tests the order in which starting and stopping
//            // bundles occurs and when their events are fired.
//            pStopEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundle));
//            pStopEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundle));
//          }
//        }
//        // Remember, the framework is stopped first, before all bundles are stopped.
//        pStopEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, f));
//        std::reverse(pStopEvts.begin(), pStopEvts.end());
//
//
//        US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Check for bundle start events")
//
//        // Stopping the framework stops all active bundles.
//        f.Stop();
//        f.WaitForStop(std::chrono::milliseconds::zero());
//
//        US_TEST_CONDITION(listener.CheckListenerEvents(pStopEvts), "Check for bundle stop events")
//    }
}
//
//void TestIndirectFrameworkStop()
//{
//    auto f = FrameworkFactory().NewFramework();
//    f.Start();
//    auto bundle = f.GetBundleContext().GetBundle(0);
//    bundle.Stop();
//    Framework f2(bundle);
//    f2.WaitForStop(std::chrono::milliseconds::zero());
//    US_TEST_CONDITION(f.GetState() == Bundle::STATE_RESOLVED, "Framework stopped");
//}
//
//void TestShutdownAndStart()
//{
//  int startCount = 0;
//
//  FrameworkEvent fwEvt;
//  {
//    auto f = FrameworkFactory().NewFramework();
//    f.Init();
//
//    auto ctx = f.GetBundleContext();
//
//    ctx.AddFrameworkListener([&startCount](const FrameworkEvent& fwEvt) {
//      if (fwEvt.GetType() == FrameworkEvent::FRAMEWORK_STARTED)
//      {
//        ++startCount;
//        auto bundle = fwEvt.GetBundle();
//        US_TEST_CONDITION_REQUIRED(bundle.GetBundleId() == 0, "Got framework bundle")
//        US_TEST_CONDITION_REQUIRED(bundle.GetState() == Bundle::STATE_ACTIVE, "Started framework");
//
//        // This stops the framework
//        bundle.Stop();
//      }
//    });
//
//    US_TEST_CONDITION_REQUIRED(f.GetState() == Framework::STATE_STARTING, "Starting framework")
//    f.Start();
//
//    // Wait for stop
//    fwEvt = f.WaitForStop(std::chrono::milliseconds::zero());
//  }
//
//  US_TEST_CONDITION_REQUIRED(fwEvt.GetType() == FrameworkEvent::FRAMEWORK_STOPPED, "Stopped framework event")
//
//  Bundle fwBundle = fwEvt.GetBundle();
//  US_TEST_CONDITION_REQUIRED(fwBundle.GetState() == Bundle::STATE_RESOLVED, "Resolved framework");
//
//  // Start the framework again
//  fwBundle.Start();
//
//  US_TEST_CONDITION_REQUIRED(fwBundle.GetState() == Bundle::STATE_ACTIVE, "Active framework");
//
//  US_TEST_CONDITION_REQUIRED(startCount == 1, "One framework start notification")
//}
//
//#ifdef US_BUILD_SHARED_LIBS
//void TestSharedLibraryStaticDestruction()
//{
//  // If a framework object is held as a static within a DLL
//  // implicit destruction of the framework object will cause
//  // either a crash or a hang on Windows.
//  // This can be mitigated by explicitly stopping the framework
//  // object before static destruction occurs.
//
//  // This test is meant to ensure explicitly stopping a static
//  // framework object does not crash or hang a process.
//
//  // NOTE: If this process crashes or hangs, this test has failed.
//
//  auto framework = singleton::testing::getFramework();
//  framework->Stop();
//  framework->WaitForStop(std::chrono::milliseconds::zero());
//}
//#endif

int FrameworkTest(int /*argc*/, char* /*argv*/[])
{
    US_TEST_BEGIN("FrameworkTest");

    try {
        TestDefaultConfig();
        //    TestDefaultLogSink();
        TestCustomConfig();
        TestCustomLogSink();
        TestProperties();
    }
    catch (...) {
        try {
            std::rethrow_exception(std::current_exception());
        }
        catch (const std::exception& e) {
            std::cout << "Exception caught: " << e.what() << "\n";
        }
        catch (...) {
            std::cout << "Unknown exception caught.\n";
        }
    }
//    TestIndirectFrameworkStop();
//    TestShutdownAndStart();
//    TestLifeCycle();
//    TestEvents();
#ifdef US_BUILD_SHARED_LIBS
//    TestSharedLibraryStaticDestruction();
#endif
#ifdef US_ENABLE_THREADING_SUPPORT
//    TestConcurrentFrameworkStart();
//    TestConcurrentFrameworkStop();
//    TestConcurrentFrameworkWaitForStop();
#endif

    // try and catch a crash on Mac. Turn on the diagnostic logger to try and get
    // an idea of what is going on.
    auto framework = FrameworkFactory().NewFramework(std::map<std::string, Any>{ {cppmicroservices::Constants::FRAMEWORK_LOG, Any(true)} });
    framework.Start();
    framework.Stop();

    US_TEST_END()
}
