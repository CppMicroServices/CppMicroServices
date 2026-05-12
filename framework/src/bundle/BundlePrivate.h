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

#ifndef CPPMICROSERVICES_BUNDLEPRIVATE_H
#define CPPMICROSERVICES_BUNDLEPRIVATE_H

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleInitialization.h"
#include "cppmicroservices/BundleVersion.h"
#include "cppmicroservices/SharedLibrary.h"
#include "cppmicroservices/detail/Threads.h"
#include "cppmicroservices/detail/WaitCondition.h"

#include "BundleArchive.h"
#include "BundleManifest.h"

#include <functional>
#include <future>
#include <memory>
#include <ostream>
#include <thread>

namespace cppmicroservices
{

    class CoreBundleContext;
    class Bundle;
    class BundleContextPrivate;
    class BundleLifecycleState;
    struct BundleActivator;

    /**
     * \ingroup MicroServices
     */
    class BundlePrivate
        : public detail::MultiThreaded<detail::MutexLockingStrategy<>, detail::WaitCondition>
        , public std::enable_shared_from_this<BundlePrivate>
    {

      public:
        using MutexHost = MutexLockingStrategy<>;
        using LockType = MutexHost::UniqueLock;
        using WaitConditionType = WaitCondition<MutexHost>;

        BundlePrivate(BundlePrivate const&) = delete;
        BundlePrivate& operator=(BundlePrivate const&) = delete;

        /**
         * Construct a new Bundle.
         *
         * Only called for the system bundle.
         *
         * @param coreCtx CoreBundleContext for this bundle.
         */
        BundlePrivate(CoreBundleContext* coreCtx);

        /**
         * Construct a new Bundle based on a BundleArchive.
         *
         * @param coreCtx CoreBundleContext for this bundle.
         * @param ba Bundle archive with holding the contents of the bundle.
         * @throws std::runtime_error If we have duplicate symbolic name and version.
         * @throws std::invalid_argument Faulty manifest for bundle
         */
        BundlePrivate(CoreBundleContext* coreCtx, std::shared_ptr<BundleArchive> const& ba);

        virtual ~BundlePrivate();

        /**
         * Check if bundle is in state UNINSTALLED. If so, throw exception.
         */
        void CheckUninstalled() const;

        void RemoveBundleResources();

        virtual void Stop(uint32_t);

        /**
         * Fire BUNDLE_STOPPING event, call BundleActivator::Stop, check for
         * abort, then clean up resources via InvalidateBundleContext.
         */
        std::exception_ptr DeactivateBundle();

        /**
         * Run stop hooks, remove bundle resources (services, listeners),
         * and invalidate the BundleContext.
         */
        void InvalidateBundleContext();

        /**
         * Get updated bundle state. That means check if an installed bundle has been
         * resolved.
         *
         * @return Bundles state
         */
        Bundle::State GetUpdatedState();

        /**
         * Set state to BUNDLE_INSTALLED.
         * We assume that the bundle is resolved when entering this method.
         */
        void SetStateInstalled(bool sendEvent);

        /**
         * Purge any old files and data associated with this bundle.
         */
        void Purge();

        /**
         * Get bundle archive.
         *
         * @return BundleArchive object.
         */
        std::shared_ptr<BundleArchive> GetBundleArchive(/*long generation*/) const;

        /**
         * Save the autostart setting to the persistent bundle storage.
         *
         * @param setting The autostart options to save.
         */
        void SetAutostartSetting(int32_t setting);

        /**
         * Get the autostart setting from the bundle storage.
         *
         * @return the current autostart setting, "-1" if bundle not started.
         */
        int32_t GetAutostartSetting() const;

        /**
         * Auto-resolve the bundle if INSTALLED, then delegate to the
         * current lifecycle state to activate.
         */
        void ResolveAndActivate();

        virtual void Uninstall();

        virtual std::string GetLocation() const;

        virtual void Start(uint32_t options);

        virtual AnyMap const& GetHeaders() const;

        /**
         * Fire BUNDLE_STARTING event, load shared library, create and call
         * BundleActivator::Start. On success, transition to ACTIVE.
         * Returns an exception_ptr on failure.
         */
        std::exception_ptr ActivateBundle();

        /**
         * Clean up after a failed activation: transition through
         * STOPPING -> RESOLVED, fire events, invalidate context.
         */
        void CleanupAfterActivationFailure();

        /**
         * Framework context.
         */
        CoreBundleContext* coreCtx;

        /**
         * Bundle identifier.
         */
        long const id;

        /**
         * Bundle location identifier.
         */
        const std::string location;

        /**
         * Lifecycle state object (new state machine).
         * Managed via std::atomic operations (compare_exchange_strong).
         */
        std::shared_ptr<BundleLifecycleState> lifecycleState;

        /**
         * Atomically compare and swap the lifecycle state.
         * Returns true if the swap succeeded.
         */
        bool CompareAndSetState(std::shared_ptr<BundleLifecycleState>* expected,
                                std::shared_ptr<BundleLifecycleState> desired);

        /**
         * Get the current lifecycle state object.
         */
        std::shared_ptr<BundleLifecycleState> GetLifecycleState() const;

        /**
         * Get the current bundle state enum value from the lifecycle state object.
         */
        Bundle::State GetBundleStateEnum() const;

        /**
         * Directly set the lifecycle state (for FrameworkPrivate which manages
         * its own transitions). This is NOT a CAS -- use only when holding a lock.
         */
        void SetLifecycleState(std::shared_ptr<BundleLifecycleState> newState);

        /**
         * Deactivate the bundle and CAS from STOPPING to RESOLVED.
         * Called by BundleActiveState::Stop and BundleStartingState::Stop
         * after they CAS to STOPPING.
         */
        std::exception_ptr DeactivateAndTransitionToResolved(
            std::shared_ptr<BundleLifecycleState> const& stoppingState);

        /**
         * Remove bundle from registry, fire UNRESOLVED event, purge storage,
         * and transition to UNINSTALLED.
         */
        void RemoveAndCleanupBundle();

        /**
         * Helper: report an exception as a framework error event.
         */
        void ReportFrameworkError(std::exception_ptr exception);

        /**
         * Bundle archive containing persistent data.
         */
        std::shared_ptr<BundleArchive> barchive;

        /**
         * Directory for bundle data.
         */
        std::string bundleDir;

        /**
         * BundleContext for the bundle
         */
        detail::Atomic<std::shared_ptr<BundleContextPrivate>> bundleContext;

        using DestroyActivatorHook = std::function<void(BundleActivator*)>;

        DestroyActivatorHook destroyActivatorHook;

        std::unique_ptr<BundleActivator, DestroyActivatorHook> bactivator;

        /** Saved exception of resolve failure. */
        std::exception_ptr resolveFailException;

        /** Remember if bundle was started */
        bool wasStarted;

        enum class Aborted : uint8_t
        {
            NONE,
            YES,
            NO
        };

        /** start/stop time-out/uninstall flag */
        // GCC 4.6 atomics do not support custom trivially copyable types
        // like enums yet, so we use the underlying primitive type here.
        std::atomic<uint8_t> aborted;

        /**
         * Bundle symbolic name.
         */
        const std::string symbolicName;

        /**
         * Bundle version
         */
        // Does not need to be locked by "this" when accessed.
        BundleVersion version;

        /**
         * Time when bundle was last modified.
         *
         */
        Bundle::TimeStamp timeStamp;

        // Does not need to be locked by "this" when accessed.
        BundleManifest bundleManifest;

        // -------------------------------------------------------------------------------

        /**
         * Responsible for platform specific loading and unloading
         * of the bundle's physical form.
         */
        SharedLibrary lib;

        SetBundleContextFn SetBundleContext;
    };

    Bundle MakeBundle(std::shared_ptr<BundlePrivate> const& d);
    std::shared_ptr<BundlePrivate> GetPrivate(Bundle const& b);
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_BUNDLEPRIVATE_H
