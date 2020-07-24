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

#include <cassert>
#include <stdexcept>
#include <thread>

#include "cppmicroservices/Constants.h"
#include "cppmicroservices/cm/ConfigurationException.hpp"

#include "CMConstants.hpp"
#include "ConfigurationAdminImpl.hpp"

using cppmicroservices::logservice::SeverityLevel;

namespace {
  std::string getFactoryPid(const std::string& pid)
  {
    const auto pos = pid.find_first_of('~');
    if (pos == std::string::npos)
    {
      return {};
    }
    return pid.substr(0, pos);
  }

  void handleUpdatedException(const std::string& pid,
                              const cppmicroservices::AnyMap& properties,
                              cppmicroservices::logservice::LogService& logger,
                              bool isFactory)
  {
    auto thrownByMessage = "thrown by ManagedService" + (isFactory ? std::string("Factory") : "");
    thrownByMessage += " with PID " + pid + " whilst being Updated with new properties.\n\t";
    try
    {
      throw;
    }
    catch(const cppmicroservices::service::cm::ConfigurationException& ce)
    {
      const auto& property = ce.GetProperty();
      std::string propertyCausingError;
      if (!property.empty())
      {
        propertyCausingError += "The property which caused this error was '" + property + "' which had the value: \n\t";
        propertyCausingError += properties.AtCompoundKey(property, cppmicroservices::Any()).ToStringNoExcept();
      }
      else
      {
        propertyCausingError += "It was not specified which property caused this error.";
      }
      logger.Log(SeverityLevel::LOG_ERROR, "ConfigurationException " + thrownByMessage + "Exception reason: "
                                         + ce.GetReason() + "\n\t" + propertyCausingError);
    }
    catch (const std::exception &e)
    {
      logger.Log(SeverityLevel::LOG_ERROR, "Exception " + thrownByMessage + "Exception: " + e.what());
    }
    catch (...)
    {
      logger.Log(SeverityLevel::LOG_ERROR, "Unknown exception " + thrownByMessage);
    }
  }

  void notifyServiceUpdated(const std::string& pid,
                            cppmicroservices::service::cm::ManagedService& managedService,
                            const cppmicroservices::AnyMap& properties,
                            cppmicroservices::logservice::LogService& logger)
  {
    try
    {
      managedService.Updated(properties);
    }
    catch (...)
    {
      handleUpdatedException(pid, properties, logger, /* isFactory = */ false);
    }
  }

  void notifyServiceUpdated(const std::string& pid,
                            cppmicroservices::service::cm::ManagedServiceFactory& managedServiceFactory,
                            const cppmicroservices::AnyMap& properties,
                            cppmicroservices::logservice::LogService& logger)
  {
    try
    {
      managedServiceFactory.Updated(pid, properties);
    }
    catch (...)
    {
      handleUpdatedException(pid, properties, logger, /* isFactory = */ true);
    }
  }

  void notifyServiceRemoved(const std::string& pid,
                            cppmicroservices::service::cm::ManagedServiceFactory& managedServiceFactory,
                            cppmicroservices::logservice::LogService& logger)
  {
    try
    {
      managedServiceFactory.Removed(pid);
    }
    catch (const std::exception &e)
    {
      logger.Log(SeverityLevel::LOG_ERROR,
                 "Exception thrown by ManagedServiceFactory with PID " + pid
               + " whilst being notified of Configuration Removal.\n\t" + "Exception: " + e.what());
    }
    catch (...)
    {
      logger.Log(SeverityLevel::LOG_ERROR,
                 "Unknown exception thrown by ManagedServiceFactory with PID " + pid
               + " whilst being notified of Configuration Removal.");
    }
  }

