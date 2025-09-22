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

#ifndef CONFIGURATIONADMINIMPL_HPP
#define CONFIGURATIONADMINIMPL_HPP

#include <cstdint>
#include <future>
#include <memory>
#include <mutex>
#include <random>
#include <unordered_map>

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceTracker.h"
#include "cppmicroservices/ServiceTrackerCustomizer.h"
#include "cppmicroservices/ThreadpoolSafeFuture.h"
#include "cppmicroservices/asyncworkservice/AsyncWorkService.hpp"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"
#include "cppmicroservices/cm/ConfigurationListener.hpp"
#include "cppmicroservices/cm/ManagedService.hpp"
#include "cppmicroservices/cm/ManagedServiceFactory.hpp"
#include "cppmicroservices/logservice/LogService.hpp"

#include "ConfigurationAdminPrivate.hpp"
#include "ConfigurationImpl.hpp"
#include "ThreadpoolSafeFuturePrivate.hpp"

namespace cppmicroservices
{
    namespace cmimpl
    {

        /**
         * A wrapper class used for storing the pid of a given ManagedService or ManagedServiceFactory
         * with the service in the ServiceTracker.
         */
        template <typename TrackedServiceType>
        class TrackedServiceWrapper
        {
          public:
            TrackedServiceWrapper(std::string trackedPid,
                                  std::unordered_map<std::string, unsigned long> initialChangeCountPerPid,
                                  std::shared_ptr<TrackedServiceType> service)
                : pid(std::move(trackedPid))
                , trackedService(std::move(service))
                , lastUpdatedChangeCountPerPid(std::move(initialChangeCountPerPid))
            {
            }

            TrackedServiceWrapper(TrackedServiceWrapper const&) = delete;
            TrackedServiceWrapper& operator=(TrackedServiceWrapper const&) = delete;
            TrackedServiceWrapper(TrackedServiceWrapper&&) = delete;
            TrackedServiceWrapper& operator=(TrackedServiceWrapper&&) = delete;

            explicit
            operator bool() const
            {
                return static_cast<bool>(trackedService);
            }

            std::string
            getPid() noexcept
            {
                return pid;
            }

            std::shared_ptr<TrackedServiceType>
            getTrackedService() noexcept
            {
                return trackedService;
            }

            void
            setLastUpdatedChangeCount(std::string const& pid, unsigned long const& changeCount)
            {
                std::unique_lock<std::mutex> lock(updatedChangeCountMutex);
                lastUpdatedChangeCountPerPid[pid] = changeCount;
            }

            bool
            needsAnUpdateNotification(std::string const& pid, unsigned long const& changeCount)
            {
                std::unique_lock<std::mutex> lock(updatedChangeCountMutex);
                return lastUpdatedChangeCountPerPid[pid] < changeCount;
            }

            void
            removeLastUpdatedChangeCount(std::string const& pid) noexcept
            {
                std::unique_lock<std::mutex> lock(updatedChangeCountMutex);
                (void)lastUpdatedChangeCountPerPid.erase(pid);
            }

          private:
            std::string pid;
            std::shared_ptr<TrackedServiceType> trackedService;
            std::unordered_map<std::string, unsigned long>
                lastUpdatedChangeCountPerPid;   ///< the change count for each pid or factory pid instance
            std::mutex updatedChangeCountMutex; ///< guard read/write access to lastUpdatedChangeCountPerPid
        };

