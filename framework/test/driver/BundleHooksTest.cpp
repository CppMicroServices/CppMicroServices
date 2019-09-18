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
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/BundleEventHook.h"
#include "cppmicroservices/BundleFindHook.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/GetBundleContext.h"

#include "TestUtils.h"
#include "TestingConfig.h"
#include "TestingMacros.h"

// conflicts with FrameworkEvent::GetMessage
#undef GetMessage

using namespace cppmicroservices;

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

class TestFrameworkListener
{
public:
  void Event(const FrameworkEvent& frameworkEvent)
  {
    events.push_back(frameworkEvent);
  }

  std::vector<FrameworkEvent> events;
};

class TestBundleFindHook : public BundleFindHook
{
public:
  void Find(const BundleContext& /*context*/, ShrinkableVector<Bundle>& bundles)
  {
    for (auto i = bundles.begin(); i != bundles.end();) {
      if (i->GetSymbolicName() == "TestBundleA") {
        i = bundles.erase(i);
      } else {
        ++i;
      }
    }
  }
};

class TestBundleFindHookFailure : public BundleFindHook
{
public:
  void Find(const BundleContext&, ShrinkableVector<Bundle>&)
  {
    throw std::runtime_error("TestBundleFindHookFailure Event exception");
  }
};

class TestBundleEventHook : public BundleEventHook
{
public:
  void Event(const BundleEvent& event,
             ShrinkableVector<BundleContext>& contexts)
  {
    if (event.GetType() == BundleEvent::BUNDLE_STARTING ||
        event.GetType() == BundleEvent::BUNDLE_STOPPING) {
      contexts
        .clear(); //erase(std::remove(contexts.begin(), contexts.end(), GetBundleContext()), contexts.end());
    }
  }
};

class TestBundleEventHookFailure : public BundleEventHook
{
public:
  void Event(const BundleEvent&,
             ShrinkableVector<BundleContext>& )
  {
    throw std::runtime_error("TestBundleEventHookFailure Event exception");
  }
};

void TestFindHook(const Framework& framework)
{
  auto bundleA =
    testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
  US_TEST_CONDITION_REQUIRED(bundleA, "Test for existing bundle TestBundleA")

  US_TEST_CONDITION(bundleA.GetSymbolicName() == "TestBundleA",
                    "Test bundle name")

  bundleA.Start();

  US_TEST_CONDITION(bundleA.GetState() & Bundle::STATE_ACTIVE,
                    "Test if started correctly");

  long bundleAId = bundleA.GetBundleId();
  US_TEST_CONDITION_REQUIRED(bundleAId > 0, "Test for valid bundle id")

  US_TEST_CONDITION_REQUIRED(framework.GetBundleContext().GetBundle(bundleAId),
                             "Test for non-filtered GetBundle(long) result")

  auto findHookReg =
    framework.GetBundleContext().RegisterService<BundleFindHook>(
      std::make_shared<TestBundleFindHook>());

  US_TEST_CONDITION_REQUIRED(!framework.GetBundleContext().GetBundle(bundleAId),
                             "Test for filtered GetBundle(long) result")

  auto bundles = framework.GetBundleContext().GetBundles();
  for (auto const& i : bundles) {
    if (i.GetSymbolicName() == "TestBundleA") {
      US_TEST_FAILED_MSG(<< "TestBundleA not filtered from GetBundles()")
    }
  }

  findHookReg.Unregister();

  bundleA.Stop();
}

void TestEventHook(const Framework& framework)
{
  TestBundleListener bundleListener;
  framework.GetBundleContext().AddBundleListener(
    &bundleListener, &TestBundleListener::BundleChanged);

  auto bundleA =
    testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
  US_TEST_CONDITION_REQUIRED(bundleA, "Non-null bundle");

  bundleA.Start();
  US_TEST_CONDITION_REQUIRED(bundleListener.events.size() == 2,
                             "Test for received load bundle events");

  bundleA.Stop();
  US_TEST_CONDITION_REQUIRED(bundleListener.events.size() == 4,
                             "Test for received unload bundle events");

  auto eventHookReg =
    framework.GetBundleContext().RegisterService<BundleEventHook>(
      std::make_shared<TestBundleEventHook>());

  bundleListener.events.clear();

  bundleA.Start();
  US_TEST_CONDITION_REQUIRED(bundleListener.events.size() == 1,
                             "Test for filtered load bundle events");
  US_TEST_CONDITION_REQUIRED(bundleListener.events[0].GetType() ==
                               BundleEvent::BUNDLE_STARTED,
                             "Test for BUNDLE_STARTED event")

  bundleA.Stop();
  US_TEST_CONDITION_REQUIRED(bundleListener.events.size() == 2,
                             "Test for filtered unload bundle events");
  US_TEST_CONDITION_REQUIRED(bundleListener.events[1].GetType() ==
                               BundleEvent::BUNDLE_STOPPED,
                             "Test for BUNDLE_STOPPED event");

  eventHookReg.Unregister();
  framework.GetBundleContext().RemoveBundleListener(
    &bundleListener, &TestBundleListener::BundleChanged);
}

