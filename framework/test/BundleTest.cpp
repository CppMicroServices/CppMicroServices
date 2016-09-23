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
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/ServiceEvent.h"

#include "TestDriverActivator.h"
#include "TestingConfig.h"
#include "TestingMacros.h"
#include "TestUtilBundleListener.h"
#include "TestUtils.h"
#include "Utils.h" // cppmicroservices::ToString()

#include <thread>

using namespace cppmicroservices;

class FrameworkTestSuite
{

  TestBundleListener listener;

  BundleContext bc;
  Bundle bu;
  Bundle buExec;
  Bundle buA;

public:

  FrameworkTestSuite(const BundleContext& bc)
    : bc(bc)
    , bu(bc.GetBundle())
  {}

  void setup()
  {
    try
    {
      bc.AddBundleListener(&listener, &TestBundleListener::BundleChanged);
    }
    catch (const std::logic_error& ise)
    {
      US_TEST_OUTPUT( << "bundle listener registration failed " << ise.what() );
      throw;
    }

    try
    {
      bc.AddServiceListener(&listener, &TestBundleListener::ServiceChanged);
    }
    catch (const std::logic_error& ise)
    {
      US_TEST_OUTPUT( << "service listener registration failed " << ise.what() );
      throw;
    }
  }

  void cleanup()
  {
    std::vector<Bundle> bundles =  {
      buA,
      buExec
    };

    for(auto& b : bundles)
    {
      try
      {
        if (b) b.Uninstall();
      }
      catch (...) { /* ignored */ }
    }

    buExec = nullptr;
    buA = nullptr;

    try
    {
      bc.RemoveBundleListener(&listener, &TestBundleListener::BundleChanged);
    }
    catch (...) { /* ignored */ }

    try
    {
      bc.RemoveServiceListener(&listener, &TestBundleListener::ServiceChanged);
    }
    catch (...) { /* ignored */ }

  }


//----------------------------------------------------------------------------
//Test result of GetService(ServiceReference()). Should throw std::invalid_argument
void frame018a()
{
  try
  {
    bc.GetService(ServiceReferenceU());
    US_TEST_FAILED_MSG(<< "Got service object, expected std::invalid_argument exception")
  }
  catch (const std::invalid_argument& )
  {}
  catch (...)
  {
    US_TEST_FAILED_MSG(<< "Got wrong exception, expected std::invalid_argument")
  }
}

// Load libA and check that it exists and that its expected service does not exist,
// Also check that the expected events in the framework occurs
void frame020a()
{
  buA = testing::InstallLib(bc, "TestBundleA");
  US_TEST_CONDITION_REQUIRED(buA, "Test for existing bundle TestBundleA")
  US_TEST_CONDITION(buA.GetSymbolicName() == "TestBundleA", "Test bundle name")

  US_TEST_CONDITION_REQUIRED(buA.GetState() == Bundle::STATE_INSTALLED, "Test bundle A in state installed")

  US_TEST_CONDITION(buA.GetLastModified() > Clock::time_point(), "Test bundle A last modified")
  US_TEST_CONDITION(buA.GetLastModified() <= Clock::now(), "Test bundle A last modified")

  // Check that no service reference exist yet.
  ServiceReferenceU sr1 = bc.GetServiceReference("cppmicroservices::TestBundleAService");
  US_TEST_CONDITION_REQUIRED(!sr1, "service from bundle A must not exist yet")

  // check the listeners for events
  std::vector<BundleEvent> pEvts;
#ifdef US_BUILD_SHARED_LIBS // The bundle is installed at framework startup for static builds
  pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, buA));
  bool relaxed = false;
#else
  bool relaxed = true;
#endif
  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, relaxed), "Test for unexpected events");
}

// Start libA and check that it gets state ACTIVE
// and that the service it registers exist
void frame025a()
{
  buA.Start();
  US_TEST_CONDITION_REQUIRED(buA.GetState() == Bundle::STATE_ACTIVE, "Test bundle A in state active")

  try
  {
    // Check if testbundleA registered the expected service
    ServiceReferenceU sr1 = bc.GetServiceReference("cppmicroservices::TestBundleAService");
    US_TEST_CONDITION_REQUIRED(sr1, "expecting service")

    auto o1 = bc.GetService(sr1);
    US_TEST_CONDITION_REQUIRED(o1 && !o1->empty(), "no service object found")

    // check the listeners for events
    std::vector<BundleEvent> pEvts;
    pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, buA));
    pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, buA));
    pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, buA));

    std::vector<ServiceEvent> seEvts;
    seEvts.push_back(ServiceEvent(ServiceEvent::SERVICE_REGISTERED, sr1));

    US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");
  }
  catch (const ServiceException& /*se*/)
  {
    US_TEST_FAILED_MSG(<< "test bundle, expected service not found");
  }
}