  template <typename T>
  std::string getPidFromServiceReference(const T& reference)
  {
    using namespace cppmicroservices::cmimpl::CMConstants;
    try
    {
      const auto serviceProp = reference.GetProperty(CM_SERVICE_KEY);
      const auto &serviceMap = cppmicroservices::ref_any_cast<cppmicroservices::AnyMap>(serviceProp);
      return cppmicroservices::any_cast<std::string>(serviceMap.AtCompoundKey(CM_SERVICE_SUBKEY));
    }
    catch (...)
    {
      // Service does not have a "service" property with a "pid" string subproperty.
    }
    try
    {
      const auto componentProp = reference.GetProperty(CM_COMPONENT_KEY);
      const auto &componentMap = cppmicroservices::ref_any_cast<cppmicroservices::AnyMap>(componentProp);
      return cppmicroservices::any_cast<std::string>(componentMap.AtCompoundKey(CM_COMPONENT_SUBKEY));
    }
    catch (...)
    {
      // Service does not have a "component" property with a "name" string property.
    }
    try
    {
      return cppmicroservices::any_cast<std::string>(reference.GetProperty(CM_SERVICE_KEY + std::string(".") + CM_SERVICE_SUBKEY));
    }
    catch (...)
    {
      // Service does not have a "service.pid" string property.
    }
    try
    {
      return cppmicroservices::any_cast<std::string>(reference.GetProperty(CM_COMPONENT_KEY + std::string(".") + CM_COMPONENT_SUBKEY));
    }
    catch (...)
    {
      // Service does not have a "component.name" string property.
    }
    return {};
  }
}  // namespace

namespace cppmicroservices {
  namespace cmimpl {

    ConfigurationAdminImpl::ConfigurationAdminImpl(cppmicroservices::BundleContext context,
                                                   std::shared_ptr<cppmicroservices::logservice::LogService> lggr)
    : cmContext(std::move(context))
    , logger(std::move(lggr))
    , futuresID{0u}
    , managedServiceTracker(cmContext, this)
    , managedServiceFactoryTracker(cmContext, this)
    , randomGenerator(std::random_device{}())
    {
      managedServiceTracker.Open();
      managedServiceFactoryTracker.Open();
    }

    ConfigurationAdminImpl::~ConfigurationAdminImpl()
    {
      auto managedServiceWrappers = managedServiceTracker.GetServices();
      auto managedServiceFactoryWrappers = managedServiceFactoryTracker.GetServices();

      managedServiceFactoryTracker.Close();
      managedServiceTracker.Close();

      decltype(factoryInstances) factoryInstancesCopy;
      decltype(configurations) configurationsToInvalidate;
      {
        std::lock_guard<std::mutex> lk{configurationsMutex};
        factoryInstancesCopy.swap(factoryInstances);
        configurationsToInvalidate.swap(configurations);
      }
      for (auto& configuration : configurationsToInvalidate)
      {
        configuration.second->Invalidate();
      }
      AnyMap emptyMap{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS};
      for (auto& managedService : managedServiceWrappers)
      {
        // The ServiceTracker will return a default constructed shared_ptr for each ManagedService
        // that we aren't tracking. We must be careful not to dereference these!
        if (managedService)
        {
          notifyServiceUpdated(managedService->pid, *(managedService->trackedService), emptyMap, *logger);
        }
      }
      for (auto & managedServiceFactory : managedServiceFactoryWrappers)
      {
        // The ServiceTracker will return a default constructed shared_ptr for each ManagedServiceFactory
        // that we aren't tracking. We must be careful not to dereference these!
        if (!managedServiceFactory)
        {
          continue;
        }
        auto it = factoryInstancesCopy.find(managedServiceFactory->pid);
        if (it == std::end(factoryInstancesCopy))
        {
          continue;
        }
        for (const auto& pid : it->second)
        {
          notifyServiceRemoved(pid, *(managedServiceFactory->trackedService), *logger);
        }
      }
      std::unique_lock<std::mutex> ul{futuresMutex};
      if (!incompleteFutures.empty())
      {
        futuresCV.wait(ul, [this] { return incompleteFutures.empty(); });
      }
    }

