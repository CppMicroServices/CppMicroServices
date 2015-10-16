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

#include <usBundle.h>
#include <usBundleEvent.h>
#include <usServiceEvent.h>
#include <usBundleContext.h>
#include <usGetBundleContext.h>
#include <usBundleActivator.h>
#include <usBundleSettings.h>
#include <usLog.h>

#include "usTestUtils.h"
#include "usTestUtilBundleListener.h"
#include "usTestDriverActivator.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

using namespace us;

namespace {

// Check that the executable's activator was started and called
void frame01(BundleContext* mc)
{
  try
  {
    Bundle* bundle = mc->InstallBundle(BIN_PATH + DIR_SEP + "usCoreTestDriver" + EXE_EXT + "/main");
    US_TEST_CONDITION_REQUIRED(bundle != NULL, "Test installation of bundle main")

    bundle->Start();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Install bundle exception: " << e.what() << " + in frame01:FAIL")
  }

  US_TEST_CONDITION_REQUIRED(TestDriverActivator::StartCalled(), "BundleActivator::Start() called for executable")
}

// Verify that the same member function pointers registered as listeners
// with different receivers works.
void frame02a(BundleContext* mc)
{
  TestBundleListener listener1;
  TestBundleListener listener2;

  try
  {
    mc->RemoveBundleListener(&listener1, &TestBundleListener::BundleChanged);
    mc->AddBundleListener(&listener1, &TestBundleListener::BundleChanged);
    mc->RemoveBundleListener(&listener2, &TestBundleListener::BundleChanged);
    mc->AddBundleListener(&listener2, &TestBundleListener::BundleChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_FAILED_MSG( << "bundle listener registration failed " << ise.what()
                        << " : frameSL02a:FAIL" );
  }

  InstallTestBundle(mc, "TestBundleA");

  Bundle* bundleA = mc->GetBundle("TestBundleA");
  US_TEST_CONDITION_REQUIRED(bundleA != nullptr, "Test for existing bundle TestBundleA")

  bundleA->Start();

  std::vector<BundleEvent> pEvts;
  pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundleA));
  pEvts.push_back(BundleEvent(BundleEvent::STARTING, bundleA));
  pEvts.push_back(BundleEvent(BundleEvent::STARTED, bundleA));

  std::vector<ServiceEvent> seEvts;

  US_TEST_CONDITION(listener1.CheckListenerEvents(pEvts, seEvts), "Check first bundle listener")
  US_TEST_CONDITION(listener2.CheckListenerEvents(pEvts, seEvts), "Check second bundle listener")

  mc->RemoveBundleListener(&listener1, &TestBundleListener::BundleChanged);
  mc->RemoveBundleListener(&listener2, &TestBundleListener::BundleChanged);

  bundleA->Stop();

}

// Verify information from the BundleInfo struct
void frame005a(BundleContext* mc)
{
  Bundle* m = mc->GetBundle();

  // check expected headers

  US_TEST_CONDITION("main" == m->GetName(), "Test bundle name")
  US_TEST_CONDITION(BundleVersion(0,1,0) == m->GetVersion(), "Test test driver bundle version")
  US_TEST_CONDITION(BundleVersion(CppMicroServices_MAJOR_VERSION, CppMicroServices_MINOR_VERSION, CppMicroServices_PATCH_VERSION) == mc->GetBundle(1)->GetVersion(), "Test CppMicroServices version")
}

// Get context id, location, persistent storage and status of the bundle
void frame010a(Framework* framework, BundleContext* mc)
{
  Bundle* m = mc->GetBundle();

  long int contextid = m->GetBundleId();
  US_DEBUG << "CONTEXT ID:" << contextid;

  std::string location = m->GetLocation();
  US_DEBUG << "LOCATION:" << location;
  US_TEST_CONDITION(!location.empty(), "Test for non-empty bundle location")

  US_TEST_CONDITION(m->IsStarted(), "Test for started flag")

  // launching properties should be accessible through any bundle
  US_TEST_CONDITION(framework->GetProperty("org.osgi.framework.storage").Empty(), "Test for empty base storage path")
  US_TEST_CONDITION(m->GetProperty("org.osgi.framework.storage").Empty(), "Test for empty base storage path")
  US_TEST_CONDITION(m->GetBundleContext()->GetDataFile("").empty(), "Test for empty data path")
  US_TEST_CONDITION(m->GetBundleContext()->GetDataFile("bla").empty(), "Test for empty data file path")
}

