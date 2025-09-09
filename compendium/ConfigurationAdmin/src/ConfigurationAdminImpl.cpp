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
#include <memory>
#include <stdexcept>
#include <thread>

#include "cppmicroservices/Constants.h"
#include "cppmicroservices/cm/ConfigurationException.hpp"
#include "cppmicroservices/detail/ScopeGuard.h"

#include "CMConstants.hpp"
#include "ConfigurationAdminImpl.hpp"
#include "SingleInvokeTask.hpp"

using cppmicroservices::logservice::SeverityLevel;

namespace
{
    std::string
    getFactoryPid(std::string const& pid)
    {
        auto const pos = pid.find_first_of('~');
        if (pos == std::string::npos)
        {
            return {};
        }
        return pid.substr(0, pos);
    }

    void
    handleUpdatedException(std::string const& pid,
                           cppmicroservices::AnyMap const& properties,
                           cppmicroservices::logservice::LogService& logger,
                           bool isFactory)
    {
        auto thrownByMessage = "thrown by ManagedService" + (isFactory ? std::string("Factory") : "");
        thrownByMessage += " with PID " + pid + " whilst being Updated with new properties.\n\t";
        try
        {
            throw;
        }
        catch (cppmicroservices::service::cm::ConfigurationException const& ce)
        {
            auto const& property = ce.GetProperty();
            std::string propertyCausingError;
            if (!property.empty())
            {
                propertyCausingError
                    += "The property which caused this error was '" + property + "' which had the value: \n\t";
                propertyCausingError += properties.AtCompoundKey(property, cppmicroservices::Any()).ToStringNoExcept();
            }
            else
            {
                propertyCausingError += "It was not specified which property caused this error.";
            }
            logger.Log(SeverityLevel::LOG_ERROR,
                       "ConfigurationException " + thrownByMessage + "Exception reason: " + ce.GetReason() + "\n\t"
                           + propertyCausingError);
        }
        catch (std::exception const& e)
        {
            logger.Log(SeverityLevel::LOG_ERROR, "Exception " + thrownByMessage + "Exception: " + e.what());
        }
        catch (...)
        {
            logger.Log(SeverityLevel::LOG_ERROR, "Unknown exception " + thrownByMessage);
        }
    }

