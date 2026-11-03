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
#include <cppmicroservices/cm/ManagedServiceFactory.hpp>

#include "../TestUtils.hpp"

#include <chrono>
#include <future>
#include <mutex>
#include <string>
#include <thread>

namespace cm = cppmicroservices::service::cm;

namespace
{
    auto const WAIT_TIMEOUT = std::chrono::seconds(10);

    std::string const FACTORY_PID { "test.shutdowndeadlock.factory" };

    /**
     * Shared state used to line up the CA worker thread (B) and the framework
     * shutdown thread (A).
     */
    struct Sync
    {
        cppmicroservices::Bundle bundleToStop;

        std::promise<void> updated;
        std::once_flag updatedOnce;

        std::mutex removedThreadMutex;
        std::thread::id removedThread;

        // B is inside the BundleEventHook, i.e. the framework has already
        // snapshotted the bundle listeners (including CA's) for this event.
        std::promise<void> hookReached;
        std::once_flag hookReachedOnce;

        // A is inside CMActivator::Stop holding WriteLock(*notificationLock).
        std::promise<void> caUnregistering;
        std::once_flag caUnregisteringOnce;
    };

    /**
     * A ManagedServiceFactory which stops a bundle from inside Removed().
     */
    class BundleStoppingFactory : public cm::ManagedServiceFactory
    {
      public:
        explicit BundleStoppingFactory(std::shared_ptr<Sync> s) : sync(std::move(s)) {}

        void
        Updated(std::string const&, cppmicroservices::AnyMap const&) override
        {
            std::call_once(sync->updatedOnce, [this] { sync->updated.set_value(); });
        }

        void
        Removed(std::string const&) override
        {
            {
                std::lock_guard<std::mutex> l(sync->removedThreadMutex);
                sync->removedThread = std::this_thread::get_id();
            }
            try
            {
                sync->bundleToStop.Stop();
            }
            catch (...)
            {
            }
        }

      private:
        std::shared_ptr<Sync> sync;
    };

    /**
     * Holds the BUNDLE_STOPPING event fired from Removed() until CMActivator::Stop
     * holds the notification write lock. The hook runs after the framework has
     * copied the bundle listener map, so CA's bundle listener will still be invoked
     * for this event even though CMActivator::Stop removes it.
     */
    class BlockingBundleEventHook : public cppmicroservices::BundleEventHook
    {
      public:
        BlockingBundleEventHook(std::shared_ptr<Sync> s, std::shared_future<void> unregistering)
            : sync(std::move(s))
            , caUnregistering(std::move(unregistering))
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
            {
                std::lock_guard<std::mutex> l(sync->removedThreadMutex);
                if (sync->removedThread != std::this_thread::get_id())
                {
                    return;
                }
            }
            std::call_once(sync->hookReachedOnce, [this] { sync->hookReached.set_value(); });
            caUnregistering.wait_for(WAIT_TIMEOUT);
        }

      private:
        std::shared_ptr<Sync> sync;
        std::shared_future<void> caUnregistering;
    };
} // namespace

/**
 * Framework shutdown must not deadlock when a ManagedServiceFactory::Removed callback,
 * running on a CA async task, fires a bundle event while CMActivator::Stop waits for
 * that task to finish.
 *
 * Thread A (shutdown): CMActivator::Stop takes WriteLock(*notificationLock) and then
 * waits in StopAndWaitForAllAsync for the Removed task.
 * Thread B (CA worker): Removed -> Bundle::Stop -> CA's bundle listener blocks on
 * ReadLock(*notificationLock).
 */
TEST(CMShutdownDeadlockTest, testFrameworkStopWhileManagedServiceFactoryRemovedStopsBundle)
{
    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();
    auto context = framework.GetBundleContext();

    auto sync = std::make_shared<Sync>();

    // Install the bundle stopped by Removed() before CA, so it has a lower bundle id
    // and framework shutdown has not reached it yet when it stops CA.
    sync->bundleToStop = test::InstallAndStartBundle(context, "TestBundleManagedServiceFactory");
    ASSERT_TRUE(sync->bundleToStop);
    test::InstallAndStartConfigAdmin(context);

    auto caRef = context.GetServiceReference<cm::ConfigurationAdmin>();
    ASSERT_TRUE(caRef);
    auto configAdmin = context.GetService<cm::ConfigurationAdmin>(caRef);

    // Signal when CMActivator::Stop unregisters the ConfigurationAdmin service.
    // That happens on the shutdown thread, after it has taken the write lock.
    std::shared_future<void> caUnregistering = sync->caUnregistering.get_future().share();
    context.AddServiceListener(
        [sync](cppmicroservices::ServiceEvent const& evt)
        {
            if (evt.GetType() == cppmicroservices::ServiceEvent::SERVICE_UNREGISTERING)
            {
                std::call_once(sync->caUnregisteringOnce, [&sync] { sync->caUnregistering.set_value(); });
            }
        },
        "(" + cppmicroservices::Constants::OBJECTCLASS + "=" + us_service_interface_iid<cm::ConfigurationAdmin>()
            + ")");

    context.RegisterService<cppmicroservices::BundleEventHook>(
        std::make_shared<BlockingBundleEventHook>(sync, caUnregistering));

    context.RegisterService<cm::ManagedServiceFactory>(
        std::make_shared<BundleStoppingFactory>(sync),
        cppmicroservices::ServiceProperties { { "service.pid", std::string(FACTORY_PID) } });

    auto configuration = configAdmin->GetFactoryConfiguration(FACTORY_PID, "remotehost");
    configuration
        ->Update(cppmicroservices::AnyMap { cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS,
                                            { { "host", std::string("remotehost") } } })
        .get();
    ASSERT_EQ(sync->updated.get_future().wait_for(WAIT_TIMEOUT), std::future_status::ready)
        << "ManagedServiceFactory::Updated was never called";

    // Queue the CA task which calls ManagedServiceFactory::Removed. Don't wait on it.
    auto removeFuture = configuration->Remove();
    ASSERT_EQ(sync->hookReached.get_future().wait_for(WAIT_TIMEOUT), std::future_status::ready)
        << "ManagedServiceFactory::Removed never stopped the bundle";

    configAdmin.reset();
    configuration.reset();

    // Shut down while the Removed() task is still running. Before the fix this blocks
    // forever in CMActivator::Stop.
    framework.Stop();
    auto evt = framework.WaitForStop(std::chrono::milliseconds::zero());
    EXPECT_EQ(evt.GetType(), cppmicroservices::FrameworkEvent::Type::FRAMEWORK_STOPPED);
}