//----------------------------------------------------------------------------
//Test result of GetService(ServiceReference()). Should throw std::invalid_argument
void frame018a(BundleContext* mc)
{
  try
  {
    mc->GetService(ServiceReferenceU());
    US_DEBUG << "Got service object, expected std::invalid_argument exception";
    US_TEST_FAILED_MSG(<< "Got service object, excpected std::invalid_argument exception")
  }
  catch (const std::invalid_argument& )
  {}
  catch (...)
  {
    US_TEST_FAILED_MSG(<< "Got wrong exception, expected std::invalid_argument")
  }
}

// Start libA and check that it exists and that the service it registers exists,
// also check that the expected events occur
void frame020a(Framework* framework, TestBundleListener& listener)
{
  BundleContext* mc = framework->GetBundleContext();

  Bundle* bundleA = mc->GetBundle("TestBundleA");
  US_TEST_CONDITION_REQUIRED(bundleA != nullptr, "Test for existing bundle TestBundleA")

  US_TEST_CONDITION(bundleA->GetName() == "TestBundleA", "Test bundle name")

  bundleA->Start();

  // Check if libA registered the expected service
  try
  {
    ServiceReferenceU sr1 = mc->GetServiceReference("us::TestBundleAService");
    InterfaceMap o1 = mc->GetService(sr1);
    US_TEST_CONDITION(!o1.empty(), "Test if service object found");

    try
    {
      US_TEST_CONDITION(mc->UngetService(sr1), "Test if Service UnGet returns true");
    }
    catch (const std::logic_error le)
    {
      US_TEST_FAILED_MSG(<< "UnGetService exception: " << le.what())
    }

    // check the listeners for events
    std::vector<BundleEvent> pEvts;
    pEvts.push_back(BundleEvent(BundleEvent::STARTING, bundleA));
    pEvts.push_back(BundleEvent(BundleEvent::STARTED, bundleA));

    std::vector<ServiceEvent> seEvts;
    seEvts.push_back(ServiceEvent(ServiceEvent::REGISTERED, sr1));

    US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");

  }
  catch (const ServiceException& /*se*/)
  {
    US_TEST_FAILED_MSG(<< "test bundle, expected service not found");
  }

  US_TEST_CONDITION(bundleA->IsStarted() == true, "Test if started correctly");
}


// Start libA and check that it exists and that the storage paths are correct
void frame02b(Framework* framework)
{
  BundleContext* mc = framework->GetBundleContext();

  US_TEST_CONDITION(framework->GetProperty("org.osgi.framework.storage").ToString() == "/tmp", "Test for valid base storage path")

  Bundle* bundleA = mc->GetBundle("TestBundleA");
  US_TEST_CONDITION_REQUIRED(bundleA != nullptr, "Test for existing bundle TestBundleA")
  // launching properties should be accessible through any bundle
  US_TEST_CONDITION(bundleA->GetProperty("org.osgi.framework.storage").ToString() == "/tmp", "Test for valid base storage path")
  US_TEST_CONDITION(bundleA->GetName() == "TestBundleA", "Test bundle name")

  bundleA->Start();

  std::cout << bundleA->GetBundleContext()->GetDataFile("") << std::endl;
  std::stringstream ss;
  ss << bundleA->GetBundleId();
  const std::string baseStoragePath = std::string("/tmp") + DIR_SEP + ss.str() + "_TestBundleA" + DIR_SEP;
  US_TEST_CONDITION(bundleA->GetBundleContext()->GetDataFile("") == baseStoragePath, "Test for valid data path")
  US_TEST_CONDITION(bundleA->GetBundleContext()->GetDataFile("bla") == baseStoragePath + "bla", "Test for valid data file path")

  US_TEST_CONDITION(bundleA->IsStarted() == true, "Test if started correctly");
}