// Start libA and check that it exists and that the storage paths are correct
void frame020b()
{
  buA = testing::InstallLib(bc, "TestBundleA");
  US_TEST_CONDITION_REQUIRED(buA, "Test for existing bundle TestBundleA");

  buA.Start();

  US_TEST_CONDITION(bc.GetBundle().GetProperty(Constants::FRAMEWORK_STORAGE).ToString() == testing::GetTempDirectory(), "Test for valid base storage path");

  // launching properties should be accessible through any bundle
  US_TEST_CONDITION(buA.GetProperty(Constants::FRAMEWORK_STORAGE).ToString() == testing::GetTempDirectory(), "Test for valid base storage path");

  std::cout << "Framework Storage Base Directory: " << bc.GetDataFile("") << std::endl;
  const std::string baseStoragePath = testing::GetTempDirectory() + testing::DIR_SEP + "data" + testing::DIR_SEP + cppmicroservices::ToString(buA.GetBundleId()) + testing::DIR_SEP;
  US_TEST_CONDITION(buA.GetBundleContext().GetDataFile("") == baseStoragePath, "Test for valid data path");
  US_TEST_CONDITION(buA.GetBundleContext().GetDataFile("bla") == (baseStoragePath + "bla"), "Test for valid data file path");

  US_TEST_CONDITION(buA.GetState() & Bundle::STATE_ACTIVE, "Test if started correctly");
}

// Stop libA and check for correct events
void frame030b()
{
  ServiceReferenceU sr1
      = buA.GetBundleContext().GetServiceReference("cppmicroservices::TestBundleAService");
  US_TEST_CONDITION(sr1, "Test for valid service reference")

  try
  {
    auto lm = buA.GetLastModified();
    buA.Stop();
    US_TEST_CONDITION(!(buA.GetState() & Bundle::STATE_ACTIVE), "Test for stopped state")
    US_TEST_CONDITION(lm == buA.GetLastModified(), "Unchanged last modified time after stop")
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Stop bundle exception: " << e.what())
  }

  std::vector<BundleEvent> pEvts;
  pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, buA));
  pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, buA));

  std::vector<ServiceEvent> seEvts;
  seEvts.push_back(ServiceEvent(ServiceEvent::SERVICE_UNREGISTERING, sr1));

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");
}

// Check that the executable's activator was started and called
void frame035a()
{
  try
  {
    buExec = testing::GetBundle("main", bc);
    US_TEST_CONDITION_REQUIRED(buExec, "Bundle 'main' not found");
    buExec.Start();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Install bundle exception: " << e.what() << " + in frame01:FAIL")
  }

  US_TEST_CONDITION_REQUIRED(TestDriverActivator::StartCalled(), "BundleActivator::Start() called for executable")

  long systemId = 0;
  // check expected meta-data
  US_TEST_CONDITION("main" == buExec.GetSymbolicName(), "Test bundle name")
  US_TEST_CONDITION(BundleVersion(0,1,0) == buExec.GetVersion(), "Test test driver bundle version")
  US_TEST_CONDITION(BundleVersion(CppMicroServices_MAJOR_VERSION, CppMicroServices_MINOR_VERSION, CppMicroServices_PATCH_VERSION) == buExec.GetBundleContext().GetBundle(systemId).GetVersion(), "Test CppMicroServices version")
}


// Get location, persistent storage and status of the bundle
void frame037a()
{
  std::string location = buExec.GetLocation();
  std::cout << "LOCATION:" << location;
  US_TEST_CONDITION(!location.empty(), "Test for non-empty bundle location")

  US_TEST_CONDITION(buExec.GetState() & Bundle::STATE_ACTIVE, "Test for started flag")

  // launching properties should be accessible through any bundle
  auto p1 = bc.GetBundle().GetProperty(Constants::FRAMEWORK_UUID);
  auto p2 = buExec.GetProperty(Constants::FRAMEWORK_UUID);
  US_TEST_CONDITION(!p1.Empty() && p1.ToString() == p2.ToString(), "Test for uuid accesible from framework and bundle")

  std::cout << buExec.GetBundleContext().GetDataFile("") << std::endl;
  const std::string baseStoragePath = testing::GetCurrentWorkingDirectory();

  US_TEST_CONDITION(buExec.GetBundleContext().GetDataFile("").substr(0, baseStoragePath.size()) == baseStoragePath, "Test for valid data path")
  US_TEST_CONDITION(buExec.GetBundleContext().GetDataFile("bla").substr(0, baseStoragePath.size()) == baseStoragePath, "Test for valid data file path")
}



