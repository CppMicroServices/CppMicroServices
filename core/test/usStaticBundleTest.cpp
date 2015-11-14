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

#include "usTestUtils.h"
#include "usTestUtilBundleListener.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

using namespace us;

namespace {

// Install and start libTestBundleB and check that it exists and that the service it registers exists,
// also check that the expected events occur
void frame020a(BundleContext* mc, TestBundleListener& listener)
{
  InstallTestBundle(mc, "TestBundleB");

  Bundle* bundleB = mc->GetBundle("TestBundleB");
  US_TEST_CONDITION_REQUIRED(bundleB != nullptr, "Test for existing bundle TestBundleB")

  try
  {
#if defined (US_BUILD_SHARED_LIBS)
    Bundle* bundle = mc->InstallBundle(LIB_PATH + DIR_SEP + LIB_PREFIX + "TestBundleB" + LIB_EXT + "/TestBundleImportedByB");
#else
    Bundle* bundle = mc->InstallBundle(BIN_PATH + DIR_SEP + "usCoreTestDriver" + EXE_EXT + "/TestBundleImportedByB");
#endif
    US_TEST_CONDITION_REQUIRED(bundle != NULL, "Test installation of bundle TestBundleImportedByB")
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Install bundle exception: " << e.what() << " + in frame020a:FAIL")
  }

  Bundle* bundleImportedByB = mc->GetBundle("TestBundleImportedByB");
  US_TEST_CONDITION_REQUIRED(bundleImportedByB != nullptr, "Test for existing bundle TestBundleImportedByB")

  US_TEST_CONDITION(bundleB->GetName() == "TestBundleB", "Test bundle name")
  US_TEST_CONDITION(bundleImportedByB->GetName() == "TestBundleImportedByB", "Test bundle name")

  bundleB->Start();
  bundleImportedByB->Start();
  // Check if libB registered the expected service
  try
  {
    std::vector<ServiceReferenceU> refs = mc->GetServiceReferences("us::TestBundleBService");
    US_TEST_CONDITION_REQUIRED(refs.size() == 2, "Test that both the service from the shared and imported library are regsitered");

    InterfaceMapConstPtr o1 = mc->GetService(refs.front());
    US_TEST_CONDITION(o1 && !o1->empty(), "Test if first service object found");

    InterfaceMapConstPtr o2 = mc->GetService(refs.back());
    US_TEST_CONDITION(o1 && !o2->empty(), "Test if second service object found");

    // check the listeners for events
    std::vector<BundleEvent> pEvts;
    pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundleB));
    pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundleImportedByB));
    pEvts.push_back(BundleEvent(BundleEvent::STARTING, bundleB));
    pEvts.push_back(BundleEvent(BundleEvent::STARTED, bundleB));
    pEvts.push_back(BundleEvent(BundleEvent::STARTING, bundleImportedByB));
    pEvts.push_back(BundleEvent(BundleEvent::STARTED, bundleImportedByB));

    std::vector<ServiceEvent> seEvts;
    seEvts.push_back(ServiceEvent(ServiceEvent::REGISTERED, refs.back()));
    seEvts.push_back(ServiceEvent(ServiceEvent::REGISTERED, refs.front()));

    US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");

  }
  catch (const ServiceException& /*se*/)
  {
    US_TEST_FAILED_MSG(<< "test bundle, expected service not found");
  }

  US_TEST_CONDITION(bundleB->IsStarted() == true, "Test if started correctly");
}


// Stop libB and check for correct events
void frame030b(BundleContext* mc, TestBundleListener& listener)
{
  Bundle* bundleB = mc->GetBundle("TestBundleB");
  US_TEST_CONDITION_REQUIRED(bundleB != nullptr, "Test for non-null bundle")

  Bundle* bundleImportedByB = mc->GetBundle("TestBundleImportedByB");
  US_TEST_CONDITION_REQUIRED(bundleImportedByB != nullptr, "Test for non-null bundle")

  std::vector<ServiceReferenceU> refs
      = mc->GetServiceReferences("us::TestBundleBService");
  US_TEST_CONDITION(refs.front(), "Test for first valid service reference")
  US_TEST_CONDITION(refs.back(), "Test for second valid service reference")

  try
  {
    bundleB->Stop();
    US_TEST_CONDITION(bundleB->IsStarted() == false, "Test for stopped state")
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Stop bundle exception: " << e.what())
  }

  try
  {
    bundleImportedByB->Stop();
    US_TEST_CONDITION(bundleImportedByB->IsStarted() == false, "Test for stopped state")
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Stop bundle exception: " << e.what())
  }

  std::vector<BundleEvent> pEvts;
  pEvts.push_back(BundleEvent(BundleEvent::STOPPING, bundleB));
  pEvts.push_back(BundleEvent(BundleEvent::STOPPED, bundleB));
  pEvts.push_back(BundleEvent(BundleEvent::STOPPING, bundleImportedByB));
  pEvts.push_back(BundleEvent(BundleEvent::STOPPED, bundleImportedByB));

  std::vector<ServiceEvent> seEvts;
  seEvts.push_back(ServiceEvent(ServiceEvent::UNREGISTERING, refs.back()));
  seEvts.push_back(ServiceEvent(ServiceEvent::UNREGISTERING, refs.front()));

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");
}

// Uninstall libB and check for correct events
void frame040c(BundleContext* mc, TestBundleListener& listener)
{
    Bundle* bundleB = mc->GetBundle("TestBundleB");
    US_TEST_CONDITION_REQUIRED(bundleB != nullptr, "Test for non-null bundle")

    Bundle* bundleImportedByB = mc->GetBundle("TestBundleImportedByB");
    US_TEST_CONDITION_REQUIRED(bundleImportedByB != nullptr, "Test for non-null bundle")

    bundleB->Uninstall();
    US_TEST_CONDITION(mc->GetBundles().size() == 2, "Test for uninstall of TestBundleB")
    bundleImportedByB->Uninstall();
    US_TEST_CONDITION(mc->GetBundles().size() == 1, "Test for uninstall of TestBundleImportedByB")

    std::vector<BundleEvent> pEvts;
    pEvts.push_back(BundleEvent(BundleEvent::UNINSTALLED, bundleB));
    pEvts.push_back(BundleEvent(BundleEvent::UNINSTALLED, bundleImportedByB));

    US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Test for unexpected events");
}

} // end unnamed namespace

int usStaticBundleTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("StaticBundleTest");

  FrameworkFactory factory;
  std::shared_ptr<Framework> framework = factory.NewFramework(std::map<std::string, std::string>());
  framework->Start();

  BundleContext* mc = framework->GetBundleContext();

  { // scope the use of the listener so its destructor is
    // called before we destroy the framework's bundle context.
    // The TestBundleListener needs to remove its listeners while
    // the framework is still active.
    TestBundleListener listener;

    BundleListenerRegistrationHelper<TestBundleListener> ml(mc, &listener, &TestBundleListener::BundleChanged);
    ServiceListenerRegistrationHelper<TestBundleListener> sl(mc, &listener, &TestBundleListener::ServiceChanged);

    frame020a(mc, listener);
    frame030b(mc, listener);
    frame040c(mc, listener);
  }

  US_TEST_END()
}