        /**
         * This class implements the {@code cppmicroservices::service::cm::ConfigurationAdmin} interface.
         */
        class ConfigurationAdminImpl final
            : public cppmicroservices::service::cm::ConfigurationAdmin
            , public ConfigurationAdminPrivate
            , public cppmicroservices::ServiceTrackerCustomizer<
                  cppmicroservices::service::cm::ManagedService,
                  TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>
            , public cppmicroservices::ServiceTrackerCustomizer<
                  cppmicroservices::service::cm::ManagedServiceFactory,
                  TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>>
        {
          public:
            ConfigurationAdminImpl(cppmicroservices::BundleContext cmContext,
                                   std::shared_ptr<cppmicroservices::logservice::LogService> const& logger,
                                   std::shared_ptr<cppmicroservices::async::AsyncWorkService> const& asyncWorkService);
            ~ConfigurationAdminImpl() override;
            ConfigurationAdminImpl(ConfigurationAdminImpl const&) = delete;
            ConfigurationAdminImpl& operator=(ConfigurationAdminImpl const&) = delete;
            ConfigurationAdminImpl(ConfigurationAdminImpl&&) = delete;
            ConfigurationAdminImpl& operator=(ConfigurationAdminImpl&&) = delete;

            /**
             * Get an existing or new {@code Configuration} object.
             *
             * See {@code ConfigurationAdmin#GetConfiguration}
             */
            std::shared_ptr<cppmicroservices::service::cm::Configuration> GetConfiguration(
                std::string const& pid) override;

            /**
             * Create a new {@code Configuration} object for a {@code ManagedServiceFactory}.
             *
             * See {@code ConfigurationAdmin#CreateFactoryConfiguration}
             */
            std::shared_ptr<cppmicroservices::service::cm::Configuration> CreateFactoryConfiguration(
                std::string const& factoryPid) override;

            /**
             * Get an existing or new {@code Configuration} object for a {@code ManagedServiceFactory}.
             *
             * See {@code ConfigurationAdmin#GetFactoryConfiguration}
             */
            std::shared_ptr<cppmicroservices::service::cm::Configuration> GetFactoryConfiguration(
                std::string const& factoryPid,
                std::string const& instanceName) override;

            /**
             * Used to list all of the {@code Configuration} objects that exist in the
             * ConfigurationAdmin repository with a pid that matches the filter expression
             * (if provided).
             * All of the {@code Configuration} objects returned have been updated at least
             * once by ConfigurationAdmin.
             *
             * See {@code ConfigurationAdmin#ListConfigurations}
             */
            std::vector<std::shared_ptr<cppmicroservices::service::cm::Configuration>> ListConfigurations(
                std::string const& filter = std::string {}) override;

            /**
             * Internal method used by {@code CMBundleExtension} to add new {@code Configuration} objects
             *
             * See {@code ConfigurationAdminPrivate#AddConfigurations}
             */
            std::vector<ConfigurationAddedInfo> AddConfigurations(
                std::vector<metadata::ConfigurationMetadata> configurationMetadata) override;

            /**
             * Internal method used by {@code CMBundleExtension} to remove the {@code Configuration} objects that it
             * created
             *
             * See {@code ConfigurationAdminPrivate#RemoveConfigurations}
             */
            void RemoveConfigurations(std::vector<ConfigurationAddedInfo> pidsAndChangeCountsAndIDs) override;

            /**
             * Internal method used to notify any {@code ManagedService} or {@code ManagedServiceFactory} or
             * {@code ConfigurationListener} of an update to a {@code Configuration}. Performs the
             * notifications asynchronously with the latest state of the properties at the time.
             *
             * See {@code ConfigurationAdminPrivate#NotifyConfigurationUpdated}
             */
            std::shared_ptr<ThreadpoolSafeFuturePrivate> NotifyConfigurationUpdated(
                std::string const& pid,
                unsigned long const changeCount,
                std::shared_ptr<AsyncWorkService> strand) override;

            /**
             * Internal method used by {@code ConfigurationImpl} to notify any {@code ManagedService} or
             * {@code ManagedServiceFactory} of the removal of a {@code Configuration}. Performs the notifications
             * asynchronously.
             *
             * See {@code ConfigurationAdminPrivate#NotifyConfigurationRemoved}
             */
            std::shared_ptr<ThreadpoolSafeFuturePrivate> NotifyConfigurationRemoved(
                std::string const& pid,
                std::uintptr_t configurationId,
                unsigned long changeCount,
                std::shared_ptr<AsyncWorkService> strand) override;

            // methods from the cppmicroservices::ServiceTrackerCustomizer interface for ManagedService
            std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>> AddingService(
                ServiceReference<cppmicroservices::service::cm::ManagedService> const& reference) override;
            void ModifiedService(
                ServiceReference<cppmicroservices::service::cm::ManagedService> const& reference,
                std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>> const& service)
                override;
            void RemovedService(
                ServiceReference<cppmicroservices::service::cm::ManagedService> const& reference,
                std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>> const& service)
                override;

            // methods from the cppmicroservices::ServiceTrackerCustomizer interface for ManagedServiceFactory
            std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>> AddingService(
                ServiceReference<cppmicroservices::service::cm::ManagedServiceFactory> const& reference) override;
            void ModifiedService(
                ServiceReference<cppmicroservices::service::cm::ManagedServiceFactory> const& reference,
                std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>> const&
                    service) override;
            void RemovedService(
                ServiceReference<cppmicroservices::service::cm::ManagedServiceFactory> const& reference,
                std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>> const&
                    service) override;
            // Used by tests to avoid race conditions
            // Used by CMActivator to ensure all async work is finished BEFORE shutdown
            void WaitForAllAsync();

            //  prohibits any further async work from being queued in the CMAsyncWorkService
            void StopAndWaitForAllAsync();

          private:
            // Convenience wrapper which is used to perform asyncronous operations
            template <typename Functor>
            std::shared_ptr<ThreadpoolSafeFuturePrivate> PerformAsync(Functor&& f,
                                                                      std::shared_ptr<AsyncWorkService> strand
                                                                      = nullptr);

            // Flag set to false when the activator has stopped the bundle
            bool active = true;

            // Used to generate a random instance name for CreateFactoryConfiguration
            std::string RandomInstanceName();

            // Used to keep track of the instances of each ManagedServiceFactory
            void AddFactoryInstanceIfRequired(std::string const& pid, std::string const& factoryPid);
            void RemoveFactoryInstanceIfRequired(std::string const& pid);

            cppmicroservices::BundleContext cmContext;
            std::shared_ptr<cppmicroservices::logservice::LogService> logger;
            std::shared_ptr<cppmicroservices::async::AsyncWorkService> asyncWorkService;
            std::mutex configurationsMutex;
            std::unordered_map<std::string, std::shared_ptr<ConfigurationImpl>> configurations;
            std::unordered_map<std::string, unsigned long> instanceCount;
            std::unordered_map<std::string, std::set<std::string>> factoryInstances;
            std::mutex futuresMutex;
            std::uint64_t futuresID;
            std::condition_variable futuresCV;
            std::vector<std::shared_future<void>> completeFutures;
            std::unordered_map<std::uint64_t, std::shared_future<void>> incompleteFutures;
            cppmicroservices::ServiceTracker<cppmicroservices::service::cm::ManagedService,
                                             TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>
                managedServiceTracker;
            cppmicroservices::ServiceTracker<
                cppmicroservices::service::cm::ManagedServiceFactory,
                TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>>
                managedServiceFactoryTracker;
            cppmicroservices::ServiceTracker<cppmicroservices::service::cm::ConfigurationListener>
                configListenerTracker;

            // used instead of querying the service trackers since a race exists between when the service tracker
            // adds the tracked service to the internal map and when a client asks the service tracker for the
            // list of tracked objects.
            std::vector<std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>>
                trackedManagedServices_;
            std::vector<std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>>>
                trackedManagedServiceFactories_;
        };
    } // namespace cmimpl
} // namespace cppmicroservices

#endif /* CONFIGURATIONADMINIMPL_HPP */
