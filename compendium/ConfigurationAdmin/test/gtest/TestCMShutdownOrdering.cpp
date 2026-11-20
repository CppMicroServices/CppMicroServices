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


#include "gtest/gtest.h"

#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/BundleEventHook.h>
#include <cppmicroservices/Constants.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/ServiceEvent.h>
#include <cppmicroservices/ShrinkableVector.h>
#include <cppmicroservices/cm/ConfigurationAdmin.hpp>

#include "../TestUtils.hpp"

#include <atomic>
#include <chrono>
#include <future>
#include <mutex>
#include <string>

namespace cm = cppmicroservices::service::cm;

namespace
{
    auto const kWaitTimeout = std::chrono::seconds(10);
    // How long CMActivator::Stop is held to give a stopping bundle the chance to finish
    // first. A correct CA blocks that bundle until its configurations are removed, so this
    // only ever runs out when CA is behaving correctly.
    auto const kHoldTimeout = std::chrono::milliseconds(500);

    // One of the configurations in ManagedServiceAndFactoryBundle's cm metadata.
    std::string const BUNDLE_PID_FILTER { "(pid=cm.testservice)" };

    /**
     * Shared state used to line up the thread stopping a bundle with cm metadata (B)
     * and the thread stopping CA (A).
     */
    struct Sync
    {
        cppmicroservices::Bundle bundleToStop;

        // B is inside the BundleEventHook, i.e. the framework has already
        // snapshotted the bundle listeners (including CA's) for this event.
        std::promise<void> hookReached;
        std::once_flag hookReachedOnce;

        // A is inside CMActivator::Stop holding WriteLock(*notificationLock), and has
        // not yet removed any bundle's configurations.
        std::promise<void> caStopping;
        std::once_flag caStoppingOnce;

        // B has finished stopping bundleToStop.
        std::promise<void> bundleStopped;
        std::once_flag bundleStoppedOnce;

        // Whether bundleToStop's configurations were still loaded when it reached BUNDLE_STOPPED.
        std::atomic<bool> stoppedChecked { false };
        std::atomic<bool> configurationsLeftWhenStopped { false };

        std::atomic<bool> hookTimedOut { false };

        void
        ReleaseHook()
        {
            std::call_once(caStoppingOnce, [this] { caStopping.set_value(); });
        }
        void
        ReleaseCa()
        {
            std::call_once(bundleStoppedOnce, [this] { bundleStopped.set_value(); });
        }
    };

    /**
     * Holds the BUNDLE_STOPPING event for bundleToStop until CMActivator::Stop is
     * running. The hook runs after the framework has copied the bundle listener map,
     * so CA's bundle listener will still be invoked for this event even though
     * CMActivator::Stop removes it.
     */
    class BlockingBundleEventHook : public cppmicroservices::BundleEventHook
    {
      public:
        BlockingBundleEventHook(std::shared_ptr<Sync> s, std::shared_future<void> stopping)
            : sync(std::move(s))
            , caStopping(std::move(stopping))
        {
        }

        void
        Event(cppmicroservices::BundleEvent const& evt,
              cppmicroservices::ShrinkableVector<cppmicroservices::BundleContext>&) override
        {
            if (evt.GetType() != cppmicroservices::BundleEvent::BUNDLE_STOPPING
                || evt.GetBundle() != sync->bundleToStop)
            {
                return;
            }
            std::call_once(sync->hookReachedOnce, [this] { sync->hookReached.set_value(); });
            if (caStopping.wait_for(kWaitTimeout) != std::future_status::ready)
            {
                sync->hookTimedOut = true;
            }
        }

      private:
        std::shared_ptr<Sync> sync;
        std::shared_future<void> caStopping;
    };
} // namespace

/**
 * A bundle with cm metadata which stops while CA is stopping must not finish stopping
 * before CA has removed its configurations.
 *
 * Thread B stops ManagedServiceAndFactoryBundle. Its BUNDLE_STOPPING event reaches CA's
 * bundle listener only once CMActivator::Stop (thread A) is running. Thread A is then
 * held until thread B has finished stopping the bundle (or a short timeout, since a
 * correct CA keeps B waiting until A has removed the bundle's configurations). When the
 * bundle reaches BUNDLE_STOPPED, its configurations must already be gone.
 */
