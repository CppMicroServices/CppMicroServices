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
#include <usBundleContext.h>
#include <usBundleEvent.h>
#include <usGetBundleContext.h>
#include <usBundle.h>
#include <usBundleSettings.h>

#include <usTestingConfig.h>
#include "usTestUtils.h"
#include "usTestUtilBundleListener.h"
#include "usTestingMacros.h"

#include <cassert>

using namespace us;

namespace {

void testDefaultAutoLoadPath(bool autoLoadEnabled, Framework* framework)
{
  BundleContext* mc = framework->GetBundleContext();
  assert(mc);
  TestBundleListener listener;

  BundleListenerRegistrationHelper<TestBundleListener> listenerReg(mc, &listener, &TestBundleListener::BundleChanged);

  InstallTestBundle(mc, "TestBundleAL");

  Bundle* bundleAL = mc->GetBundle("TestBundleAL");
  US_TEST_CONDITION_REQUIRED(bundleAL != NULL, "Test for existing bundle TestBundleAL")

  US_TEST_CONDITION(bundleAL->GetName() == "TestBundleAL", "Test bundle name")

  bundleAL->Start();

  Any installedBundles = bundleAL->GetProperty(Bundle::PROP_AUTOINSTALLED_BUNDLES());
  Bundle* bundleAL_1 = mc->GetBundle("TestBundleAL_1");

  // check the listeners for events
  std::vector<BundleEvent> pEvts;
  if (autoLoadEnabled)
  {
      pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundleAL_1));
  }
  pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundleAL));
  pEvts.push_back(BundleEvent(BundleEvent::STARTING, bundleAL));
  pEvts.push_back(BundleEvent(BundleEvent::STARTED, bundleAL));

  if (autoLoadEnabled)
  {
    US_TEST_CONDITION_REQUIRED(bundleAL_1 != NULL, "Test for existing auto-installed bundle TestBundleAL_1")
    US_TEST_CONDITION(bundleAL_1->GetName() == "TestBundleAL_1", "Test bundle name")
    US_TEST_CONDITION_REQUIRED(!installedBundles.Empty(), "Test for PROP_AUTOINSTALLED_BUNDLES property")
    US_TEST_CONDITION_REQUIRED(installedBundles.Type() == typeid(std::vector<std::string>), "Test for PROP_AUTOINSTALLED_BUNDLES property type")
    std::vector<std::string> installedBundlesVec = any_cast<std::vector<std::string> >(installedBundles);
    US_TEST_CONDITION_REQUIRED(installedBundlesVec.size() == 1, "Test for PROP_AUTOINSTALLED_BUNDLES vector size")
    US_TEST_CONDITION_REQUIRED(installedBundlesVec[0] == bundleAL_1->GetName(), "Test for PROP_AUTOINSTALLED_BUNDLES vector content")

    bundleAL_1->Start();

    pEvts.push_back(BundleEvent(BundleEvent::STARTING, bundleAL_1));
    pEvts.push_back(BundleEvent(BundleEvent::STARTED, bundleAL_1));

  }
  else
  {
    US_TEST_CONDITION_REQUIRED(bundleAL_1 == NULL, "Test for non-existing auto-installed bundle TestBundleAL_1")
    US_TEST_CONDITION_REQUIRED(installedBundles.Empty(), "Test for empty PROP_AUTOINSTALLED_BUNDLES property")
  }

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Test for unexpected events");

  mc->RemoveBundleListener(&listener, &TestBundleListener::BundleChanged);

  bundleAL->Stop();
}

void testCustomAutoLoadPath(Framework* framework)
{
  BundleContext* mc = framework->GetBundleContext();
  assert(mc);
  TestBundleListener listener;

  try
  {
    mc->AddBundleListener(&listener, &TestBundleListener::BundleChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_OUTPUT( << "bundle listener registration failed " << ise.what() );
    throw;
  }

  InstallTestBundle(mc, "TestBundleAL2");

  Bundle* bundleAL2 = mc->GetBundle("TestBundleAL2");
  US_TEST_CONDITION_REQUIRED(bundleAL2 != NULL, "Test for existing bundle TestBundleAL2")

  US_TEST_CONDITION(bundleAL2->GetName() == "TestBundleAL2", "Test bundle name")

  bundleAL2->Start();

  Bundle* bundleAL2_1 = mc->GetBundle("TestBundleAL2_1");

  // check the listeners for events
  std::vector<BundleEvent> pEvts;
#ifdef US_ENABLE_AUTOLOADING_SUPPORT
  pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundleAL2_1));
#endif
  pEvts.push_back(BundleEvent(BundleEvent::INSTALLED, bundleAL2));
  pEvts.push_back(BundleEvent(BundleEvent::STARTING, bundleAL2));
  pEvts.push_back(BundleEvent(BundleEvent::STARTED, bundleAL2));

#ifdef US_ENABLE_AUTOLOADING_SUPPORT
  US_TEST_CONDITION_REQUIRED(bundleAL2_1 != NULL, "Test for existing auto-installed bundle TestBundleAL2_1")
  US_TEST_CONDITION(bundleAL2_1->GetName() == "TestBundleAL2_1", "Test bundle name")

  bundleAL2_1->Start();
  pEvts.push_back(BundleEvent(BundleEvent::STARTING, bundleAL2_1));
  pEvts.push_back(BundleEvent(BundleEvent::STARTED, bundleAL2_1));

#else
  US_TEST_CONDITION_REQUIRED(bundleAL2_1 == NULL, "Test for non-existing auto-installed bundle TestBundleAL2_1")
#endif

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Test for unexpected events");

  mc->RemoveBundleListener(&listener, &TestBundleListener::BundleChanged);

  bundleAL2->Stop();
}

} // end unnamed namespace


int usBundleAutoLoadTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("BundleLoaderTest");

  FrameworkFactory factory;
  Framework* framework = factory.NewFramework(std::map<std::string, std::string>());
  framework->Start();

  framework->SetAutoLoadingEnabled(false);
  testDefaultAutoLoadPath(false, framework);

  framework->Stop();
  delete framework;

  framework = factory.NewFramework(std::map<std::string, std::string>());
  framework->Start();

  framework->SetAutoLoadingEnabled(true);

  testDefaultAutoLoadPath(true, framework);
  testCustomAutoLoadPath(framework);

  framework->Stop();
  delete framework;

  US_TEST_END()
}