struct LocalListener {
  void ServiceChanged(const ServiceEvent&) {}
};

// Add a service listener with a broken LDAP filter to Get an exception
void frame045a()
{
  LocalListener sListen1;
  std::string brokenFilter = "A broken LDAP filter";

  try
  {
    bc.AddServiceListener(&sListen1, &LocalListener::ServiceChanged, brokenFilter);
    US_TEST_FAILED_MSG(<< "test bundle, no exception on broken LDAP filter:");
  }
  catch (const std::invalid_argument& /*ia*/)
  {
    //assertEquals("InvalidSyntaxException.GetFilter should be same as input string", brokenFilter, ise.GetFilter());
  }
  catch (...)
  {
    US_TEST_FAILED_MSG(<< "test bundle, wrong exception on broken LDAP filter:");
  }
}

};

namespace {


// Verify that the same member function pointers registered as listeners
// with different receivers works.
void TestListenerFunctors()
{
  TestBundleListener listener1;
  TestBundleListener listener2;

  FrameworkFactory factory;
  auto framework = factory.NewFramework();
  framework.Start();

  auto bc = framework.GetBundleContext();

  try
  {
    bc.RemoveBundleListener(&listener1, &TestBundleListener::BundleChanged);
    bc.AddBundleListener(&listener1, &TestBundleListener::BundleChanged);
    bc.RemoveBundleListener(&listener2, &TestBundleListener::BundleChanged);
    bc.AddBundleListener(&listener2, &TestBundleListener::BundleChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_FAILED_MSG( << "bundle listener registration failed " << ise.what()
                        << " : frameSL02a:FAIL" );
  }

  auto bundleA = testing::InstallLib(bc, "TestBundleA");
  US_TEST_CONDITION_REQUIRED(bundleA, "Test for existing bundle TestBundleA")

  bundleA.Start();

  std::vector<BundleEvent> pEvts;
#ifdef US_BUILD_SHARED_LIBS
  pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundleA));
#endif
  pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundleA));
  pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundleA));
  pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundleA));

  std::vector<ServiceEvent> seEvts;

  bool relaxed = false;
#ifndef US_BUILD_SHARED_LIBS
  relaxed = true;
#endif
  US_TEST_CONDITION(listener1.CheckListenerEvents(pEvts, seEvts, relaxed), "Check first bundle listener")
  US_TEST_CONDITION(listener2.CheckListenerEvents(pEvts, seEvts, relaxed), "Check second bundle listener")

  bc.RemoveBundleListener(&listener1, &TestBundleListener::BundleChanged);
  bc.RemoveBundleListener(&listener2, &TestBundleListener::BundleChanged);

  bundleA.Stop();
}


void TestBundleStates()
{
    TestBundleListener listener;
    std::vector<BundleEvent> bundleEvents;
    FrameworkFactory factory;

    std::map<std::string, Any> frameworkConfig;
    auto framework = factory.NewFramework(frameworkConfig);
    framework.Start();

    auto frameworkCtx = framework.GetBundleContext();
    frameworkCtx.AddBundleListener(&listener, &TestBundleListener::BundleChanged);
  
    // Test Start -> Stop for auto-installed bundles
    auto bundles = frameworkCtx.GetBundles();
    for (auto& bundle : bundles)
    {
      if (bundle != framework)
      {
        US_TEST_CONDITION(bundle.GetState() & Bundle::STATE_INSTALLED, "Test installed bundle state")
        bundle.Start();
        US_TEST_CONDITION(bundle.GetState() & Bundle::STATE_ACTIVE, "Test started bundle state")
        bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundle));
        bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundle));
        bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundle));
        bundle.Stop();
        US_TEST_CONDITION((bundle.GetState() & Bundle::STATE_ACTIVE) == false, "Test stopped bundle state")
        bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundle));
        bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundle));
      }
    }
    US_TEST_CONDITION(listener.CheckListenerEvents(bundleEvents, false), "Test for unexpected events");
    bundleEvents.clear();
  
