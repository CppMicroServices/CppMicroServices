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

#include <future>

#include <usFrameworkFactory.h>
#include <usFramework.h>
#include <usServiceTracker.h>
#include <usBundleEvent.h>

#include "usTestUtils.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

/*
 * This test is meant to be run with a thread sanity checker. E.g. thread
 * sanitizer (using Clang or GCC) or helgrind.
 */

using namespace us;

void bundleListener(const BundleEvent& be)
{
  auto b = be.GetBundle();
  auto type = be.GetType();
  if (type == BundleEvent::BUNDLE_STARTING || type == BundleEvent::BUNDLE_STARTED)
  {
    b.Stop();
  }
  else if (type == BundleEvent::BUNDLE_STOPPING || type == BundleEvent::BUNDLE_STOPPED)
  {
    b.Start();
  }
}

int usConcurrencyTest(int /*argc*/, char* /*argv*/[])
{
    US_TEST_BEGIN("ConcurrencyTest");

    FrameworkFactory factory;

    auto f = factory.NewFramework();
    f.Start();

    auto context = f.GetBundleContext();

    context.AddBundleListener(bundleListener);

    ServiceTracker<void> tracker(f.GetBundleContext(), "org.cppmicroservices.c1.additional");
    tracker.Open();

    auto bundle = testing::InstallLib(context, "TestBundleC1");

    /* --- The Bundle class is not thread safe yet with respect to state changes ---

    // concurrently start and stop the bundle multiple times
    std::vector<std::future<void>> fs;
    for (std::size_t i = 0; i < 10; ++i)
    {
      fs.emplace_back(std::async(std::launch::async, [&bundle] { bundle->Stop(); }));
      fs.emplace_back(std::async(std::launch::async, [&bundle] { bundle->Start(); }));
    }
    for (auto& f : fs)
    {
      f.get();
    }

    */

    // make sure the bundle really is started
    context.RemoveBundleListener(bundleListener);
    bundle.Start();

    tracker.WaitForService();
    auto im = tracker.GetService();
    US_TEST_CONDITION_REQUIRED(*std::static_pointer_cast<int>(ExtractInterface(im, "org.cppmicroservices.c1.additional")) == 2, "Wait for service")

    bundle.Stop();

    US_TEST_END()
}
