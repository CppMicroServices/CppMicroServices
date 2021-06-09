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

namespace cppmicroservices {
namespace scrimpl {

ComponentContextImpl::ComponentContextImpl(
  std::weak_ptr<ComponentConfiguration> cm)
  : configManager(std::move(cm))
  , usingBundle(Bundle())
{
  InitializeServicesCache();
}

ComponentContextImpl::ComponentContextImpl(
  std::weak_ptr<ComponentConfiguration> cm,
  cppmicroservices::Bundle usingBundle)
  : configManager(std::move(cm))
  , usingBundle(std::move(usingBundle))
{
  InitializeServicesCache();
}

void ComponentContextImpl::InitializeServicesCache()
{
  const auto configManagerPtr = configManager.lock();
  if (!configManagerPtr) {
    throw ComponentException("Context is invalid");
  }

  const auto refManagers = configManagerPtr->GetAllDependencyManagers();
  for (const auto& refManager : refManagers) {
    const auto& sRefs = refManager->GetBoundReferences();
    const auto& refName = refManager->GetReferenceName();
    const auto& refScope = refManager->GetReferenceScope();
    std::for_each(
      sRefs.rbegin(),
      sRefs.rend(),
      [&](const cppmicroservices::ServiceReferenceBase& sRef) {
        if (sRef) {
          ServiceReferenceU sRefU(sRef);
          auto bc = GetBundleContext();
          auto boundServicesCacheHandle = boundServicesCache.lock();
          auto& serviceMap = (*boundServicesCacheHandle)[refName];
          if (refScope == cppmicroservices::Constants::SCOPE_BUNDLE) {
            serviceMap.push_back(bc.GetService(sRefU));
          } else {
            auto registeredScope =
              sRef.GetProperty(cppmicroservices::Constants::SERVICE_SCOPE)
                .ToStringNoExcept();
            cppmicroservices::ServiceObjects<void> sObjs =
              bc.GetServiceObjects(sRefU);
            auto interfaceMap = sObjs.GetService();
            if (interfaceMap) {
              serviceMap.push_back(interfaceMap);
            }
          }
        }
      });
  }
}

std::unordered_map<std::string, cppmicroservices::Any>
ComponentContextImpl::GetProperties() const
{
  const auto configManagerPtr = configManager.lock();
  if (!configManagerPtr) {
    throw ComponentException("Context is invalid");
  }
  return configManagerPtr->GetProperties();
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

  bool IsValid() const
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
ServiceInterfacePointerLookupInfo GetInterfacePointer(
  const InterfaceMapConstPtr& map,
  const std::string& interfaceId)
{
  ServiceInterfacePointerLookupInfo lookupInfo{};

  if (!map) {
    lookupInfo.wasFoundInMapOrMapEmpty = true;
    lookupInfo.service = nullptr;
    return lookupInfo;
  }

  if (interfaceId.empty() && map && !map->empty()) {
    lookupInfo.wasFoundInMapOrMapEmpty = true;
    lookupInfo.service = map->begin()->second;
    return lookupInfo;
  }

  auto iter = map->find(interfaceId);
  if (iter != map->end()) {
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
std::shared_ptr<void> GetServicePointer(
  const std::shared_ptr<ComponentConfiguration>& configManagerPtr,
  const cppmicroservices::InterfaceMapConstPtr& m,
  const std::string& name,
  const std::string& type)
{
  ServiceInterfacePointerLookupInfo lookupInfo = GetInterfacePointer(m, type);

  /* The returned service and boolean in the ServiceInterfacePointerLookupInfo struct
   * are valid if the service pointer is null because it was not found in the map or if the
   * service pointer is not null and it was found in the map.
   * 
   * If this condition is not true, then we proceed with error checking and throw
   * a ComponentException if the service's cardinality conditions were violated.
   */
  bool isValid = lookupInfo.IsValid();
  if (isValid) {
    return lookupInfo.service;
  } else { // Checking for error condition and throwing if necessary
    std::string cardinality{};
    auto metadata = configManagerPtr->GetMetadata();
    for (const auto& _data : metadata->refsMetadata) {
      if (_data.name == name) {
        cardinality = _data.cardinality;
      }
    }

    bool isMandatory = false;
    if (cardinality.empty()) {
      isMandatory = true;
    } else {
      isMandatory = cardinality.find("1..") != std::string::npos;
    }

    // In the case of service being a nullptr, we throw because this implies
    // that the construction or activation of the service failed.
    if (!lookupInfo.IsValid() && isMandatory) {
      std::string errMsg = "Service ";
      errMsg += name;
      errMsg += " with type ";
      errMsg += type;
      errMsg += " failed to construct or activate.";

      throw ComponentException(errMsg);
    }
  }

  return nullptr;
}

std::shared_ptr<void> ComponentContextImpl::LocateService(
  const std::string& name,
  const std::string& type) const
{
  const auto configManagerPtr = configManager.lock();
  if (!configManagerPtr) {
    throw ComponentException("Context is invalid");
  }
  auto boundServicesCacheHandle = boundServicesCache.lock();
  auto serviceMapItr = boundServicesCacheHandle->find(name);
  if (serviceMapItr != boundServicesCacheHandle->end()) {
    auto& serviceMaps = serviceMapItr->second;
    if (!serviceMaps.empty()) {
      std::shared_ptr<void> service =
        GetServicePointer(configManagerPtr, serviceMaps.at(0), name, type);
      return service;
    }
  }

  return nullptr;
}

std::vector<std::shared_ptr<void>> ComponentContextImpl::LocateServices(
  const std::string& name,
  const std::string& type) const
{
  const auto configManagerPtr = configManager.lock();
  if (!configManagerPtr) {
    throw ComponentException("Context is invalid");
  }
  std::vector<std::shared_ptr<void>> services;
  auto boundServicesCacheHandle = boundServicesCache.lock();
  auto serviceMapItr = boundServicesCacheHandle->find(name);
  if (serviceMapItr != boundServicesCacheHandle->end()) {
    auto& serviceMaps = serviceMapItr->second;
    std::for_each(serviceMaps.begin(),
                  serviceMaps.end(),
                  [&configManagerPtr, &services, &name, &type](
                    const InterfaceMapConstPtr& iMap) {
                    std::shared_ptr<void> service =
                      GetServicePointer(configManagerPtr, iMap, name, type);
                    services.push_back(service);
                  });
  }
  return services;
}

cppmicroservices::BundleContext ComponentContextImpl::GetBundleContext() const
{
  const auto configManagerPtr = configManager.lock();
  return (configManagerPtr ? (configManagerPtr->GetBundle()).GetBundleContext()
                           : BundleContext());
}

unsigned long ComponentContextImpl::GetBundleId() const
{
  const auto configManagerPtr = configManager.lock();
  if (!configManagerPtr) {
    throw ComponentException("Context is invalid");
  }
  return configManagerPtr->GetBundle().GetBundleId();
}

cppmicroservices::Bundle ComponentContextImpl::GetUsingBundle() const
{
  return usingBundle;
}

void ComponentContextImpl::EnableComponent(const std::string& name)
{
  const auto configManagerPtr = configManager.lock();
  if (!configManagerPtr) {
    throw ComponentException("Context is invalid");
  }
  const auto reg = configManagerPtr->GetRegistry();
  if (name.empty()) {
    auto mgrs = reg->GetComponentManagers(GetBundleId());
    for (auto& mgr : mgrs) {
      mgr->Enable();
    }
  } else {
    auto mgr = reg->GetComponentManager(GetBundleId(), name);
    mgr->Enable();
  }
}

void ComponentContextImpl::DisableComponent(const std::string& name)
{
  const auto configManagerPtr = configManager.lock();
  if (!configManagerPtr) {
    throw ComponentException("Context is invalid");
  }

  const auto reg = configManagerPtr->GetRegistry();
  if (reg) {
    auto cm = reg->GetComponentManager(GetBundleId(), name);
    cm->Disable();
  }
}

ServiceReferenceBase ComponentContextImpl::GetServiceReference() const
{
  const auto configManagerPtr = configManager.lock();
  return configManagerPtr ? configManagerPtr->GetServiceReference()
                          : ServiceReferenceU();
}

void ComponentContextImpl::Invalidate()
{
  configManager = std::weak_ptr<ComponentConfiguration>();
  auto boundServicesCacheHandle = boundServicesCache.lock();
  boundServicesCacheHandle->clear();
}

bool ComponentContextImpl::AddToBoundServicesCache(
  const std::string& refName,
  const cppmicroservices::ServiceReferenceBase& sRef)
{
  auto bc = GetBundleContext();
  cppmicroservices::ServiceObjects<void> sObjs =
    bc.GetServiceObjects(ServiceReferenceU(sRef));
  auto boundServicesCacheHandle = boundServicesCache.lock();
  auto interfaceMap = sObjs.GetService();
  if (!interfaceMap) {
    return false;
  }
  (*boundServicesCacheHandle)[refName].emplace_back(interfaceMap);
  return true;
}

void ComponentContextImpl::RemoveFromBoundServicesCache(
  const std::string& refName,
  const cppmicroservices::ServiceReferenceBase& sRef)
{
  auto bc = GetBundleContext();
  cppmicroservices::ServiceObjects<void> sObjs =
    bc.GetServiceObjects(ServiceReferenceU(sRef));

  const auto removedService = sObjs.GetService();
  const auto& serviceInterface = removedService->begin()->first;
  auto boundServicesCacheHandle = boundServicesCache.lock();
  auto& services = boundServicesCacheHandle->at(refName);
  // std::remove_if doesn't actually remove the elements from
  // 'services'. Instead it shifts them to the end of the range
  // and returns a past-the-end iterator for the new end range.
  // erase is then called to remove all elements that were shifted,
  // which are those starting from the past-the-end iterator to
  // the end of `services`.
  services.erase(
    std::remove_if(
      services.begin(),
      services.end(),
      [&removedService, &serviceInterface](
        const cppmicroservices::InterfaceMapConstPtr& servicesMap) {
        return (1 == servicesMap->count(serviceInterface)) &&
               (removedService->at(serviceInterface) ==
                servicesMap->at(serviceInterface));
      }),
    services.end());
  return;
}

}
}