#ifdef US_BUILD_SHARED_LIBS    // following combination can be tested only for shared library builds
    Bundle bundle;
    // Test install -> uninstall
    // expect 2 event (BUNDLE_INSTALLED, BUNDLE_UNINSTALLED)
    bundle = testing::InstallLib(frameworkCtx, "TestBundleA");
    US_TEST_CONDITION(bundle, "Test non-empty bundle")
    US_TEST_CONDITION(bundle.GetState() & Bundle::STATE_INSTALLED, "Test installed bundle state")
    bundle.Uninstall();
    US_TEST_CONDITION(!frameworkCtx.GetBundle(bundle.GetBundleId()), "Test bundle install -> uninstall")
    US_TEST_CONDITION(bundle.GetState() & Bundle::STATE_UNINSTALLED, "Test uninstalled bundle state")
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNRESOLVED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, bundle));


    US_TEST_CONDITION(listener.CheckListenerEvents(bundleEvents, false), "Test for unexpected events");
    bundleEvents.clear();

    // Test install -> start -> uninstall
    // expect 6 events (BUNDLE_INSTALLED, BUNDLE_STARTING, BUNDLE_STARTED, BUNDLE_STOPPING, BUNDLE_STOPPED, BUNDLE_UNINSTALLED)
    bundle = testing::InstallLib(frameworkCtx, "TestBundleA");
    US_TEST_CONDITION(bundle, "Test non-empty bundle")
    US_TEST_CONDITION(bundle.GetState() & Bundle::STATE_INSTALLED, "Test installed bundle state")
    bundle.Start();
    US_TEST_CONDITION(bundle.GetState() & Bundle::STATE_ACTIVE, "Test started bundle state")
    bundle.Uninstall();
    US_TEST_CONDITION(bundle.GetState() & Bundle::STATE_UNINSTALLED, "Test uninstalled bundle state")
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNRESOLVED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, bundle));
    US_TEST_CONDITION(listener.CheckListenerEvents(bundleEvents), "Test for unexpected events");
    bundleEvents.clear();

    // Test install -> stop -> uninstall
    // expect 2 event (BUNDLE_INSTALLED, BUNDLE_UNINSTALLED)
    bundle = testing::InstallLib(frameworkCtx, "TestBundleA");
    US_TEST_CONDITION(bundle, "Test non-empty bundle")
    US_TEST_CONDITION(bundle.GetState() & Bundle::STATE_INSTALLED, "Test installed bundle state")
    bundle.Stop();
    US_TEST_CONDITION((bundle.GetState() & Bundle::STATE_ACTIVE) == false, "Test stopped bundle state")
    bundle.Uninstall();
    US_TEST_CONDITION(bundle.GetState() & Bundle::STATE_UNINSTALLED, "Test uninstalled bundle state")
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNRESOLVED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, bundle));
    US_TEST_CONDITION(listener.CheckListenerEvents(bundleEvents), "Test for unexpected events");
    bundleEvents.clear();

    // Test install -> start -> stop -> uninstall
    // expect 6 events (BUNDLE_INSTALLED, BUNDLE_STARTING, BUNDLE_STARTED, BUNDLE_STOPPING, BUNDLE_STOPPED, BUNDLE_UNINSTALLED)
    bundle = testing::InstallLib(frameworkCtx, "TestBundleA");
    auto lm = bundle.GetLastModified();
    US_TEST_CONDITION(bundle, "Test non-empty bundle")
    US_TEST_CONDITION(bundle.GetState() & Bundle::STATE_INSTALLED, "Test installed bundle state")
    bundle.Start();
    US_TEST_CONDITION(bundle.GetState() & Bundle::STATE_ACTIVE, "Test started bundle state")
    bundle.Stop();
    US_TEST_CONDITION((bundle.GetState() & Bundle::STATE_ACTIVE) == false, "Test stopped bundle state")
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    bundle.Uninstall();
    US_TEST_CONDITION(bundle.GetState() & Bundle::STATE_UNINSTALLED, "Test uninstalled bundle state")
    US_TEST_CONDITION(lm < bundle.GetLastModified(), "Last modified time changed after uninstall")
    US_TEST_CONDITION(bundle.GetLastModified() <= Clock::now(), "Last modified time <= now")
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNRESOLVED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, bundle));
    US_TEST_CONDITION(listener.CheckListenerEvents(bundleEvents), "Test for unexpected events");
    bundleEvents.clear();
#endif
}