// Stop libA and check for correct events
void frame030b(BundleContext* mc, TestBundleListener& listener)
{
  Bundle* bundleA = mc->GetBundle("TestBundleA");
  US_TEST_CONDITION_REQUIRED(bundleA != nullptr, "Test for non-null bundle")

  ServiceReferenceU sr1
      = mc->GetServiceReference("us::TestBundleAService");
  US_TEST_CONDITION(sr1, "Test for valid service reference")

  try
  {
    bundleA->Stop();
    US_TEST_CONDITION(bundleA->IsStarted() == false, "Test for stopped state")
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Stop bundle exception: " << e.what())
  }

  std::vector<BundleEvent> pEvts;
  pEvts.push_back(BundleEvent(BundleEvent::STOPPING, bundleA));
  pEvts.push_back(BundleEvent(BundleEvent::STOPPED, bundleA));

  std::vector<ServiceEvent> seEvts;
  seEvts.push_back(ServiceEvent(ServiceEvent::UNREGISTERING, sr1));

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");
}


struct LocalListener {
  void ServiceChanged(const ServiceEvent&) {}
};

// Add a service listener with a broken LDAP filter to Get an exception
void frame045a(BundleContext* mc)
{
  LocalListener sListen1;
  std::string brokenFilter = "A broken LDAP filter";

  try
  {
    mc->AddServiceListener(&sListen1, &LocalListener::ServiceChanged, brokenFilter);
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

void TestBundleStates()
{
    TestBundleListener listener;
    std::vector<BundleEvent> bundleEvents;
    FrameworkFactory factory;

    Framework* framework = factory.NewFramework(std::map<std::string, std::string>());
    framework->Start();

    BundleContext* frameworkCtx = framework->GetBundleContext();
    frameworkCtx->AddBundleListener(&listener, &TestBundleListener::BundleChanged);

    Bundle* bundle = nullptr;

    // Test install -> uninstall
    // expect 2 event (INSTALLED, UNINSTALLED)
    bundle = InstallTestBundle(frameworkCtx, "TestBundleA");
    bundle->Uninstall();
    US_TEST_CONDITION(0 == frameworkCtx->GetBundle("TestBundleA"), "Test bundle install -> uninstall")
    US_TEST_CONDITION(1 == frameworkCtx->GetBundles().size(), "Test # of installed bundles")
    bundleEvents.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::UNINSTALLED, bundle));
    US_TEST_CONDITION(listener.CheckListenerEvents(bundleEvents), "Test for unexpected events");
    bundleEvents.clear();

    // Test install -> start -> uninstall
    // expect 6 events (INSTALLED, STARTING, STARTED, STOPPING, STOPPED, UNINSTALLED)
    bundle = InstallTestBundle(frameworkCtx, "TestBundleA");
    bundle->Start();
    bundle->Uninstall();
    US_TEST_CONDITION(0 == frameworkCtx->GetBundle("TestBundleA"), "Test bundle install -> start -> uninstall")
    US_TEST_CONDITION(1 == frameworkCtx->GetBundles().size(), "Test # of installed bundles")
    bundleEvents.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::STARTING, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::STARTED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::STOPPING, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::STOPPED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::UNINSTALLED, bundle));
    US_TEST_CONDITION(listener.CheckListenerEvents(bundleEvents), "Test for unexpected events");
    bundleEvents.clear();

    // Test install -> stop -> uninstall
    // expect 2 event (INSTALLED, UNINSTALLED)
    bundle = InstallTestBundle(frameworkCtx, "TestBundleA");
    bundle->Stop();
    bundle->Uninstall();
    US_TEST_CONDITION(0 == frameworkCtx->GetBundle("TestBundleA"), "Test bundle install -> stop -> uninstall")
    US_TEST_CONDITION(1 == frameworkCtx->GetBundles().size(), "Test # of installed bundles")
    bundleEvents.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::UNINSTALLED, bundle));
    US_TEST_CONDITION(listener.CheckListenerEvents(bundleEvents), "Test for unexpected events");
    bundleEvents.clear();

    // Test install -> start -> stop -> uninstall
    // expect 6 events (INSTALLED, STARTING, STARTED, STOPPING, STOPPED, UNINSTALLED)
    bundle = InstallTestBundle(frameworkCtx, "TestBundleA");
    bundle->Start();
    bundle->Stop();
    bundle->Uninstall();
    US_TEST_CONDITION(0 == frameworkCtx->GetBundle("TestBundleA"), "Test bundle install -> start -> stop -> uninstall")
    US_TEST_CONDITION(1 == frameworkCtx->GetBundles().size(), "Test # of installed bundles")
    bundleEvents.push_back(BundleEvent(BundleEvent::INSTALLED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::STARTING, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::STARTED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::STOPPING, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::STOPPED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::UNINSTALLED, bundle));
    US_TEST_CONDITION(listener.CheckListenerEvents(bundleEvents), "Test for unexpected events");
    bundleEvents.clear();

    framework->Stop();
    delete framework;
}

