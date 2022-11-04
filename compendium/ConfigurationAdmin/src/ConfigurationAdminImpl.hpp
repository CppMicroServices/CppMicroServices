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
#include "cppmicroservices/asyncworkservice/AsyncWorkService.hpp"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"
#include "cppmicroservices/cm/ConfigurationListener.hpp"
#include "cppmicroservices/cm/ManagedService.hpp"
#include "cppmicroservices/cm/ManagedServiceFactory.hpp"
#include "cppmicroservices/logservice/LogService.hpp"

#include "ConfigurationAdminPrivate.hpp"
#include "ConfigurationImpl.hpp"

namespace cppmicroservices {
namespace cmimpl {

/**
 * A wrapper class used for storing the pid of a given ManagedService or ManagedServiceFactory
 * with the service in the ServiceTracker.
 */
template<typename TrackedServiceType>
class TrackedServiceWrapper
{
public:

  TrackedServiceWrapper(std::string trackedPid,
                        std::unordered_map<std::string, unsigned long> initialChangeCountPerPid,
                        std::shared_ptr<TrackedServiceType> service)
      : pid(std::move(trackedPid))
      , trackedService(std::move(service))
      , lastUpdatedChangeCountPerPid(std::move(initialChangeCountPerPid))
  {}

  TrackedServiceWrapper(const TrackedServiceWrapper&) = delete;
  TrackedServiceWrapper& operator=(const TrackedServiceWrapper&) = delete;
  TrackedServiceWrapper(TrackedServiceWrapper&&) = delete;
  TrackedServiceWrapper& operator=(TrackedServiceWrapper&&) = delete;

  explicit operator bool() const { return static_cast<bool>(trackedService); }

  std::string getPid() noexcept {
    return pid;
  }

  std::shared_ptr<TrackedServiceType> getTrackedService() noexcept {
    return trackedService;
  }

  void setLastUpdatedChangeCount(const std::string& pid, const unsigned long& changeCount) {
    std::unique_lock<std::mutex> lock(updatedChangeCountMutex);
    lastUpdatedChangeCountPerPid[pid] = changeCount;
  }