void TestForInstallFailure()
{
    FrameworkFactory factory;

    auto framework = factory.NewFramework();
    framework.Start();

    auto frameworkCtx = framework.GetBundleContext();

    // Test that bogus bundle installs throw the appropriate exception
    try
    {
        frameworkCtx.InstallBundles(std::string());
        US_TEST_FAILED_MSG(<< "Failed to throw a std::runtime_error")
    }
    catch (const std::runtime_error& ex)
    {
        US_TEST_OUTPUT(<< "Caught std::runtime_exception: " << ex.what())
    }
    catch (...)
    {
        US_TEST_FAILED_MSG(<< "Failed to throw a std::runtime_error")
    }

    try
    {
        frameworkCtx.InstallBundles(std::string("\\path\\which\\won't\\exist\\phantom_bundle"));
        US_TEST_FAILED_MSG(<< "Failed to throw a std::runtime_error")
    }
    catch (const std::runtime_error& ex)
    {
        US_TEST_OUTPUT(<< "Caught std::runtime_exception: " << ex.what())
    }
    catch (...)
    {
        US_TEST_FAILED_MSG(<< "Failed to throw a std::runtime_error")
    }
#ifdef US_BUILD_SHARED_LIBS
    // 2 bundles - the framework(system_bundle) and the executable(main).
    US_TEST_CONDITION(2 == frameworkCtx.GetBundles().size(), "Test # of installed bundles")
#else
    // There are atleast 2 bundles, maybe more depending on how the executable is created
    US_TEST_CONDITION(2 <= frameworkCtx.GetBundles().size(), "Test # of installed bundles")
#endif
}

void TestDuplicateInstall()
{
    FrameworkFactory factory;

    auto framework = factory.NewFramework();
    framework.Start();

    auto frameworkCtx = framework.GetBundleContext();

    // Test installing the same bundle (i.e. a bundle with the same location) twice.
    // The exact same bundle should be returned on the second install.
    auto bundle = testing::InstallLib(frameworkCtx, "TestBundleA");
    auto bundleDuplicate = testing::InstallLib(frameworkCtx, "TestBundleA");

    US_TEST_CONDITION(bundle == bundleDuplicate, "Test for the same bundle instance");
    US_TEST_CONDITION(bundle.GetBundleId() == bundleDuplicate.GetBundleId(), "Test for the same bundle id");
}
  
void TestAutoInstallEmbeddedBundles()
{
  FrameworkFactory factory;
  auto f = factory.NewFramework();
  f.Start();
  auto frameworkCtx = f.GetBundleContext();
  US_TEST_FOR_EXCEPTION_BEGIN(std::runtime_error)
  frameworkCtx.InstallBundles(testing::BIN_PATH + testing::DIR_SEP + "usFrameworkTestDriver" + testing::EXE_EXT);
  US_TEST_FOR_EXCEPTION_END(std::runtime_error)
#ifdef US_BUILD_SHARED_LIBS
  // 2 bundles - the framework(system_bundle) and the executable(main).
  US_TEST_CONDITION(2 == frameworkCtx.GetBundles().size(), "Test # of installed bundles")
#else
  // There are atleast 2 bundles, maybe more depending on how the executable is created
  US_TEST_CONDITION(2 <= frameworkCtx.GetBundles().size(), "Test # of installed bundles")
#endif
  auto bundle = frameworkCtx.GetBundles();
  US_TEST_FOR_EXCEPTION_BEGIN(std::runtime_error)
  bundle[0].Uninstall();
  US_TEST_FOR_EXCEPTION_END(std::runtime_error)
}

}

int BundleTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("BundleTest");

  {
    auto framework = FrameworkFactory().NewFramework();
    framework.Start();

    auto bundles = framework.GetBundleContext().GetBundles();
    for (auto const& bundle : bundles)
    {
      std::cout << "----- " << bundle.GetSymbolicName() << std::endl;
    }

    FrameworkTestSuite ts(framework.GetBundleContext());

    ts.setup();

    ts.frame018a();

    ts.frame020a();
    ts.frame025a();
    ts.frame030b();

    ts.frame035a();
    ts.frame037a();
    ts.frame045a();

    ts.cleanup();
  }


  // test a non-default framework instance using a different persistent storage location.
  {
    std::map<std::string, Any> frameworkConfig;
    frameworkConfig[Constants::FRAMEWORK_STORAGE] = testing::GetTempDirectory();
    auto framework = FrameworkFactory().NewFramework(frameworkConfig);
    framework.Start();

    FrameworkTestSuite ts(framework.GetBundleContext());
    ts.setup();
    ts.frame020b();
    ts.cleanup();
  }

  TestListenerFunctors();
  TestBundleStates();
  TestForInstallFailure();
  TestDuplicateInstall();
  TestAutoInstallEmbeddedBundles();

  US_TEST_END()
}