    void
    notifyServiceUpdated(std::string const& pid,
                         cppmicroservices::service::cm::ManagedService& managedService,
                         cppmicroservices::AnyMap const& properties,
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

    void
    notifyServiceUpdated(std::string const& pid,
                         cppmicroservices::service::cm::ManagedServiceFactory& managedServiceFactory,
                         cppmicroservices::AnyMap const& properties,
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

    void
    notifyServiceRemoved(std::string const& pid,
                         cppmicroservices::service::cm::ManagedServiceFactory& managedServiceFactory,
                         cppmicroservices::logservice::LogService& logger)
    {
        try
        {
            managedServiceFactory.Removed(pid);
        }
        catch (std::exception const& e)
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
    std::string
    getPidFromServiceReference(T const& reference)
    {
        using namespace cppmicroservices::cmimpl::CMConstants;
        try
        {
            auto const serviceProp = reference.GetProperty(CM_SERVICE_KEY);
            auto const& serviceMap = cppmicroservices::ref_any_cast<cppmicroservices::AnyMap>(serviceProp);
            return cppmicroservices::any_cast<std::string>(serviceMap.AtCompoundKey(CM_SERVICE_SUBKEY));
        }
        catch (...)
        {
            // Service does not have a "service" property with a "pid" string subproperty.
        }
        try
        {
            auto const componentProp = reference.GetProperty(CM_COMPONENT_KEY);
            auto const& componentMap = cppmicroservices::ref_any_cast<cppmicroservices::AnyMap>(componentProp);
            return cppmicroservices::any_cast<std::string>(componentMap.AtCompoundKey(CM_COMPONENT_SUBKEY));
        }
        catch (...)
        {
            // Service does not have a "component" property with a "name" string property.
        }
        try
        {
            return cppmicroservices::any_cast<std::string>(
                reference.GetProperty(CM_SERVICE_KEY + std::string(".") + CM_SERVICE_SUBKEY));
        }
        catch (...)
        {
            // Service does not have a "service.pid" string property.
        }
        try
        {
            return cppmicroservices::any_cast<std::string>(
                reference.GetProperty(CM_COMPONENT_KEY + std::string(".") + CM_COMPONENT_SUBKEY));
        }
        catch (...)
        {
            // Service does not have a "component.name" string property.
        }
        return {};
    }
} // namespace

namespace cppmicroservices
{
    namespace cmimpl
    {
        ConfigurationAdminImpl::ConfigurationAdminImpl(
            cppmicroservices::BundleContext context,
            std::shared_ptr<cppmicroservices::logservice::LogService> const& lggr,
            std::shared_ptr<cppmicroservices::async::AsyncWorkService> const& asyncWS)
            : cmContext(std::move(context))
            , logger(lggr)
            , asyncWorkService(asyncWS)
            , futuresID { 0u }
            , managedServiceTracker(cmContext, this)
            , managedServiceFactoryTracker(cmContext, this)
            , configListenerTracker(cmContext)
        {
            managedServiceTracker.Open();
            managedServiceFactoryTracker.Open();
            configListenerTracker.Open();
        }

        ConfigurationAdminImpl::~ConfigurationAdminImpl()
        {
            auto managedServiceWrappers = managedServiceTracker.GetServices();
            auto managedServiceFactoryWrappers = managedServiceFactoryTracker.GetServices();

            try
            {
                managedServiceFactoryTracker.Close();
                managedServiceTracker.Close();
                configListenerTracker.Close();
            }
            catch (...)
            {
                auto thrownByMessage = "thrown by ConfiguratonAdminImpl destructor "
                                       "while closing the service trackers.\n\t ";
                logger->Log(SeverityLevel::LOG_ERROR, thrownByMessage);
            }

            decltype(factoryInstances) factoryInstancesCopy;
            decltype(configurations) configurationsToInvalidate;
            {
                std::lock_guard<std::mutex> lk { configurationsMutex };
                factoryInstancesCopy.swap(factoryInstances);
                configurationsToInvalidate.swap(configurations);
            }
            for (auto const& configuration : configurationsToInvalidate)
            {
                configuration.second->Invalidate();
            }
            AnyMap emptyMap { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS };
            for (auto const& managedService : managedServiceWrappers)
            {
                // The ServiceTracker will return a default constructed shared_ptr for each ManagedService
                // that we aren't tracking. We must be careful not to dereference these!
                if (managedService)
                {
                    // can only send a Removed notification if the managedService has previously
                    // received an Updated notification.
                    auto const it = configurationsToInvalidate.find(managedService->getPid());
                    if (it != std::end(configurationsToInvalidate) && it->second->HasBeenUpdatedAtLeastOnce())
                    {
                        notifyServiceUpdated(managedService->getPid(),
                                             *(managedService->getTrackedService()),
                                             emptyMap,
                                             *logger);
                    }
                }
            }
            for (auto const& managedServiceFactory : managedServiceFactoryWrappers)
            {
                // The ServiceTracker will return a default constructed shared_ptr for each ManagedServiceFactory
                // that we aren't tracking. We must be careful not to dereference these!
                if (!managedServiceFactory)
                {
                    continue;
                }
                auto it = factoryInstancesCopy.find(managedServiceFactory->getPid());
                if (it == std::end(factoryInstancesCopy))
                {
                    continue;
                }
                for (auto const& pid : it->second)
                {
                    // can only send a Removed notification if the managedServiceFactory has previously
                    // received an Updated notification.

                    auto const it = configurationsToInvalidate.find(pid);
                    if (it != std::end(configurationsToInvalidate) && (it->second->HasBeenUpdatedAtLeastOnce()))
                    {
                        notifyServiceRemoved(pid, *(managedServiceFactory->getTrackedService()), *logger);
                    }
                }
            }
            std::unique_lock<std::mutex> ul { futuresMutex };
            if (!incompleteFutures.empty())
            {
                futuresCV.wait(ul, [this] { return incompleteFutures.empty(); });
            }
        }

        std::shared_ptr<cppmicroservices::service::cm::Configuration>
        ConfigurationAdminImpl::GetConfiguration(std::string const& pid)
        {
            std::shared_ptr<cppmicroservices::service::cm::Configuration> result;
            auto created = false;
            {
                std::lock_guard<std::mutex> lk { configurationsMutex };
                auto it = configurations.find(pid);
                if (it == std::end(configurations))
                {
                    auto factoryPid = getFactoryPid(pid);
                    AddFactoryInstanceIfRequired(pid, factoryPid);
                    it = configurations
                             .emplace(pid,
                                      std::make_shared<ConfigurationImpl>(
                                          this,
                                          pid,
                                          std::move(factoryPid),
                                          AnyMap { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS }))
                             .first;
                    created = true;
                }
                result = it->second;
            }
            logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                        "GetConfiguration: returning " + (created ? std::string("new") : "existing")
                            + " Configuration instance with PID " + pid);
            return result;
        }

        std::shared_ptr<cppmicroservices::service::cm::Configuration>
        ConfigurationAdminImpl::CreateFactoryConfiguration(std::string const& factoryPid)
        {
            std::shared_ptr<cppmicroservices::service::cm::Configuration> result;
            auto pid = factoryPid + "~" + RandomInstanceName();
            {
                std::lock_guard<std::mutex> lk { configurationsMutex };
                auto it = configurations.find(pid);
                while (it != std::end(configurations))
                {
                    pid = factoryPid + "~" + RandomInstanceName();
                    it = configurations.find(pid);
                }
                AddFactoryInstanceIfRequired(pid, factoryPid);
                it = configurations
                         .emplace(
                             pid,
                             std::make_shared<ConfigurationImpl>(this,
                                                                 pid,
                                                                 factoryPid,
                                                                 AnyMap { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS }))
                         .first;
                result = it->second;
            }
            logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                        "CreateFactoryConfiguration: returning new Configuration "
                        "instance with PID "
                            + pid);
            return result;
        }

        std::shared_ptr<cppmicroservices::service::cm::Configuration>
        ConfigurationAdminImpl::GetFactoryConfiguration(std::string const& factoryPid, std::string const& instanceName)
        {
            auto const pid = factoryPid + "~" + instanceName;
            logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                        "GetFactoryConfiguration: deferring to GetConfiguration for PID " + pid);
            return GetConfiguration(pid);
        }
        /* ListConfigurations looks for configuration objects in the repository that match the
         * input parameter. An LDAP filter expression can be used to filter based on any property of the
         * configuration, including the pid or factory pid. Typical inputs for a filter string might
         * be "pid=startup.options", (to search for a configuration object with a matching pid)
         * "pid=virtualfilesystem~user1", (to search for a configuration object with a matching factory pid)
         *  "key1=abc" (to search for a configuration object containing a property with key "key1" with a value "abc".
         * Regular expressions are allowed.
         */
        std::vector<std::shared_ptr<cppmicroservices::service::cm::Configuration>>
        ConfigurationAdminImpl::ListConfigurations(std::string const& filter)
        {
            std::vector<std::shared_ptr<cppmicroservices::service::cm::Configuration>> result;
            {
                std::lock_guard<std::mutex> lk { configurationsMutex };

                // return all configuration objects if the filter is empty
                if (filter.empty())
                {
                    result.reserve(configurations.size());
                    for (auto const& it : configurations)
                    {
                        if (it.second->HasBeenUpdatedAtLeastOnce())
                        {
                            result.push_back(it.second);
                        }
                    }
                    return result;
                }

                // filter is not empty so look for pid and property matches
                LDAPFilter ldap { filter };
                cppmicroservices::AnyMap pidMap { cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS };

                for (auto const& it : configurations)
                {
                    // configurations that have not yet been updated cannot be
                    // returned.
                    if (!it.second->HasBeenUpdatedAtLeastOnce())
                    {
                        continue;
                    }
                    /* Create an AnyMap containing the pid or factoryPid so that the ldap filter
                     * functionality can be used to match the pid to the
                     * input parameter. Easy way to do the comparison since input parameter could
                     * contain a regular expression
                     */
                    pidMap["pid"] = it.first;

                    if (ldap.Match(pidMap))
                    {
                        // This configuration object has a matching pid.
                        result.emplace_back(it.second);
                    }
                    else
                    {
                        // The pid wasn't a match but the properties might be. Check those.
                        auto props = it.second->GetProperties();
                        if (ldap.Match(props))
                        {
                            result.emplace_back(it.second);
                        }
                    }
                } // end for
            }
            return result;
        }

        std::vector<ConfigurationAddedInfo>
        ConfigurationAdminImpl::AddConfigurations(std::vector<metadata::ConfigurationMetadata> configurationMetadata)
        {
            std::vector<ConfigurationAddedInfo> pidsAndChangeCountsAndIDs;
            std::vector<bool> createdOrUpdated;
            std::vector<std::shared_ptr<ConfigurationImpl>> configurationsToInvalidate;

            {
                std::lock_guard<std::mutex> lk { configurationsMutex };
                for (auto const& configMetadata : configurationMetadata)
                {
                    unsigned long changeCount { 0ul };
                    auto& pid = configMetadata.pid;
                    auto it = configurations.find(pid);
                    if (it == std::end(configurations))
                    {
                        auto factoryPid = getFactoryPid(pid);
                        AddFactoryInstanceIfRequired(pid, factoryPid);
                        // construct the Configuration Object with a changeCount of 1
                        // since configuration objects created from metadata already
                        // have their properties (if any) present. The create operation
                        // counts as a create and an update operation.
                        auto newConfig = std::make_shared<ConfigurationImpl>(this,
                                                                             pid,
                                                                             std::move(factoryPid),
                                                                             configMetadata.properties,
                                                                             1u);
                        changeCount = newConfig->GetChangeCount();
                        it = configurations.emplace(pid, std::move(newConfig)).first;
                        pidsAndChangeCountsAndIDs.emplace_back(pid,
                                                               changeCount,
                                                               reinterpret_cast<std::uintptr_t>(it->second.get()));
                        createdOrUpdated.push_back(true);
                        continue;
                    }
                    // else Configuration already exists
                    try
                    {
                        auto const updatedAndChangeCount
                            = it->second->UpdateWithoutNotificationIfDifferent(configMetadata.properties);
                        changeCount = updatedAndChangeCount.second;
                        pidsAndChangeCountsAndIDs.emplace_back(pid,
                                                               std::get<1>(updatedAndChangeCount),
                                                               reinterpret_cast<std::uintptr_t>(it->second.get()));
                        createdOrUpdated.push_back(std::get<0>(updatedAndChangeCount));
                    }
                    catch (std::runtime_error const&) // Configuration has been Removed by someone else, but we've won
                                                      // the race to handle that.
                    {
                        configurationsToInvalidate.push_back(std::move(it->second));
                        it->second = std::make_shared<ConfigurationImpl>(this,
                                                                         pid,
                                                                         getFactoryPid(pid),
                                                                         configMetadata.properties);
                        pidsAndChangeCountsAndIDs.emplace_back(pid,
                                                               changeCount,
                                                               reinterpret_cast<std::uintptr_t>(it->second.get()));
                        createdOrUpdated.push_back(true);
                    }
                }
            }
            // This cannot be called whilst holding the configurationsMutex as it could cause a deadlock.
            for (auto const& configurationToInvalidate : configurationsToInvalidate)
            {
                configurationToInvalidate->Invalidate();
            }
            auto idx = 0u;
            for (auto const& pidAndChangeCountAndID : pidsAndChangeCountsAndIDs)
            {
                auto const& pid = pidAndChangeCountAndID.pid;
                if (createdOrUpdated[idx])
                {
                    NotifyConfigurationUpdated(pid, pidAndChangeCountAndID.changeCount);
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                                "AddConfigurations: Created or Updated Configuration instance with PID " + pid);
                }
                else
                {
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                                "AddConfigurations: Configuration already existed with identical properties with PID "
                                    + pid);
                }
                ++idx;
            }
            return pidsAndChangeCountsAndIDs;
        }

        void
        ConfigurationAdminImpl::RemoveConfigurations(std::vector<ConfigurationAddedInfo> pidsAndChangeCountsAndIDs)
        {
            std::vector<std::pair<bool, bool>> removedAndUpdated;
            std::vector<std::shared_ptr<ConfigurationImpl>> configurationsToInvalidate;
            bool hasBeenUpdated = false;
            {
                std::lock_guard<std::mutex> lk { configurationsMutex };
                for (auto const& pidAndChangeCountAndID : pidsAndChangeCountsAndIDs)
                {
                    auto const& pid = pidAndChangeCountAndID.pid;
                    auto const it = configurations.find(pid);
                    if (it == std::end(configurations))
                    {
                        removedAndUpdated.emplace_back(false, false);
                        continue;
                    }
                    // else Configuration still exists
                    if (reinterpret_cast<std::uintptr_t>(it->second.get()) != pidAndChangeCountAndID.configurationId)
                    {
                        // This Configuration is not the same one that was originally created by the CMBundleExtension.
                        // It must have been removed and re-added by someone else in the interim.
                        removedAndUpdated.emplace_back(false, false);
                        continue;
                    }
                    try
                    {
                        hasBeenUpdated = it->second->HasBeenUpdatedAtLeastOnce();
                        auto const configurationWasRemoved = it->second->RemoveWithoutNotificationIfChangeCountEquals(
                            pidAndChangeCountAndID.changeCount);
                        if (configurationWasRemoved)
                        {
                            configurationsToInvalidate.push_back(std::move(it->second));
                            removedAndUpdated.emplace_back(true, hasBeenUpdated);
                            configurations.erase(it);
                            RemoveFactoryInstanceIfRequired(pid);
                            continue;
                        }
                        // else Configuration now differs from the one the CMBundleExtension added. Do not remove it.
                        removedAndUpdated.emplace_back(false, false);
                    }
                    catch (std::runtime_error const&)
                    {
                        // Configuration already Removed by someone else, but we've won the race to handle that in
                        // ComponentAdminImpl
                        configurationsToInvalidate.push_back(std::move(it->second));
                        removedAndUpdated.emplace_back(true, hasBeenUpdated);
                        configurations.erase(it);
                        RemoveFactoryInstanceIfRequired(pid);
                    }
                }
            }
            // This cannot be called whilst holding the configurationsMutex as it could cause a deadlock.
            for (auto const& configurationToInvalidate : configurationsToInvalidate)
            {
                configurationToInvalidate->Invalidate();
            }
            auto idx = 0u;
            for (auto const& pidAndChangeCountAndID : pidsAndChangeCountsAndIDs)
            {
                auto const& pid = pidAndChangeCountAndID.pid;
                if (removedAndUpdated[idx].first)
                {
                    if (removedAndUpdated[idx].second)
                    {
                        NotifyConfigurationUpdated(pid, pidAndChangeCountAndID.changeCount);
                    }
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                                "RemoveConfigurations: Removed Configuration instance with PID " + pid);
                }
                else
                {
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                                "RemoveConfigurations: Configuration with PID " + pid
                                    + " was not removed (either already removed, or it has been subsequently updated)");
                }
                ++idx;
            }
        }

        std::shared_ptr<ThreadpoolSafeFuturePrivate>
        ConfigurationAdminImpl::NotifyConfigurationUpdated(std::string const& pid, unsigned long const changeCount)
        {
            // NotifyConfigurationUpdated will only send a notification to the service if
            // the configuration object has been updated at least once. In order to determine whether or not
            // a configuration object has been updated, it calls the HasBeenUpdatedAtLeastOnce method for
            // the configuration object. For a remove operation the configuration object
            // is not available and that method cannot be called. For this reason, NotifyConfigurationUpdated
            // should not be called for Remove operations unless the caller has already confirmed
            // the configuration object has been updated at least once.
            return PerformAsync(
                [this, pid, changeCount]
                {
                    AnyMap properties { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS };
                    std::string fPid;
                    std::string nonFPid;
                    auto removed = false;
                    auto hasBeenUpdated = false;
                    std::vector<std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>>
                        managedServiceWrappers;
                    std::vector<
                        std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>>>
                        managedServiceFactoryWrappers;
                    {
                        std::lock_guard<std::mutex> lk { configurationsMutex };
                        auto const it = configurations.find(pid);
                        if (it == std::end(configurations))
                        {
                            removed = true;
                            hasBeenUpdated = true;
                        }
                        else
                        {
                            try
                            {
                                hasBeenUpdated = it->second->HasBeenUpdatedAtLeastOnce();
                                properties = it->second->GetProperties();
                            }
                            catch (std::runtime_error const&)
                            {
                                // Configuration is being removed
                                removed = true;
                            }
                        }

                        // We can only send update notifications for configuration objects that have
                        // been updated. Just return without sending the notification for objects
                        // that have not yet been updated.
                        if (!hasBeenUpdated)
                        {
                            return;
                        }

                        if (pid.find('~') != std::string::npos)
                        {
                            // this is a factory pid
                            fPid = pid;
                        }
                        else
                        {
                            nonFPid = pid;
                        }
                        managedServiceWrappers = trackedManagedServices_;
                        managedServiceFactoryWrappers = trackedManagedServiceFactories_;
                    }

                    auto type = removed ? cppmicroservices::service::cm::ConfigurationEventType::CM_DELETED
                                        : cppmicroservices::service::cm::ConfigurationEventType::CM_UPDATED;

                    auto configurationListeners = configListenerTracker.GetServices();
                    auto configAdminRef = cmContext.GetServiceReference<ConfigurationAdmin>();
                    for (auto const& it : configurationListeners)
                    {
                        auto configEvent
                            = cppmicroservices::service::cm::ConfigurationEvent(configAdminRef, type, fPid, nonFPid);
                        it->configurationEvent((configEvent));
                    }

                    std::for_each(
                        managedServiceWrappers.begin(),
                        managedServiceWrappers.end(),
                        [&](auto const& managedServiceWrapper)
                        {
                            // The ServiceTracker will return a default constructed shared_ptr for each ManagedService
                            // that we aren't tracking. We must be careful not to dereference these!
                            if ((managedServiceWrapper) && (managedServiceWrapper->getPid() == pid)
                                && (removed
                                    || (!removed
                                        && managedServiceWrapper->needsAnUpdateNotification(pid, changeCount))))
                            {
                                notifyServiceUpdated(pid,
                                                     *(managedServiceWrapper->getTrackedService()),
                                                     properties,
                                                     *logger);
                                if (removed)
                                {
                                    managedServiceWrapper->removeLastUpdatedChangeCount(pid);
                                }
                                else
                                {
                                    managedServiceWrapper->setLastUpdatedChangeCount(pid, changeCount);
                                }
                            }
                        });

                    if (auto const factoryPid = getFactoryPid(pid); !factoryPid.empty())
                    {
                        std::for_each(
                            managedServiceFactoryWrappers.begin(),
                            managedServiceFactoryWrappers.end(),
                            [&](auto const& managedServiceFactoryWrapper)
                            {
                                // The ServiceTracker will return a default constructed shared_ptr for each
                                // ManagedServiceFactory that we aren't tracking. We must be careful not to dereference
                                // these!
                                if ((managedServiceFactoryWrapper)
                                    && (managedServiceFactoryWrapper->getPid() == factoryPid))
                                {
                                    if (removed)
                                    {
                                        notifyServiceRemoved(pid,
                                                             *(managedServiceFactoryWrapper->getTrackedService()),
                                                             *logger);
                                        managedServiceFactoryWrapper->removeLastUpdatedChangeCount(pid);
                                    }
                                    else if (managedServiceFactoryWrapper->needsAnUpdateNotification(pid, changeCount))
                                    {
                                        notifyServiceUpdated(pid,
                                                             *(managedServiceFactoryWrapper->getTrackedService()),
                                                             properties,
                                                             *logger);
                                        managedServiceFactoryWrapper->setLastUpdatedChangeCount(pid, changeCount);
                                    }
                                }
                            });
                    }
                    if (!removed)
                    {
                        std::ostringstream configValue;
                        any_value_to_json(configValue, properties);

                        logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                                    "Configuration Updated: Configuration instance with PID " + pid + " version: "
                                        + std::to_string(changeCount) + " properties: " + configValue.str());
                    }
                });
        }

        std::shared_ptr<ThreadpoolSafeFuturePrivate>
        ConfigurationAdminImpl::NotifyConfigurationRemoved(std::string const& pid,
                                                           std::uintptr_t configurationId,
                                                           unsigned long changeCount)
        {
            std::promise<void> ready;
            auto alreadyRemoved = std::make_shared<ThreadpoolSafeFuturePrivate>(ready.get_future().share());
            std::shared_ptr<ConfigurationImpl> configurationToInvalidate;
            bool hasBeenUpdated = false;
            {
                std::lock_guard<std::mutex> lk { configurationsMutex };
                auto it = configurations.find(pid);
                if (it == std::end(configurations))
                {
                    // This Configuration has already been removed. The thread which removed it will have triggered
                    // the notification of any ManagedService or ManagedServiceFactory, so nothing more to do.
                    ready.set_value();
                    return alreadyRemoved;
                }
                if (configurationId != reinterpret_cast<std::uintptr_t>(it->second.get()))
                {
                    // The Configuration with this PID has already been removed and replaced by a new one, and the
                    // thread which did that will have triggered the notification of any ManagedService or
                    // ManagedServiceFactory, so nothing more to do.
                    ready.set_value();
                    return alreadyRemoved;
                }
                configurationToInvalidate = it->second;
                hasBeenUpdated = it->second->HasBeenUpdatedAtLeastOnce();
                configurations.erase(it);
                RemoveFactoryInstanceIfRequired(pid);
            }
            if (configurationToInvalidate && hasBeenUpdated)
            {
                auto removeFuture = NotifyConfigurationUpdated(pid, changeCount);
                // This functor will run on another thread. Just being overly cautious to guarantee that the
                // ConfigurationImpl which has called this method doesn't run its own destructor.
                PerformAsync(
                    [this, pid, configuration = std::move(configurationToInvalidate)]
                    {
                        logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                                    "Configuration with PID " + pid + " has been removed.");
                    });

                return removeFuture;
            }
            ready.set_value();
            return alreadyRemoved;
        }

        std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>
        ConfigurationAdminImpl::AddingService(
            ServiceReference<cppmicroservices::service::cm::ManagedService> const& reference)
        {
            // GetService can cause entry into user code, so don't hold a lock.
            auto managedService = cmContext.GetService(reference);
            if (!managedService)
            {
                logger->Log(SeverityLevel::LOG_WARNING,
                            "Ignoring ManagedService as a valid service could not be "
                            "obtained from the BundleContext");
                return nullptr;
            }

            // Lock the configurations repository so no configuration objects can be
            // added or removed while AddingService is processing the new service.
            std::lock_guard<std::mutex> lk { configurationsMutex };

            auto pid = getPidFromServiceReference(reference);
            if (pid.empty())
            {
                auto const bundle = reference.GetBundle();
                auto const serviceID
                    = std::to_string(cppmicroservices::any_cast<long>(reference.GetProperty(Constants::SERVICE_ID)));
                logger->Log(SeverityLevel::LOG_WARNING,
                            "Ignoring ManagedService with ID " + serviceID + " from bundle "
                                + (bundle ? bundle.GetSymbolicName() : "Unknown")
                                + " as it does not have a service.pid or component.name property");
                return nullptr;
            }

            // Send a notification in case a valid configuration object
            // was created before the service was active. The service's properties
            // need to be updated.
            unsigned long initialChangeCount { 0ul };

            if (auto const it = configurations.find(pid); it != std::end(configurations))
            {
                AnyMap properties { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS };
                // Only send notifications for configuration objects that have been
                // Updated.
                if (it->second->HasBeenUpdatedAtLeastOnce())
                {
                    try
                    {
                        properties = it->second->GetProperties();
                        // specifically set the initial change count here because there is
                        // a race between the call to GetChangeCount() and HasBeenUpdatedAtLeastOnce().
                        // The logic is that, if HasBeenUpdatedAtLeastOnce() returns true, the change
                        // count will always be > 1 and should at this point be captured for the
                        // tracked object to eliminate redundant config updates.
                        initialChangeCount = it->second->GetChangeCount();
                    }
                    catch (std::runtime_error const&)
                    {
                        // Configuration is being removed
                        logger->Log(SeverityLevel::LOG_WARNING,
                                    "Attempted to retrieve properties from a configuration "
                                    "which has been removed.",
                                    std::current_exception());
                    }

                    PerformAsync([this, pid, managedService, properties]
                                 { notifyServiceUpdated(pid, *managedService, properties, *logger); });

                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                                "Async configuration update queued for new ManagedService with PID " + pid + ".");
                }
            }

            logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                        "New ManagedService with PID " + pid + " has been added.");

            std::unordered_map<std::string, unsigned long> initialChangeCountByPid = {
                { pid, initialChangeCount }
            };
            auto trackedManagedService
                = std::make_shared<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>>(
                    pid,
                    std::move(initialChangeCountByPid),
                    std::move(managedService));
            trackedManagedServices_.emplace_back(trackedManagedService);
            return trackedManagedService;
        }

        void
        ConfigurationAdminImpl::ModifiedService(
            ServiceReference<cppmicroservices::service::cm::ManagedService> const& /* reference */,
            std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>> const& /* service */)
        {
            // Assume the PID and component.name properties will never change, so noop
        }

        void
        ConfigurationAdminImpl::RemovedService(
            ServiceReference<cppmicroservices::service::cm::ManagedService> const& /* reference */,
            std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedService>> const& service)
        {
            // Lock because we are modifying the container of tracked managed services.
            std::lock_guard<std::mutex> lk { configurationsMutex };

            auto elemIter = std::find_if(
                std::begin(trackedManagedServices_),
                std::end(trackedManagedServices_),
                [&service](auto const& trackedServiceWrapper)
                { return (service->getTrackedService() == trackedServiceWrapper->getTrackedService()); });
            if (elemIter != trackedManagedServices_.end())
            {
                trackedManagedServices_.erase(elemIter);
            }
            // ManagedService won't receive any more updates to its Configuration.
            logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                        "ManagedService with PID " + service->getPid() + " has been removed.");
        }

        std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>>
        ConfigurationAdminImpl::AddingService(
            ServiceReference<cppmicroservices::service::cm::ManagedServiceFactory> const& reference)
        {
            // GetService can cause entry into user code, so don't hold a lock.
            auto managedServiceFactory = cmContext.GetService(reference);
            if (!managedServiceFactory)
            {
                logger->Log(SeverityLevel::LOG_DEBUG,
                            "Ignoring ManagedServiceFactory as a valid service could not "
                            "be obtained from the BundleContext");
                return nullptr;
            }

            std::lock_guard<std::mutex> lk { configurationsMutex };

            auto pid = getPidFromServiceReference(reference);
            if (pid.empty())
            {
                auto const bundle = reference.GetBundle();
                auto const serviceID
                    = std::to_string(cppmicroservices::any_cast<long>(reference.GetProperty(Constants::SERVICE_ID)));
                logger->Log(SeverityLevel::LOG_WARNING,
                            "Ignoring ManagedServiceFactory with ID " + serviceID + " from bundle "
                                + (bundle ? bundle.GetSymbolicName() : "Unknown")
                                + " as it does not have a service.pid or component.name property");
                return nullptr;
            }

            std::vector<std::pair<std::string, AnyMap>> pidsAndProperties;
            std::unordered_map<std::string, unsigned long> initialChangeCountPerPid;

            auto const it = factoryInstances.find(pid);
            if (it != std::end(factoryInstances))
            {
                for (auto const& instance : it->second)
                {
                    auto const configurationIt = configurations.find(instance);
                    assert(configurationIt != std::end(configurations) && "Invalid Configuration iterator");
                    try
                    {
                        // Notifications can only be sent for configuration objects that
                        // been Updated. Only add it to the notification list if it has
                        // been Updated.
                        if (configurationIt->second->HasBeenUpdatedAtLeastOnce())
                        {
                            auto properties = configurationIt->second->GetProperties();
                            initialChangeCountPerPid[configurationIt->second->GetPid()]
                                = configurationIt->second->GetChangeCount();
                            pidsAndProperties.emplace_back(instance, std::move(properties));
                        }
                    }
                    catch (std::runtime_error const&)
                    {
                        // Configuration is being removed
                        logger->Log(SeverityLevel::LOG_WARNING,
                                    "Attempted to retrieve properties for a configuration "
                                    "which has been removed.",
                                    std::current_exception());
                    }
                }
            }

            if (pidsAndProperties.size() > 0)
            {
                PerformAsync(
                    [this, pid, managedServiceFactory, pidsAndProperties]
                    {
                        for (auto const& pidAndProperties : pidsAndProperties)
                        {
                            notifyServiceUpdated(pidAndProperties.first,
                                                 *managedServiceFactory,
                                                 pidAndProperties.second,
                                                 *logger);
                        }
                    });
            }

            logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                        "New ManagedServiceFactory with PID " + pid
                            + " has been added, and async Update has been queued for all updated instances.");
            auto trackedManagedServiceFactory
                = std::make_shared<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>>(
                    pid,
                    std::move(initialChangeCountPerPid),
                    std::move(managedServiceFactory));
            trackedManagedServiceFactories_.emplace_back(trackedManagedServiceFactory);
            return trackedManagedServiceFactory;
        }

        void
        ConfigurationAdminImpl::ModifiedService(
            ServiceReference<cppmicroservices::service::cm::ManagedServiceFactory> const& /* reference */,
            std::shared_ptr<
                TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>> const& /* service */)
        {
            // Assume the PID and component.name properties will never change, so noop
        }

        void
        ConfigurationAdminImpl::RemovedService(
            ServiceReference<cppmicroservices::service::cm::ManagedServiceFactory> const& /* reference */,
            std::shared_ptr<TrackedServiceWrapper<cppmicroservices::service::cm::ManagedServiceFactory>> const& service)
        {
            // Lock because we are modifying the container of tracked managed services.
            std::lock_guard<std::mutex> lk { configurationsMutex };

            auto elemIter = std::find_if(
                std::begin(trackedManagedServiceFactories_),
                std::end(trackedManagedServiceFactories_),
                [&service](auto const& trackedServiceWrapper)
                { return (service->getTrackedService() == trackedServiceWrapper->getTrackedService()); });
            if (elemIter != trackedManagedServiceFactories_.end())
            {
                trackedManagedServiceFactories_.erase(elemIter);
            }

            // ManagedServiceFactory won't receive any more updates to any of its Configurations.
            logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                        "ManagedServiceFactory with PID " + service->getPid() + " has been removed.");
        }

        using PostTask = std::packaged_task<void()>;

        template <typename Functor>
        std::shared_ptr<ThreadpoolSafeFuturePrivate>
        ConfigurationAdminImpl::PerformAsync(Functor&& f)
        {

            std::lock_guard<std::mutex> lk { futuresMutex };
            // if this configAdminImpl has been marked as inactive by the CMActivator, it should not queue any work
            if (!active)
            {
                // we return a default future which will always return immediately if waited on...
                // this isn't really a 'valid' future, but when the activator has stopped the bundle,
                // configAdmin can be assumed to be non functional
                return std::make_shared<ThreadpoolSafeFuturePrivate>();
            }
            decltype(completeFutures) {}.swap(completeFutures);
            auto id = ++futuresID;

            PostTask task(
                [this, func = std::forward<Functor>(f), id]() mutable
                {
                    // func() can throw, make sure that the futures
                    // are correctly cleaned up if an exception occurs.
                    detail::ScopeGuard cleanupFutures(
                        [this, id]()
                        {
                            std::lock_guard<std::mutex> lk { futuresMutex };
                            auto it = incompleteFutures.find(id);
                            assert(it != std::end(incompleteFutures) && "Invalid future iterator");
                            completeFutures.push_back(std::move(it->second));
                            incompleteFutures.erase(it);
                            if (incompleteFutures.empty())
                            {
                                futuresCV.notify_one();
                            }
                        });
                    func();
                });
            std::shared_future<void> fut = task.get_future().share();

            auto singleInvokeTask = std::make_shared<SingleInvokeTask>(std::make_shared<PostTask>(std::move(task)));

            PostTask post_task([singleInvokeTask]() mutable { (*singleInvokeTask)(); });

            incompleteFutures.emplace(id, fut);

            asyncWorkService->post(std::move(post_task));

            return std::make_shared<ThreadpoolSafeFuturePrivate>(fut, singleInvokeTask);
        }

        std::string
        ConfigurationAdminImpl::RandomInstanceName()
        {
            thread_local static std::mt19937 randomGenerator(std::random_device {}());
            static auto const characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
            std::uniform_int_distribution<> dist(0, 61);
            std::string randomString(6, '\0');
            for (auto& c : randomString)
            {
                c = characters[dist(randomGenerator)];
            }
            return randomString;
        }

        void
        ConfigurationAdminImpl::AddFactoryInstanceIfRequired(std::string const& pid, std::string const& factoryPid)
        {
            if (factoryPid.empty())
            {
                return;
            }
            auto it = factoryInstances.find(factoryPid);
            if (it == std::end(factoryInstances))
            {
                it = factoryInstances.emplace(factoryPid, std::set<std::string> {}).first;
            }
            it->second.insert(pid);
        }

        void
        ConfigurationAdminImpl::RemoveFactoryInstanceIfRequired(std::string const& pid)
        {
            auto const factoryPid = getFactoryPid(pid);
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

        void
        ConfigurationAdminImpl::WaitForAllAsync()
        {
            std::unique_lock<std::mutex> ul { futuresMutex };
            if (!incompleteFutures.empty())
            {
                futuresCV.wait(ul, [this] { return incompleteFutures.empty(); });
            }
        }
        void
        ConfigurationAdminImpl::StopAndWaitForAllAsync()
        {
            {
                std::lock_guard<std::mutex> lk { futuresMutex };
                active = false;
            }
            WaitForAllAsync();
        }
    } // namespace cmimpl
} // namespace cppmicroservices
