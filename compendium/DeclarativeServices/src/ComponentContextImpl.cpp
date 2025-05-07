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

#include "ComponentContextImpl.hpp"
#include "ComponentRegistry.hpp"
#include "ServiceReferenceComparator.hpp"
#include "manager/ComponentConfiguration.hpp"
#include "manager/ComponentConfigurationImpl.hpp"
#include "manager/ReferenceManager.hpp"
#include "manager/RegistrationManager.hpp"

#include <cassert>
#include <cppmicroservices/ServiceObjects.h>
#include <cppmicroservices/servicecomponent/ComponentException.hpp>

using cppmicroservices::ServiceReferenceBase;
using cppmicroservices::service::component::ComponentException;

namespace cppmicroservices
{
    namespace scrimpl
    {

        ComponentContextImpl::ComponentContextImpl(std::weak_ptr<ComponentConfiguration> cm)
            : configManager(std::move(cm))
            , usingBundle(Bundle())
        {
            InitializeServicesCache();
        }

        ComponentContextImpl::ComponentContextImpl(std::weak_ptr<ComponentConfiguration> cm,
                                                   cppmicroservices::Bundle usingBundle)
            : configManager(std::move(cm))
            , usingBundle(std::move(usingBundle))
        {
            InitializeServicesCache();
        }

        void
        ComponentContextImpl::InitializeServicesCache()
        {
            auto const configManagerPtr = configManager.lock();
            if (!configManagerPtr)
            {
                throw ComponentException("Context is invalid");
            }

            auto const refManagers = configManagerPtr->GetAllDependencyManagers();
            for (auto const& refManager : refManagers)
            {
                auto const& sRefs = refManager->GetBoundReferences();
                auto const& refName = refManager->GetReferenceName();
                bool foundAtLeastOneValidBoundService = false;
                std::for_each(sRefs.rbegin(),
                              sRefs.rend(),
                              [&](cppmicroservices::ServiceReferenceBase const& sRef)
                              {
                                  if (sRef)
                                  {
                                      ServiceReferenceU sRefU(sRef);
                                      auto bc = GetBundleContext();
                                      auto boundServicesCacheHandle = boundServicesCache.lock();
                                      auto& serviceMap = (*boundServicesCacheHandle)[refName];
                                      // get us the service instance
                                      cppmicroservices::ServiceObjects<void> sObjs = bc.GetServiceObjects(sRefU);
                                      auto interfaceMap = sObjs.GetService();
                                      if (interfaceMap && interfaceMap->size() != 0)
                                      {
                                          foundAtLeastOneValidBoundService = true;
                                          serviceMap.emplace_back(std::make_pair(sRef, interfaceMap));
                                      }
                                  }
                              });

                // Check that all the refernece managers have a valid bound service reference if one
                // is manodatory.
                // If this check fails, it's usually because a dependent service was unregistered
                // while the service object was being retrieved to add to the container of bound services.
                if (!refManager->IsOptional() && !foundAtLeastOneValidBoundService)
                {
                    throw ComponentException(
                        "Failed to construct a component context for " + configManagerPtr->GetMetadata()->implClassName
                        + ". No services were found which satisfy the mandatory service dependency " + refName
                        + ". This typically occurs because the dependent service was unregistered before it could be "
                          "used.");
                }
            }
        }

        std::unordered_map<std::string, cppmicroservices::Any>
        ComponentContextImpl::GetProperties() const
        {
            auto const configManagerPtr = configManager.lock();
            if (!configManagerPtr)
            {
                throw ComponentException("Context is invalid");
            }
            return configManagerPtr->GetProperties();
        }

        bool
        ServiceReferenceTargetIsMandatory(std::shared_ptr<ComponentConfiguration> const& configManagerPtr,
                                          std::string const& svcRefTargetName)
        {
            auto metadata = configManagerPtr->GetMetadata();
            for (auto const& _data : metadata->refsMetadata)
            {
                if (auto cardinality = _data.cardinality; _data.name == svcRefTargetName)
                {
                    return (cardinality.empty()) ? true : (cardinality.find("1..") != std::string::npos);
                }
            }

            return true;
        }

        /**
         * The ServiceInterfacePointerLookupInfo struct is used to pass back
         * the shared_ptr from GetInterfacePointer in addition to a boolean
         * which is used to determine whether or not the pointer was retrieved
         * from the map or if the map is null.
         *
         * wasFoundInMapOrMapEmpty:
         *   - true, if the service pointer was in the map
         *       or if the map if null (service can be either
         *       valid or a nullptr)
         *    - false if the service pointer was not found
         *       in the map (implies service is nullptr)
         */
        struct ServiceInterfacePointerLookupInfo
        {
            std::shared_ptr<void> service = nullptr;
            bool wasFoundInMapOrMapEmpty = false;

            bool
            IsValid() const
            {
                return static_cast<bool>(service) == wasFoundInMapOrMapEmpty;
            }
        };