  bool needsAnUpdateNotification(const std::string& pid, const unsigned long& changeCount) {
    std::unique_lock<std::mutex> lock(updatedChangeCountMutex);
    return lastUpdatedChangeCountPerPid[pid] < changeCount;
  }

private:
  std::string pid;
  std::shared_ptr<TrackedServiceType> trackedService;
  std::unordered_map<std::string, unsigned long> lastUpdatedChangeCountPerPid; ///< the change count for each pid or factory pid instance
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
      TrackedServiceWrapper<
        cppmicroservices::service::cm::ManagedServiceFactory>>
{
public:
  ConfigurationAdminImpl(
    cppmicroservices::BundleContext cmContext,
    const std::shared_ptr<cppmicroservices::logservice::LogService>& logger,
    const std::shared_ptr<cppmicroservices::async::AsyncWorkService>&
      asyncWorkService);
  ~ConfigurationAdminImpl() override;
  ConfigurationAdminImpl(const ConfigurationAdminImpl&) = delete;
  ConfigurationAdminImpl& operator=(const ConfigurationAdminImpl&) = delete;
  ConfigurationAdminImpl(ConfigurationAdminImpl&&) = delete;
  ConfigurationAdminImpl& operator=(ConfigurationAdminImpl&&) = delete;

  /**
   * Get an existing or new {@code Configuration} object.
   *
   * See {@code ConfigurationAdmin#GetConfiguration}
   */
  std::shared_ptr<cppmicroservices::service::cm::Configuration>
  GetConfiguration(const std::string& pid) override;

  /**
   * Create a new {@code Configuration} object for a {@code ManagedServiceFactory}.
   *
   * See {@code ConfigurationAdmin#CreateFactoryConfiguration}
   */
  std::shared_ptr<cppmicroservices::service::cm::Configuration>
  CreateFactoryConfiguration(const std::string& factoryPid) override;

  /**
   * Get an existing or new {@code Configuration} object for a {@code ManagedServiceFactory}.
   *
   * See {@code ConfigurationAdmin#GetFactoryConfiguration}
   */
  std::shared_ptr<cppmicroservices::service::cm::Configuration>
  GetFactoryConfiguration(const std::string& factoryPid,
                          const std::string& instanceName) override;

  /**
   * Used to list all of the {@code Configuration} objects that exist in the
   * ConfigurationAdmin repository with a pid that matches the filter expression 
   * (if provided).
   * All of the {@code Configuration} objects returned have been updated at least
   * once by ConfigurationAdmin.
   *
   * See {@code ConfigurationAdmin#ListConfigurations}
   */
  std::vector<std::shared_ptr<cppmicroservices::service::cm::Configuration>>
  ListConfigurations(const std::string& filter = std::string{}) override;

  /**
   * Internal method used by {@code CMBundleExtension} to add new {@code Configuration} objects
   *
   * See {@code ConfigurationAdminPrivate#AddConfigurations}
   */
  std::vector<ConfigurationAddedInfo> AddConfigurations(
    std::vector<metadata::ConfigurationMetadata> configurationMetadata)
    override;

  /**
   * Internal method used by {@code CMBundleExtension} to remove the {@code Configuration} objects that it created
   *
   * See {@code ConfigurationAdminPrivate#RemoveConfigurations}
   */
  void RemoveConfigurations(
    std::vector<ConfigurationAddedInfo> pidsAndChangeCountsAndIDs) override;

  /**
   * Internal method used to notify any {@code ManagedService} or {@code ManagedServiceFactory} or 
   * {@code ConfigurationListener} of an update to a {@code Configuration}. Performs the 
   * notifications asynchronously with the latest state of the properties at the time.
   *
   * See {@code ConfigurationAdminPrivate#NotifyConfigurationUpdated}
   */
  std::shared_future<void> NotifyConfigurationUpdated(
    const std::string& pid, const unsigned long changeCount) override;

  /**
   * Internal method used by {@code ConfigurationImpl} to notify any {@code ManagedService} or
   * {@code ManagedServiceFactory} of the removal of a {@code Configuration}. Performs the notifications
   * asynchronously.
   *
   * See {@code ConfigurationAdminPrivate#NotifyConfigurationRemoved}
   */
  std::shared_future<void> NotifyConfigurationRemoved(
    const std::string& pid,
    std::uintptr_t configurationId) override;

  // methods from the cppmicroservices::ServiceTrackerCustomizer interface for ManagedService
  std::shared_ptr<
    TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>
  AddingService(
    const ServiceReference<cppmicroservices::service::cm::ManagedService>&
      reference) override;
  void ModifiedService(
    const ServiceReference<cppmicroservices::service::cm::ManagedService>&
      reference,
    const std::shared_ptr<
      TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>&
      service) override;
  void RemovedService(
    const ServiceReference<cppmicroservices::service::cm::ManagedService>&
      reference,
    const std::shared_ptr<
      TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>&
      service) override;

  // methods from the cppmicroservices::ServiceTrackerCustomizer interface for ManagedServiceFactory
  std::shared_ptr<
    TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>>
  AddingService(const ServiceReference<
                cppmicroservices::service::cm::ManagedServiceFactory>&
                  reference) override;
  void ModifiedService(
    const ServiceReference<
      cppmicroservices::service::cm::ManagedServiceFactory>& reference,
    const std::shared_ptr<TrackedServiceWrapper<
      cppmicroservices::service::cm::ManagedServiceFactory>>& service) override;
  void RemovedService(
    const ServiceReference<
      cppmicroservices::service::cm::ManagedServiceFactory>& reference,
    const std::shared_ptr<TrackedServiceWrapper<
      cppmicroservices::service::cm::ManagedServiceFactory>>& service) override;

  // Used by tests to avoid race conditions and CMBundleExtension destructor to make sure all asynchronous 
  // threads have completed. 
  void WaitForAllAsync();

private:
  // Convenience wrapper which is used to perform asyncronous operations
  template<typename Functor>
  std::shared_future<void> PerformAsync(Functor&& f);

  // Used to generate a random instance name for CreateFactoryConfiguration
  std::string RandomInstanceName();

  // Used to keep track of the instances of each ManagedServiceFactory
  void AddFactoryInstanceIfRequired(const std::string& pid,
                                    const std::string& factoryPid);
  void RemoveFactoryInstanceIfRequired(const std::string& pid);

  cppmicroservices::BundleContext cmContext;
  std::shared_ptr<cppmicroservices::logservice::LogService> logger;
  std::shared_ptr<cppmicroservices::async::AsyncWorkService> asyncWorkService;
  std::mutex configurationsMutex;
  std::unordered_map<std::string, std::shared_ptr<ConfigurationImpl>>
    configurations;
  std::unordered_map<std::string, std::set<std::string>> factoryInstances;
  std::mutex futuresMutex;
  std::uint64_t futuresID;
  std::condition_variable futuresCV;
  std::vector<std::shared_future<void>> completeFutures;
  std::unordered_map<std::uint64_t, std::shared_future<void>> incompleteFutures;
  cppmicroservices::ServiceTracker<
    cppmicroservices::service::cm::ManagedService,
    TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>
    managedServiceTracker;
  cppmicroservices::ServiceTracker<
    cppmicroservices::service::cm::ManagedServiceFactory,
    TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>>
    managedServiceFactoryTracker;
  std::mt19937 randomGenerator;
  cppmicroservices::ServiceTracker<
    cppmicroservices::service::cm::ConfigurationListener>
    configListenerTracker;

  // used instead of querying the service trackers since a race exists between when the service tracker
  // adds the tracked service to the internal map and when a client asks the service tracker for the
  // list of tracked objects.
  std::vector<std::shared_ptr<
    TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>>
    trackedManagedServices_;
  std::vector<std::shared_ptr<TrackedServiceWrapper<
    cppmicroservices::service::cm::ManagedServiceFactory>>>
    trackedManagedServiceFactories_;
};
} // cmimpl
} // cppmicroservices

#endif /* CONFIGURATIONADMINIMPL_HPP */