    std::shared_ptr<cppmicroservices::service::cm::Configuration> ConfigurationAdminImpl::GetConfiguration(const std::string& pid)
    {
      std::shared_ptr<cppmicroservices::service::cm::Configuration> result;
      auto created = false;
      {
        std::lock_guard<std::mutex> lk{configurationsMutex};
        auto it = configurations.find(pid);
        if (it == std::end(configurations)) {
            auto factoryPid = getFactoryPid(pid);
            AddFactoryInstanceIfRequired(pid, factoryPid);
            it = configurations.emplace(pid, std::make_shared<ConfigurationImpl>(this, pid, std::move(factoryPid), AnyMap{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS})).first;
            created = true;
        }
        result = it->second;
      }
      if (created)
      {
        NotifyConfigurationUpdated(pid);
      }
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                  "GetConfiguration: returning " + (created ? std::string("new") : "existing") + " Configuration instance with PID " + pid);
      return result;
    }

    std::shared_ptr<cppmicroservices::service::cm::Configuration> ConfigurationAdminImpl::CreateFactoryConfiguration(const std::string& factoryPid)
    {
      std::shared_ptr<cppmicroservices::service::cm::Configuration> result;
      auto pid = factoryPid + "~" + RandomInstanceName();
      {
        std::lock_guard<std::mutex> lk{configurationsMutex};
        auto it = configurations.find(pid);
        while (it != std::end(configurations))
        {
          pid = factoryPid + "~" + RandomInstanceName();
          it = configurations.find(pid);
        }
        AddFactoryInstanceIfRequired(pid, factoryPid);
        it = configurations.emplace(pid, std::make_shared<ConfigurationImpl>(this, pid, factoryPid, AnyMap{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS})).first;
        result = it->second;
      }
      NotifyConfigurationUpdated(pid);
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                  "CreateFactoryConfiguration: returning new Configuration instance with PID " + pid);
      return result;
    }

    std::shared_ptr<cppmicroservices::service::cm::Configuration> ConfigurationAdminImpl::GetFactoryConfiguration(const std::string& factoryPid, const std::string& instanceName)
    {
      const auto pid = factoryPid + "~" + instanceName;
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                  "GetFactoryConfiguration: deferring to GetConfiguration for PID " + pid);
      return GetConfiguration(pid);
    }

    std::vector<std::shared_ptr<cppmicroservices::service::cm::Configuration>> ConfigurationAdminImpl::ListConfigurations(const std::string& /* filter */)
    {
      throw std::invalid_argument("Method not currently implemented");
    }

    std::vector<ConfigurationAddedInfo> ConfigurationAdminImpl::AddConfigurations(std::vector<metadata::ConfigurationMetadata> configurationMetadata)
    {
      std::vector<ConfigurationAddedInfo> pidsAndChangeCountsAndIDs;
      std::vector<bool> createdOrUpdated;
      std::vector<std::shared_ptr<ConfigurationImpl>> configurationsToInvalidate;
      {
        std::lock_guard<std::mutex> lk{configurationsMutex};
        for (auto& configMetadata : configurationMetadata)
        {
          auto &pid = configMetadata.pid;
          auto it = configurations.find(pid);
          if (it == std::end(configurations))
          {
            auto factoryPid = getFactoryPid(pid);
            AddFactoryInstanceIfRequired(pid, factoryPid);
            it = configurations.emplace(pid, std::make_shared<ConfigurationImpl>(this, pid, std::move(factoryPid), std::move(configMetadata.properties))).first;
            pidsAndChangeCountsAndIDs.emplace_back(pid, 1u, reinterpret_cast<std::uintptr_t>(it->second.get()));
            createdOrUpdated.push_back(true);
            continue;
          }
          // else Configuration already exists
          try {
            const auto updatedAndChangeCount = it->second->UpdateWithoutNotificationIfDifferent(configMetadata.properties);
            pidsAndChangeCountsAndIDs.emplace_back(pid, updatedAndChangeCount.second, reinterpret_cast<std::uintptr_t>(it->second.get()));
            createdOrUpdated.push_back(updatedAndChangeCount.first);
          }
          catch (const std::runtime_error&) // Configuration has been Removed by someone else, but we've won the race to handle that.
          {
            configurationsToInvalidate.push_back(std::move(it->second));
            it->second = std::make_shared<ConfigurationImpl>(this, pid, getFactoryPid(pid), std::move(configMetadata.properties));
            pidsAndChangeCountsAndIDs.emplace_back(pid, 1u, reinterpret_cast<std::uintptr_t>(it->second.get()));
            createdOrUpdated.push_back(true);
          }
        }
      }
      // This cannot be called whilst holding the configurationsMutex as it could cause a deadlock.
      for (auto& configurationToInvalidate : configurationsToInvalidate)
      {
        configurationToInvalidate->Invalidate();
      }
      auto idx = 0u;
      for (const auto& pidAndChangeCountAndID : pidsAndChangeCountsAndIDs)
      {
        const auto& pid = pidAndChangeCountAndID.pid;
        if (createdOrUpdated[idx])
        {
          NotifyConfigurationUpdated(pid);
          logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                      "AddConfigurations: Created or Updated Configuration instance with PID " + pid);
        }
        else
        {
          logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                      "AddConfigurations: Configuration already existed with identical properties with PID " + pid);
        }
        ++idx;
      }
      return pidsAndChangeCountsAndIDs;
    }

    void ConfigurationAdminImpl::RemoveConfigurations(std::vector<ConfigurationAddedInfo> pidsAndChangeCountsAndIDs)
    {
      std::vector<bool> removed;
      std::vector<std::shared_ptr<ConfigurationImpl>> configurationsToInvalidate;
      {
        std::lock_guard<std::mutex> lk{configurationsMutex};
        for (const auto& pidAndChangeCountAndID : pidsAndChangeCountsAndIDs)
        {
          const auto& pid = pidAndChangeCountAndID.pid;
          const auto it = configurations.find(pid);
          if (it == std::end(configurations))
          {
            removed.push_back(false);
            continue;
          }
          // else Configuration still exists
          if (reinterpret_cast<std::uintptr_t>(it->second.get()) != pidAndChangeCountAndID.configurationId)
          {
            // This Configuration is not the same one that was originally created by the CMBundleExtension. It must have been
            // removed and re-added by someone else in the interim.
            removed.push_back(false);
            continue;
          }
          try {
            const auto configurationWasRemoved = it->second->RemoveWithoutNotificationIfChangeCountEquals(pidAndChangeCountAndID.changeCount);
            if (configurationWasRemoved)
            {
              configurationsToInvalidate.push_back(std::move(it->second));
              removed.push_back(true);
              configurations.erase(it);
              RemoveFactoryInstanceIfRequired(pid);
              continue;
            }
            // else Configuration now differs from the one the CMBundleExtension added. Do not remove it.
            removed.push_back(false);
          }
          catch (const std::runtime_error&)
          {
            // Configuration already Removed by someone else, but we've won the race to handle that in ComponentAdminImpl
            configurationsToInvalidate.push_back(std::move(it->second));
            removed.push_back(true);
            configurations.erase(it);
            RemoveFactoryInstanceIfRequired(pid);
          }
        }
      }
      // This cannot be called whilst holding the configurationsMutex as it could cause a deadlock.
      for (auto& configurationToInvalidate : configurationsToInvalidate)
      {
        configurationToInvalidate->Invalidate();
      }
      auto idx = 0u;
      for (const auto& pidAndChangeCountAndID : pidsAndChangeCountsAndIDs)
      {
        const auto& pid = pidAndChangeCountAndID.pid;
        if (removed[idx])
        {
          NotifyConfigurationUpdated(pid);
          logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                      "RemoveConfigurations: Removed Configuration instance with PID " + pid);
        }
        else
        {
          logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                      "RemoveConfigurations: Configuration with PID " + pid + " was not removed"
                      " (either already removed, or it has been subsequently updated)");
        }
        ++idx;
      }
    }

    void ConfigurationAdminImpl::NotifyConfigurationUpdated(const std::string& pid)
    {
      PerformAsync([this, pid]
      {
        AnyMap properties{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS};
        auto removed = false;
        {
          std::lock_guard<std::mutex> lk{configurationsMutex};
          const auto it = configurations.find(pid);
          if (it == std::end(configurations))
          {
            removed = true;
          }
          else
          {
            try
            {
              properties = it->second->GetProperties();
            }
            catch (const std::runtime_error&)
            {
              // Configuration is being removed
              removed = true;
            }
          }
        }
        const auto managedServiceWrappers = managedServiceTracker.GetServices();
        const auto it = std::find_if(std::begin(managedServiceWrappers), std::end(managedServiceWrappers),
                                     [&pid](const auto& managedServiceWrapper)
                                     {
                                       // The ServiceTracker will return a default constructed shared_ptr for each ManagedService
                                       // that we aren't tracking. We must be careful not to dereference these!
                                       return (managedServiceWrapper ? (pid == managedServiceWrapper->pid) : false);
                                     });
        if (it != std::end(managedServiceWrappers))
        {
          const auto &managedServiceWrapper = *it;
          notifyServiceUpdated(pid, *(managedServiceWrapper->trackedService), properties, *logger);
        }
        const auto factoryPid = getFactoryPid(pid);
        if (factoryPid.empty())
        {
          return;
        }
        const auto managedServiceFactoryWrappers = managedServiceFactoryTracker.GetServices();
        const auto factoryIt = std::find_if(std::begin(managedServiceFactoryWrappers), std::end(managedServiceFactoryWrappers),
                                            [&factoryPid](const auto& managedServiceFactoryWrapper)
                                            {
                                              // The ServiceTracker will return a default constructed shared_ptr for each ManagedServiceFactory
                                              // that we aren't tracking. We must be careful not to dereference these!
                                              return (managedServiceFactoryWrapper ? (factoryPid == managedServiceFactoryWrapper->pid) : false);
                                            });
        if (factoryIt != std::end(managedServiceFactoryWrappers))
        {
          const auto &managedServiceFactoryWrapper = *factoryIt;
          if (removed)
          {
            notifyServiceRemoved(pid, *(managedServiceFactoryWrapper->trackedService), *logger);
          }
          else
          {
            notifyServiceUpdated(pid, *(managedServiceFactoryWrapper->trackedService), properties, *logger);
          }
        }
      });
    }

    void ConfigurationAdminImpl::NotifyConfigurationRemoved(const std::string &pid, std::uintptr_t configurationId)
    {
      std::shared_ptr<ConfigurationImpl> configurationToInvalidate;
      {
        std::lock_guard<std::mutex> lk{configurationsMutex};
        auto it = configurations.find(pid);
        if (it == std::end(configurations))
        {
          // This Configuration has already been removed. The thread which removed it will have triggered
          // the notification of any ManagedService or ManagedServiceFactory, so nothing more to do.
          return;
        }
        if (configurationId != reinterpret_cast<std::uintptr_t>(it->second.get()))
        {
          // The Configuration with this PID has already been removed and replaced by a new one, and the thread
          // which did that will have triggered the notification of any ManagedService or ManagedServiceFactory,
          // so nothing more to do.
          return;
        }
        configurationToInvalidate = it->second;
        configurations.erase(it);
        RemoveFactoryInstanceIfRequired(pid);
      }
      if (configurationToInvalidate)
      {
        NotifyConfigurationUpdated(pid);
        // This functor will run on another thread. Just being overly cautious to guarantee that the
        // ConfigurationImpl which has called this method doesn't run its own destructor.
        PerformAsync([this, pid, configuration = std::move(configurationToInvalidate)]
        {
          logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                      "Configuration with PID " + pid + " has been removed.");
        });
      }
    }

    std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>
    ConfigurationAdminImpl::AddingService(const ServiceReference<cppmicroservices::service::cm::ManagedService>& reference)
    {
      const auto pid = getPidFromServiceReference(reference);
      if (pid.empty())
      {
        const auto bundle = reference.GetBundle();
        const auto serviceID = std::to_string(cppmicroservices::any_cast<long>(reference.GetProperty(Constants::SERVICE_ID)));
        logger->Log(SeverityLevel::LOG_WARNING, "Ignoring ManagedService with ID " + serviceID + " from bundle "
                                              + (bundle ? bundle.GetSymbolicName() : "Unknown")
                                              + " as it does not have a service.pid or component.name property");
        return nullptr;
      }
      auto managedService = cmContext.GetService(reference);
      if (!managedService)
      {
        logger->Log(SeverityLevel::LOG_DEBUG, "Ignoring ManagedService as a valid service could not be obtained from the BundleContext");
        return nullptr;
      }
      // Ensure there's a Configuration for this PID if one doesn't exist already.
      {
        std::lock_guard<std::mutex> lk{configurationsMutex};
        const auto it = configurations.find(pid);
        if (it == std::end(configurations)) {
            auto factoryPid = getFactoryPid(pid);
            AddFactoryInstanceIfRequired(pid, factoryPid);
            configurations.emplace(pid, std::make_shared<ConfigurationImpl>(this, pid, std::move(factoryPid), AnyMap{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS}));
        }
      }
      PerformAsync([this, pid, managedService]
      {
        AnyMap properties{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS};
        {
          std::lock_guard<std::mutex> lk{configurationsMutex};
          const auto it = configurations.find(pid);
          if (it != std::end(configurations))
          {
            try
            {
              properties = it->second->GetProperties();
            }
            catch (const std::runtime_error&)
            {
              // Configuration is being removed
            }
          }
        }
        notifyServiceUpdated(pid, *managedService, properties, *logger);
      });
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                  "New ManagedService with PID " + pid + " has been added, and async Update has been queued.");
      return std::make_shared<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>(pid, std::move(managedService));
    }

    void ConfigurationAdminImpl::ModifiedService(const ServiceReference<cppmicroservices::service::cm::ManagedService>& /* reference */,
                                                 const std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>& /* service */)
    {
      // Assume the PID and component.name properties will never change, so noop
    }

    void ConfigurationAdminImpl::RemovedService(const ServiceReference<cppmicroservices::service::cm::ManagedService>& /* reference */,
                                                const std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>& service)
    {
      // No need to do anything other than log; ManagedService just won't receive any more updates to its Configuration.
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                  "ManagedService with PID " + service->pid + " has been removed.");
    }

    std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>>
    ConfigurationAdminImpl::AddingService(const ServiceReference<cppmicroservices::service::cm::ManagedServiceFactory>& reference)
    {
      const auto pid = getPidFromServiceReference(reference);
      if (pid.empty())
      {
        const auto bundle = reference.GetBundle();
        const auto serviceID = std::to_string(cppmicroservices::any_cast<long>(reference.GetProperty(Constants::SERVICE_ID)));
        logger->Log(SeverityLevel::LOG_WARNING, "Ignoring ManagedServiceFactory with ID " + serviceID + " from bundle "
                                              + (bundle ? bundle.GetSymbolicName() : "Unknown")
                                              + " as it does not have a service.pid or component.name property");
        return nullptr;
      }
      auto managedServiceFactory = cmContext.GetService(reference);
      if (!managedServiceFactory)
      {
        logger->Log(SeverityLevel::LOG_DEBUG, "Ignoring ManagedServiceFactory as a valid service could not be obtained from the BundleContext");
        return nullptr;
      }
      PerformAsync([this, pid, managedServiceFactory]
      {
        std::vector<std::pair<std::string, AnyMap>> pidsAndProperties;
        {
          std::lock_guard<std::mutex> lk{configurationsMutex};
          const auto it = factoryInstances.find(pid);
          if (it != std::end(factoryInstances))
          {
            for (const auto &instance : it->second)
            {
              const auto configurationIt = configurations.find(instance);
              assert(configurationIt != std::end(configurations) && "Invalid Configuration iterator");
              try
              {
                auto properties = configurationIt->second->GetProperties();
                pidsAndProperties.emplace_back(instance, std::move(properties));
              }
              catch (const std::runtime_error&)
              {
                // Configuration is being removed
              }
            }
          }
        }
        for (const auto &pidAndProperties : pidsAndProperties)
        {
          notifyServiceUpdated(pidAndProperties.first, *managedServiceFactory, pidAndProperties.second, *logger);
        }
      });
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                  "New ManagedServiceFactory with PID " + pid + " has been added, and async Update has been queued for all instances.");
      return std::make_shared<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>>(pid, std::move(managedServiceFactory));
    }

    void ConfigurationAdminImpl::ModifiedService(const ServiceReference<cppmicroservices::service::cm::ManagedServiceFactory>& /* reference */,
                                                 const std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>>& /* service */)
    {
      // Assume the PID and component.name properties will never change, so noop
    }

    void ConfigurationAdminImpl::RemovedService(const ServiceReference<cppmicroservices::service::cm::ManagedServiceFactory>& /* reference */,
                                                const std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>>& service)
    {
      // No need to do anything other than log; ManagedServiceFactory just won't receive any more updates to any of its Configurations.
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                  "ManagedServiceFactory with PID " + service->pid + " has been removed.");
    }

    template <typename Functor>
    void ConfigurationAdminImpl::PerformAsync(Functor&& f)
    {
       std::lock_guard<std::mutex> lk{futuresMutex};
       decltype(completeFutures){}.swap(completeFutures);
       auto id = ++futuresID;
       incompleteFutures.emplace(id, std::async(std::launch::async, [this, func = std::forward<Functor>(f), id]
       {
         func();
         std::lock_guard<std::mutex> lk{futuresMutex};
         auto it = incompleteFutures.find(id);
         assert(it != std::end(incompleteFutures) && "Invalid future iterator");
         completeFutures.push_back(std::move(it->second));
         incompleteFutures.erase(it);
         if (incompleteFutures.empty())
         {
           futuresCV.notify_one();
         }
       }));
    }

    std::string ConfigurationAdminImpl::RandomInstanceName()
    {
      static const auto characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
      std::uniform_int_distribution<> dist(0, 61);
      std::string randomString(6, '\0');
      for(auto& c: randomString)
      {
        c = characters[dist(randomGenerator)];
      }
      return randomString;
    }

    void ConfigurationAdminImpl::AddFactoryInstanceIfRequired(const std::string& pid, const std::string &factoryPid)
    {
      if (factoryPid.empty())
      {
        return;
      }
      auto it = factoryInstances.find(factoryPid);
      if (it == std::end(factoryInstances))
      {
        it = factoryInstances.emplace(factoryPid, std::set<std::string>{}).first;
      }
      it->second.insert(pid);
    }

    void ConfigurationAdminImpl::RemoveFactoryInstanceIfRequired(const std::string& pid)
    {
      const auto factoryPid = getFactoryPid(pid);
      if (factoryPid.empty())
      {
        return;
      }
      auto it = factoryInstances.find(factoryPid);
      if (it != std::end(factoryInstances))
      {
        it->second.erase(pid);
        if (it->second.empty())
        {
          factoryInstances.erase(it);
        }
      }
    }

    void ConfigurationAdminImpl::WaitForAllAsync()
    {
      std::unique_lock<std::mutex> ul{futuresMutex};
      if (!incompleteFutures.empty())
      {
        futuresCV.wait(ul, [this] { return incompleteFutures.empty(); });
      }
    }
  } // cmimpl
} // cppmicroservices
