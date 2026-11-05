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
#include <cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp>

#include "../TestUtils.hpp"
#include "TestInterfaces/Interfaces.hpp"

#include <atomic>
#include <chrono>
#include <future>
#include <mutex>
#include <string>

namespace scr = cppmicroservices::service::component::runtime;

namespace
{
    auto const kWaitTimeout = std::chrono::seconds(10);
    // How long SCRActivator::Stop is held to give a stopping bundle the chance to finish
    // first. A correct SCR blocks that bundle until its components are disposed, so this
    // only ever runs out when SCR is behaving correctly.
    auto const kHoldTimeout = std::chrono::milliseconds(500);

    /**
     * Shared state used to line up the thread stopping a DS bundle (B) and the
     * thread stopping the SCR bundle (A).
     */
    struct Sync
    {
        cppmicroservices::Bundle bundleToStop;

        // B is inside the BundleEventHook, i.e. the framework has already
        // snapshotted the bundle listeners (including SCR's) for this event.
        std::promise<void> hookReached;
        std::once_flag hookReachedOnce;

        // A is inside SCRActivator::Stop, past the point where it stops creating
        // extensions, and has not yet disposed any.
        std::promise<void> scrStopping;
        std::once_flag scrStoppingOnce;

        // B has finished stopping bundleToStop.
        std::promise<void> bundleStopped;
        std::once_flag bundleStoppedOnce;

        std::atomic<bool> hookTimedOut { false };

        void
        ReleaseHook()
        {
            std::call_once(scrStoppingOnce, [this] { scrStopping.set_value(); });
        }
        void
        ReleaseScr()
        {
            std::call_once(bundleStoppedOnce, [this] { bundleStopped.set_value(); });
        }
    };

    /**
     * Holds the BUNDLE_STOPPING event for bundleToStop until SCRActivator::Stop
     * is running. The hook runs after the framework has copied the bundle listener
     * map, so SCR's bundle listener will still be invoked for this event even
     * though SCRActivator::Stop removes it.
     */
    class BlockingBundleEventHook : public cppmicroservices::BundleEventHook
    {
      public:
        BlockingBundleEventHook(std::shared_ptr<Sync> s, std::shared_future<void> stopping)
            : sync(std::move(s))
            , scrStopping(std::move(stopping))
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
            if (scrStopping.wait_for(kWaitTimeout) != std::future_status::ready)
            {
                sync->hookTimedOut = true;
            }
        }

      private:
        std::shared_ptr<Sync> sync;
        std::shared_future<void> scrStopping;
    };
} // namespace

/**
 * A bundle which stops while SCR is stopping must not finish stopping before SCR
 * has disposed its components.
 *
 * Thread B stops TestBundleDSStopOrder, which has an immediate component. Its
 * BUNDLE_STOPPING event reaches SCR's bundle listener only once SCRActivator::Stop
 * (thread A) is running. Thread A is then held until thread B has finished stopping
 * the bundle (or a short timeout, since a correct SCR keeps B waiting until A has
 * disposed the bundle's components). The bundle's activator Stop() runs right after
 * BUNDLE_STOPPING and throws if the component's service is still registered, which
 * makes the bundle's Stop() throw.
 */
// Not using a fixture: this needs a fresh framework where only DS and the one test
// bundle are running, so that nothing else is stopped while the events are held.
TEST(SCRShutdownOrderingTest, testBundleStoppingDuringSCRStopIsDisposedBeforeItStops)
{
    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();
    auto context = framework.GetBundleContext();

    test::InstallAndStartDS(context);
    auto scrRef = context.GetServiceReference<scr::ServiceComponentRuntime>();
    ASSERT_TRUE(scrRef);
    auto scrBundle = scrRef.GetBundle();

    auto sync = std::make_shared<Sync>();

    // If an assertion fails, don't leave threads blocked or the framework running
    // into later tests.
    std::future<void> stopBundle;
    struct Cleanup
    {
        std::shared_ptr<Sync> sync;
        cppmicroservices::Framework& framework;
        std::future<void>& stopBundle;
        ~Cleanup()
        {
            sync->ReleaseHook();
            sync->ReleaseScr();
            if (stopBundle.valid())
            {
                stopBundle.wait_for(kWaitTimeout);
            }
            framework.Stop();
            framework.WaitForStop(kWaitTimeout);
        }
    } cleanup { sync, framework, stopBundle };

    sync->bundleToStop = test::InstallAndStartBundle(context, "TestBundleDSStopOrder");
    ASSERT_TRUE(sync->bundleToStop);
    ASSERT_TRUE(context.GetServiceReference<test::Interface1>()) << "component service was not registered";

    context.AddBundleListener(
        [sync](cppmicroservices::BundleEvent const& evt)
        {
            if (evt.GetType() == cppmicroservices::BundleEvent::BUNDLE_STOPPED && evt.GetBundle() == sync->bundleToStop)
            {
                sync->ReleaseScr();
            }
        });

    // SCRActivator::Stop unregisters the ServiceComponentRuntime service after it
    // stops creating extensions and before it disposes any. Release B there, then
    // hold A until B has finished stopping the bundle, or for kHoldTimeout.
    std::shared_future<void> scrStopping = sync->scrStopping.get_future().share();
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
        "(" + cppmicroservices::Constants::OBJECTCLASS + "=" + us_service_interface_iid<scr::ServiceComponentRuntime>()
            + ")");

    context.RegisterService<cppmicroservices::BundleEventHook>(
        std::make_shared<BlockingBundleEventHook>(sync, scrStopping));

    stopBundle = std::async(std::launch::async, [sync] { sync->bundleToStop.Stop(); });

    ASSERT_EQ(sync->hookReached.get_future().wait_for(kWaitTimeout), std::future_status::ready)
        << "BUNDLE_STOPPING was never delivered";

    scrBundle.Stop();

    ASSERT_EQ(stopBundle.wait_for(kWaitTimeout), std::future_status::ready) << "Bundle stop never finished";
    EXPECT_FALSE(sync->hookTimedOut) << "SCRActivator::Stop never ran while the bundle event was held";
    EXPECT_NO_THROW(stopBundle.get()) << "SCR had not disposed the bundle's component when its activator stopped";
}