void TestForInstallFailure()
{
    FrameworkFactory factory;

    Framework* framework = factory.NewFramework(std::map<std::string, std::string>());
    framework->Start();

    BundleContext* frameworkCtx = framework->GetBundleContext();

    // Test that bogus bundle installs throw the appropriate exception
    try
    {
        frameworkCtx->InstallBundle(std::string());
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
        frameworkCtx->InstallBundle(std::string("\\path\\which\\won't\\exist\\phantom_bundle"));
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

    US_TEST_CONDITION(1 == frameworkCtx->GetBundles().size(), "Test # of installed bundles")

    framework->Stop();
    delete framework;
}

void TestDuplicateInstall()
{
    FrameworkFactory factory;

    Framework* framework = factory.NewFramework(std::map<std::string, std::string>());
    framework->Start();

    BundleContext* frameworkCtx = framework->GetBundleContext();

    // Test installing the same bundle (i.e. a bundle with the same location) twice.
    // The exact same bundle should be returned on the second install.
    Bundle* bundle = InstallTestBundle(frameworkCtx, "TestBundleA");
    Bundle* bundleDuplicate = InstallTestBundle(frameworkCtx, "TestBundleA");

    US_TEST_CONDITION(bundle == bundleDuplicate, "Test for the same bundle instance");
    US_TEST_CONDITION(bundle->GetBundleId() == bundleDuplicate->GetBundleId(), "Test for the same bundle id");

    framework->Stop();
    delete framework;
}

} // end unnamed namespace

int usBundleTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("BundleTest");

  FrameworkFactory factory;
  Framework* framework = factory.NewFramework(std::map<std::string, std::string>());
  framework->Start();

  std::vector<Bundle*> bundles = framework->GetBundleContext()->GetBundles();
  for (std::vector<Bundle*>::iterator iter = bundles.begin(), iterEnd = bundles.end();
       iter != iterEnd; ++iter)
  {
    std::cout << "----- " << (*iter)->GetName() << std::endl;
  }

  BundleContext* mc = framework->GetBundleContext();
  TestBundleListener listener;

  frame01(mc);
  frame02a(mc);

  try
  {
    mc->AddBundleListener(&listener, &TestBundleListener::BundleChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_OUTPUT( << "bundle listener registration failed " << ise.what() );
    throw;
  }

  try
  {
    mc->AddServiceListener(&listener, &TestBundleListener::ServiceChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_OUTPUT( << "service listener registration failed " << ise.what() );
    throw;
  }

  frame005a(mc->GetBundle("main")->GetBundleContext());
  frame010a(framework, mc);
  frame018a(mc);

  frame020a(framework, listener);
  frame030b(mc, listener);

  frame045a(mc);

  mc->RemoveBundleListener(&listener, &TestBundleListener::BundleChanged);
  mc->RemoveServiceListener(&listener, &TestBundleListener::ServiceChanged);

  framework->Stop();
  delete framework;

  // test a non-default framework instance using a different persistent storage location.
  std::map<std::string, std::string> frameworkConfig;
  frameworkConfig.insert(std::pair<std::string, std::string>("org.osgi.framework.storage", "/tmp"));
  framework = factory.NewFramework(frameworkConfig);
  framework->Start();

  frame02a(framework->GetBundleContext());
  frame02b(framework);

  delete framework;

  TestBundleStates();
  TestForInstallFailure();
  TestDuplicateInstall();

  US_TEST_END()
}
