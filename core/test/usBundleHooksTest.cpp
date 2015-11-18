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
#include <usBundleFindHook.h>
#include <usBundleEventHook.h>
#include <usBundleContext.h>
#include <usGetBundleContext.h>

#include "usTestUtils.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

using namespace us;

namespace {

class TestBundleListener
{
public:

  void BundleChanged(const BundleEvent& bundleEvent)
  {
    this->events.push_back(bundleEvent);
  }

  std::vector<BundleEvent> events;
};

class TestBundleFindHook : public BundleFindHook
{
public:

  void Find(const BundleContext* /*context*/, ShrinkableVector<std::shared_ptr<Bundle>>& bundles)
  {
    for (ShrinkableVector<std::shared_ptr<Bundle>>::iterator i = bundles.begin();
         i != bundles.end();)
    {
      if ((*i)->GetName() == "TestBundleA")
      {
        i = bundles.erase(i);
      }
      else
      {
        ++i;
      }
    }
  }
};

class TestBundleEventHook : public BundleEventHook
{
public:

  void Event(const BundleEvent& event, ShrinkableVector<BundleContext*>& contexts)
  {
    if (event.GetType() == BundleEvent::STARTING || event.GetType() == BundleEvent::STOPPING)
    {
      contexts.clear();//erase(std::remove(contexts.begin(), contexts.end(), GetBundleContext()), contexts.end());
    }
  }
};

void TestFindHook(const std::shared_ptr<Framework>& framework)
{
  InstallTestBundle(framework->GetBundleContext(), "TestBundleA");

  auto bundleA = framework->GetBundleContext()->GetBundle("TestBundleA");
  US_TEST_CONDITION_REQUIRED(bundleA != nullptr, "Test for existing bundle TestBundleA")

  US_TEST_CONDITION(bundleA->GetName() == "TestBundleA", "Test bundle name")

  bundleA->Start();

  US_TEST_CONDITION(bundleA->IsStarted() == true, "Test if started correctly");

  long bundleAId = bundleA->GetBundleId();
  US_TEST_CONDITION_REQUIRED(bundleAId > 0, "Test for valid bundle id")

  US_TEST_CONDITION_REQUIRED(framework->GetBundleContext()->GetBundle(bundleAId) != nullptr, "Test for non-filtered GetBundle(long) result")

  ServiceRegistration<BundleFindHook> findHookReg = framework->GetBundleContext()->RegisterService<BundleFindHook>(std::make_shared<TestBundleFindHook>());

  US_TEST_CONDITION_REQUIRED(framework->GetBundleContext()->GetBundle(bundleAId) == nullptr, "Test for filtered GetBundle(long) result")

  auto bundles = framework->GetBundleContext()->GetBundles();
  for (auto const& i : bundles)
  {
    if (i->GetName() == "TestBundleA")
    {
      US_TEST_FAILED_MSG(<< "TestBundleA not filtered from GetBundles()")
    }
  }

  findHookReg.Unregister();

  bundleA->Stop();
}

void TestEventHook(std::shared_ptr<Framework> framework)
{
  TestBundleListener bundleListener;
  framework->GetBundleContext()->AddBundleListener(&bundleListener, &TestBundleListener::BundleChanged);

  InstallTestBundle(framework->GetBundleContext(), "TestBundleA");

  auto bundleA = framework->GetBundleContext()->GetBundle("TestBundleA");
  bundleA->Start();
  US_TEST_CONDITION_REQUIRED(bundleListener.events.size() == 3, "Test for received load bundle events")

  bundleA->Stop();
  US_TEST_CONDITION_REQUIRED(bundleListener.events.size() == 5, "Test for received unload bundle events")

  ServiceRegistration<BundleEventHook> eventHookReg = framework->GetBundleContext()->RegisterService<BundleEventHook>(std::make_shared<TestBundleEventHook>());

  bundleListener.events.clear();

  bundleA->Start();
  US_TEST_CONDITION_REQUIRED(bundleListener.events.size() == 1, "Test for filtered load bundle events")
  US_TEST_CONDITION_REQUIRED(bundleListener.events[0].GetType() == BundleEvent::STARTED, "Test for STARTED event")

  bundleA->Stop();
  US_TEST_CONDITION_REQUIRED(bundleListener.events.size() == 2, "Test for filtered unload bundle events")
  US_TEST_CONDITION_REQUIRED(bundleListener.events[1].GetType() == BundleEvent::STOPPED, "Test for STOPPED event")

  eventHookReg.Unregister();
  framework->GetBundleContext()->RemoveBundleListener(&bundleListener, &TestBundleListener::BundleChanged);
}

} // end unnamed namespace

int usBundleHooksTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("BundleHooksTest");

  FrameworkFactory factory;
  std::shared_ptr<Framework> framework = factory.NewFramework(std::map<std::string, std::string>());
  framework->Start();

  TestFindHook(framework);
  TestEventHook(framework);

  US_TEST_END()
}