void TestEventHookFailure(const Framework& framework)
{
  auto bundleA =
    testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
  US_TEST_CONDITION_REQUIRED(bundleA, "Non-null bundle");

  auto eventHookReg =
    framework.GetBundleContext().RegisterService<BundleEventHook>(
      std::make_shared<TestBundleEventHookFailure>());

  TestFrameworkListener listener;
  auto fwkListenerToken = framework.GetBundleContext().AddFrameworkListener(
    std::bind(&TestFrameworkListener::Event, &listener, std::placeholders::_1));

  bundleA.Start();

  // bundle starting and bundle started events
  US_TEST_CONDITION_REQUIRED(listener.events.size() == 2,
                             "Test for expected number of framework events");

  std::for_each(
      listener.events.begin(),
      listener.events.end(),
      [](const FrameworkEvent& evt) {
        US_TEST_CONDITION_REQUIRED(evt.GetThrowable() != nullptr,
                                    "Test for the existence of an exception");
        US_TEST_CONDITION_REQUIRED(evt.GetType() ==
                                    FrameworkEvent::Type::FRAMEWORK_WARNING,
                                    "Test for the correct framework event type");
        std::string msg(evt.GetMessage());
        US_TEST_CONDITION_REQUIRED(
        std::string::npos != msg.find("Failed to call Bundle EventHook #"),
        "Test for the correct event message");
      });

  bundleA.Stop();
  eventHookReg.Unregister();
  framework.GetBundleContext().RemoveListener(std::move(fwkListenerToken));
}

void TestFindHookFailure(const Framework& framework)
{
  auto eventHookReg =
    framework.GetBundleContext().RegisterService<BundleFindHook>(
      std::make_shared<TestBundleFindHookFailure>());

  TestFrameworkListener listener;
  auto fwkListenerToken = framework.GetBundleContext().AddFrameworkListener(
    std::bind(&TestFrameworkListener::Event, &listener, std::placeholders::_1));

  auto bundleA =
    testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
  US_TEST_CONDITION_REQUIRED(bundleA, "Non-null bundle");
  bundleA.Start();

  // bundle starting and bundle started events
  US_TEST_CONDITION_REQUIRED(listener.events.size() == 1,
                             "Test for expected number of framework events");

  std::for_each(
    listener.events.begin(),
    listener.events.end(),
    [](const FrameworkEvent& evt) {
      US_TEST_CONDITION_REQUIRED(evt.GetThrowable() != nullptr,
                                 "Test for the existence of an exception");
      US_TEST_CONDITION_REQUIRED(evt.GetType() ==
                                   FrameworkEvent::Type::FRAMEWORK_WARNING,
                                 "Test for the correct framework event type");
      std::string msg(evt.GetMessage());
      US_TEST_CONDITION_REQUIRED(
        std::string::npos != msg.find("Failed to call Bundle FindHook  #"),
        "Test for the correct event message");
    });

  bundleA.Stop();
  eventHookReg.Unregister();
  framework.GetBundleContext().RemoveListener(std::move(fwkListenerToken));
}

} // end unnamed namespace

int BundleHooksTest(int /*argc*/, char* /*argv*/ [])
{
  US_TEST_BEGIN("BundleHooksTest");

  FrameworkFactory factory;
  auto framework = factory.NewFramework();
  framework.Start();

  TestFindHook(framework);
  TestEventHook(framework);

  TestEventHookFailure(framework);
  TestFindHookFailure(framework);

  framework.Stop();
  framework.WaitForStop(std::chrono::milliseconds::zero());

  US_TEST_END()
}