// Not using a fixture: this needs a fresh framework where only CA and the one test
// bundle are running, so that nothing else is stopped while the events are held.
TEST(CMShutdownOrderingTest, testBundleStoppingDuringCAStopHasConfigurationsRemovedBeforeItStops)
{
    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();
    auto context = framework.GetBundleContext();

    test::InstallAndStartConfigAdmin(context);
    auto caRef = context.GetServiceReference<cm::ConfigurationAdmin>();
    ASSERT_TRUE(caRef);
    auto caBundle = caRef.GetBundle();
    auto configAdmin = context.GetService<cm::ConfigurationAdmin>(caRef);

    auto sync = std::make_shared<Sync>();

    // Declared before Cleanup so that its destructor, which waits for the bundle stop to
    // finish, runs after Cleanup has released the threads held by the test.
    std::future<void> stopBundle;
    // If an assertion fails, release the threads held by the test and stop the framework,
    // so later tests aren't affected. This can't recover from a real deadlock:
    // Framework::WaitForStop and the Framework destructor both join the stuck shutdown
    // thread, so the test then hangs until the ctest timeout.
    struct Cleanup
    {
        std::shared_ptr<Sync> sync;
        cppmicroservices::Framework& framework;
        ~Cleanup()
        {
            sync->ReleaseHook();
            sync->ReleaseCa();
            framework.Stop();
            framework.WaitForStop(kWaitTimeout);
        }
    } cleanup { sync, framework };

    sync->bundleToStop = test::InstallAndStartBundle(context, "ManagedServiceAndFactoryBundle");
    ASSERT_TRUE(sync->bundleToStop);
    ASSERT_FALSE(configAdmin->ListConfigurations(BUNDLE_PID_FILTER).empty())
        << "configurations from the bundle's cm metadata were not loaded";

    // Check the bundle's configurations before releasing A, so that A cannot remove
    // them in between.
    context.AddBundleListener(
        [sync, configAdmin](cppmicroservices::BundleEvent const& evt)
        {
            if (evt.GetType() == cppmicroservices::BundleEvent::BUNDLE_STOPPED && evt.GetBundle() == sync->bundleToStop)
            {
                sync->configurationsLeftWhenStopped = !configAdmin->ListConfigurations(BUNDLE_PID_FILTER).empty();
                sync->stoppedChecked = true;
                sync->ReleaseCa();
            }
        });

    // CMActivator::Stop unregisters the ConfigurationAdmin service while holding the
    // write lock and before it removes any configurations. Release B there, then hold A
    // until B has finished stopping the bundle, or for kHoldTimeout.
    std::shared_future<void> caStopping = sync->caStopping.get_future().share();
    std::shared_future<void> bundleStopped = sync->bundleStopped.get_future().share();
    context.AddServiceListener(
        [sync, bundleStopped](cppmicroservices::ServiceEvent const& evt)
        {
            if (evt.GetType() != cppmicroservices::ServiceEvent::SERVICE_UNREGISTERING)
            {
                return;
            }
            sync->ReleaseHook();
            bundleStopped.wait_for(kHoldTimeout);
        },
        "(" + cppmicroservices::Constants::OBJECTCLASS + "=" + us_service_interface_iid<cm::ConfigurationAdmin>()
            + ")");

    context.RegisterService<cppmicroservices::BundleEventHook>(
        std::make_shared<BlockingBundleEventHook>(sync, caStopping));

    stopBundle = std::async(std::launch::async, [sync] { sync->bundleToStop.Stop(); });

    ASSERT_EQ(sync->hookReached.get_future().wait_for(kWaitTimeout), std::future_status::ready)
        << "BUNDLE_STOPPING was never delivered";

    caBundle.Stop();

    ASSERT_EQ(stopBundle.wait_for(kWaitTimeout), std::future_status::ready) << "Bundle stop never finished";
    EXPECT_FALSE(sync->hookTimedOut) << "CMActivator::Stop never ran while the bundle event was held";
    ASSERT_TRUE(sync->stoppedChecked) << "BUNDLE_STOPPED was never delivered";
    EXPECT_FALSE(sync->configurationsLeftWhenStopped)
        << "Bundle finished stopping before CA removed its configurations";
}