        /**
         * Returns a ServiceInterfacePointerLookup struct containing the service interface
         * pointer from the InterfaceMapConstPtr given the interfaceid as the key.
         *
         * This function has a similar implementation to ExtractService except it returns
         * an instance of the ServiceInterfacePointerLookupInfo struct to do error handling
         * if necessary.
         *
         * - map, a InterfaceMap instance.
         * - interfaceId, The interface id string.
         * - return value: ServiceInterfacePointerLookupInfo
         */
        ServiceInterfacePointerLookupInfo
        GetInterfacePointer(InterfaceMapConstPtr const& map, std::string const& interfaceId)
        {
            ServiceInterfacePointerLookupInfo lookupInfo {};

            if (!map)
            {
                lookupInfo.wasFoundInMapOrMapEmpty = true;
                lookupInfo.service = nullptr;
                return lookupInfo;
            }

            if (interfaceId.empty() && map && !map->empty())
            {
                lookupInfo.wasFoundInMapOrMapEmpty = true;
                lookupInfo.service = map->begin()->second;
                return lookupInfo;
            }

            auto iter = map->find(interfaceId);
            if (iter != map->end())
            {
                lookupInfo.wasFoundInMapOrMapEmpty = true;
                lookupInfo.service = iter->second;
                return lookupInfo;
            }

            return lookupInfo;
        }

        /**
         * This function returns the service interface pointer from the InterfaceMap.
         *
         * If the returned pointer is not valid, then error checking is done to ensure
         * that if this failure of construction/activation of the searched service
         * violates the service's cardinality conditions, an exception is thrown.
         *
         * This function throws an exception when the service pointer was found in the map
         * and the value of that service pointer is nullptr.
         */
        std::shared_ptr<void>
        GetServicePointer(std::shared_ptr<ComponentConfiguration> const& configManagerPtr,
                          cppmicroservices::InterfaceMapConstPtr const& m,
                          std::string const& name,
                          std::string const& type)
        {
            ServiceInterfacePointerLookupInfo lookupInfo = GetInterfacePointer(m, type);

            /* The returned service and boolean in the ServiceInterfacePointerLookupInfo struct
             * are valid if
                1) service pointer matching the type was not found in the map or
                2) service pointer is found in map AND non null
             *
             * If this condition is not true, then we proceed with error checking and throw
             * a ComponentException if the service's cardinality conditions were violated.
             */
            bool isValid = lookupInfo.IsValid();
            if (isValid)
            {
                return lookupInfo.service;
            }

            // Checking for error condition and throwing if necessary
            // In the case of service being a nullptr, we throw because this implies
            // that the construction or activation of the service failed.
            if (ServiceReferenceTargetIsMandatory(configManagerPtr, name))
            {
                std::string errMsg = "Service ";
                errMsg += name;
                errMsg += " with type ";
                errMsg += type;
                errMsg += " failed to construct or activate.";

                throw ComponentException(errMsg);
            }

            return nullptr;
        }

        std::shared_ptr<void>
        ComponentContextImpl::LocateService(std::string const& name, std::string const& type) const
        {
            auto const configManagerPtr = configManager.lock();
            if (!configManagerPtr)
            {
                throw ComponentException("Context is invalid");
            }
            auto boundServicesCacheHandle = boundServicesCache.lock();
            auto serviceMapItr = boundServicesCacheHandle->find(name);
            if (serviceMapItr != boundServicesCacheHandle->end())
            {
                auto& services = serviceMapItr->second;

                if (!services.empty())
                {
                    auto& interfaceMapPtr = std::get<1>(services[0]);

                    return GetServicePointer(configManagerPtr, interfaceMapPtr, name, type);
                }
            }

            return nullptr;
        }

        std::shared_ptr<void>
        ComponentContextImpl::LocateService(std::string const& name, cppmicroservices::ServiceReferenceBase const& sRef)
        {
            auto const configManagerPtr = configManager.lock();
            if (!configManagerPtr)
            {
                throw ComponentException("Context is invalid");
            }
            auto boundServicesCacheHandle = boundServicesCache.lock();
            auto serviceMapItr = boundServicesCacheHandle->find(name);
            if (serviceMapItr != boundServicesCacheHandle->end())
            {
                // vector of serviceInterfaceMapConstPtr
                auto& services = serviceMapItr->second;

                auto matchingServiceInterfaceMapPtr
                    = std::find_if(services.begin(),
                                   services.end(),
                                   [&sRef](std::pair<cppmicroservices::ServiceReferenceBase, InterfaceMapConstPtr> const &pair)
                                   { return pair.first == sRef; });

                if (matchingServiceInterfaceMapPtr != services.end())
                {
                    // returns the service at the interfacemap::begin() position
                    return GetServicePointer(configManagerPtr, matchingServiceInterfaceMapPtr->second, name, "");
                }
            }

            return nullptr;
        }

        std::vector<std::shared_ptr<void>>
        ComponentContextImpl::LocateServices(std::string const& name, std::string const& type) const
        {
            auto const configManagerPtr = configManager.lock();
            if (!configManagerPtr)
            {
                throw ComponentException("Context is invalid");
            }
            std::vector<std::shared_ptr<void>> services;
            auto boundServicesCacheHandle = boundServicesCache.lock();
            auto serviceMapItr = boundServicesCacheHandle->find(name);
            if (serviceMapItr != boundServicesCacheHandle->end())
            {
                auto& serviceInfos = serviceMapItr->second;
                std::for_each(
                    serviceInfos.begin(),
                    serviceInfos.end(),
                    [&services, &configManagerPtr, &name, &type](
                        std::pair<cppmicroservices::ServiceReferenceBase, InterfaceMapConstPtr> const& serviceInfo)
                    {
                        auto const& intMapPtr = serviceInfo.second;
                        auto const servicePtr = GetServicePointer(configManagerPtr, intMapPtr, name, type);
                        if (servicePtr)
                        {
                            services.push_back(servicePtr);
                        }
                    });
            }
            return services;
        }

        cppmicroservices::BundleContext
        ComponentContextImpl::GetBundleContext() const
        {
            auto const configManagerPtr = configManager.lock();
            return (configManagerPtr ? (configManagerPtr->GetBundle()).GetBundleContext() : BundleContext());
        }

        unsigned long
        ComponentContextImpl::GetBundleId() const
        {
            auto const configManagerPtr = configManager.lock();
            if (!configManagerPtr)
            {
                throw ComponentException("Context is invalid");
            }
            return configManagerPtr->GetBundle().GetBundleId();
        }

        cppmicroservices::Bundle
        ComponentContextImpl::GetUsingBundle() const
        {
            return usingBundle;
        }

        void
        ComponentContextImpl::EnableComponent(std::string const& name)
        {
            auto const configManagerPtr = configManager.lock();
            if (!configManagerPtr)
            {
                throw ComponentException("Context is invalid");
            }
            auto const reg = configManagerPtr->GetRegistry();
            if (name.empty())
            {
                auto mgrs = reg->GetComponentManagers(GetBundleId());
                for (auto& mgr : mgrs)
                {
                    mgr->Enable();
                }
            }
            else
            {
                auto mgr = reg->GetComponentManager(GetBundleId(), name);
                mgr->Enable();
            }
        }

        void
        ComponentContextImpl::DisableComponent(std::string const& name)
        {
            auto const configManagerPtr = configManager.lock();
            if (!configManagerPtr)
            {
                throw ComponentException("Context is invalid");
            }

            auto const reg = configManagerPtr->GetRegistry();
            if (reg)
            {
                auto cm = reg->GetComponentManager(GetBundleId(), name);
                cm->Disable();
            }
        }

        ServiceReferenceBase
        ComponentContextImpl::GetServiceReference() const
        {
            auto const configManagerPtr = configManager.lock();
            return configManagerPtr ? configManagerPtr->GetServiceReference() : ServiceReferenceU();
        }

        void
        ComponentContextImpl::Invalidate()
        {
            configManager = std::weak_ptr<ComponentConfiguration>();
            auto boundServicesCacheHandle = boundServicesCache.lock();
            boundServicesCacheHandle->clear();
        }

        std::shared_ptr<void>
        ComponentContextImpl::AddToBoundServicesCache(std::string const& refName,
                                                      cppmicroservices::ServiceReferenceBase const& sRef)
        {
            auto bc = GetBundleContext();
            cppmicroservices::ServiceObjects<void> sObjs = bc.GetServiceObjects(ServiceReferenceU(sRef));
            auto boundServicesCacheHandle = boundServicesCache.lock();
            auto interfaceMap = sObjs.GetService();
            if (!interfaceMap || interfaceMap->empty())
            {
                return nullptr;
            }
            std::shared_ptr<void> svcToBind = interfaceMap->begin()->second;
            (*boundServicesCacheHandle)[refName].emplace_back(std::make_pair(sRef, interfaceMap));
            return svcToBind;
        }

        void
        ComponentContextImpl::RemoveFromBoundServicesCache(std::string const& refName,
                                                           cppmicroservices::ServiceReferenceBase const& sRef)
        {
            auto boundServicesCacheHandle = boundServicesCache.lock();
            auto& services = boundServicesCacheHandle->at(refName);
            // std::remove_if doesn't actually remove the elements from
            // 'services'. Instead it shifts them to the end of the range
            // and returns a past-the-end iterator for the new end range.
            // erase is then called to remove all elements that were shifted,
            // which are those starting from the past-the-end iterator to
            // the end of `services`.
            services.erase(std::remove_if(services.begin(),
                                          services.end(),
                                          [&sRef](std::pair<ServiceReferenceBase, InterfaceMapConstPtr> const& service)
                                          { return (std::get<0>(service) == sRef); }),
                           services.end());
            return;
        }

    } // namespace scrimpl
} // namespace cppmicroservices
