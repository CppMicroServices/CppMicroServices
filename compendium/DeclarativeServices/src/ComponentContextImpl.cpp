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
#include <cppmicroservices/Any.h>
#include <cppmicroservices/AnyMap.h>
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
 * Returns a service interface pointer from a given InterfaceMap instance.
 * 
 * This function uses a reference parameter (boolean) to determine whether or not
 * a nullptr returned from ExtractInterface is due to a construction/activation
 * failure or because the information passed to the function is garbage.
 * 
 * This function has a similar implementation to ExtractService except it uses
 * a boolean to keep track of whether or not the returned service interface
 * pointer is null due to an error during construction or activation
 *
 * - map, a InterfaceMap instance.
 * - interfaceId, The interface id string.
 * - isSIPointerFoundOrMapEmpty, a boolean which represents if the return value was in the map
 *     or the map is nullptr
 * - return value: The service interface pointer for the service interface id or
 *                 nullptr if "map" does not contain an entry for the given type.
 */
static std::shared_ptr<void> GetInterfacePointerAndCheckError(
  const InterfaceMapConstPtr& map,
  const std::string& interfaceId,
  bool& isSIPointerFoundOrMapEmpty)
{
  isSIPointerFoundOrMapEmpty = false;

  if (!map) {
    isSIPointerFoundOrMapEmpty = true;
    return nullptr;
  }

  if (interfaceId.empty() && map && !map->empty()) {
    isSIPointerFoundOrMapEmpty = true;
    return map->begin()->second;
  }

  auto iter = map->find(interfaceId);
  if (iter != map->end()) {
    isSIPointerFoundOrMapEmpty = true;
    return iter->second;
  }

  return nullptr;
}

/*
  This function gets the cardinality of the specified component name from
  the headers in the manifest file.

  If the headers of the manifest from the client bundle does not have a "scr"
  component or any of the required sub-components, an empty string is returned.

  If the specific component name is found in the references section, it returns
  the cardinality.

  If the specific component name cannot be found, it returns an empty string.
*/
std::string ExtractCardinalityForComponent(const std::string& name,
                                           const AnyMap& headers)
{
  try {
    auto components = cppmicroservices::ref_any_cast<std::vector<Any>>(
      headers.AtCompoundKey("scr.components"));
    // Loop over all component objects in the components array
    for (auto comp : components) {
      auto componentMap = cppmicroservices::ref_any_cast<AnyMap>(comp);
      auto references = cppmicroservices::ref_any_cast<std::vector<Any>>(
        componentMap.AtCompoundKey("references"));
      bool foundReference = false;
      // Loop over all reference objects in the references array
      for (auto ref : references) {
        auto referenceMap = cppmicroservices::ref_any_cast<AnyMap>(ref);
        // Get the name of the component
        auto refName = cppmicroservices::ref_any_cast<std::string>(
          referenceMap.AtCompoundKey("name"));
        // If the name passed to LocateService (and indirectly ExtractCardinalityForComponent)
        // matches the reference name, return the cardinality string
        if (refName == name) {
          return cppmicroservices::ref_any_cast<std::string>(
            referenceMap.AtCompoundKey("cardinality"));
        }
      }
    }
  } catch (...) { // Catch all if any of the keys cannot be found in the map
    return "";
  }

  return "";
}

/*
  ValidateExtractedService is the error-checking mechanism for LocateService and LocateServices.

  ValidateExtractedService is ran if a nullptr was returned from GetInterfacePointerAndCheckError and
  it was a nullptr that was found in the map.

  If the cardinality of the service is mandatory (1..1, 1..n), a ComponentException
  is thrown.
*/
void ValidateExtractedService(
  const std::shared_ptr<cppmicroservices::scrimpl::ComponentConfiguration>
    configManagerPtr,
  const std::string& name,
  const std::string& type,
  const std::shared_ptr<void>& service,
  bool isSIPointerFoundOrMapEmpty)
{
  auto bundle = configManagerPtr->GetBundle();
  const auto& headers = bundle.GetHeaders();

  std::string cardinality = ExtractCardinalityForComponent(name, headers);
  bool mandatoryCardinality = cardinality.find("1..") != std::string::npos;
  if (cardinality == "") {
    mandatoryCardinality = true;
  }

  // In the case of service being a nullptr, we throw because this implies
  // that the construction or activation of the service failed.
  if (!service && isSIPointerFoundOrMapEmpty && mandatoryCardinality) {
    std::string errMsg = "Service ";
    errMsg += name;
    errMsg += " with type ";
    errMsg += type;
    errMsg += " failed to construct or activate.";

    throw ComponentException(errMsg);
  }
}

std::shared_ptr<void> ComponentContextImpl::LocateService(
  const std::string& name,
  const std::string& type) const
{
  const auto configManagerPtr = configManager.lock();
  if (!configManagerPtr) {
    throw ComponentException("Context is invalid");
  }
  std::shared_ptr<void> service;
  auto boundServicesCacheHandle = boundServicesCache.lock();
  auto serviceMapItr = boundServicesCacheHandle->find(name);
  if (serviceMapItr != boundServicesCacheHandle->end()) {
    auto& serviceMaps = serviceMapItr->second;
    if (!serviceMaps.empty()) {
      bool isSIPointerFoundOrMapEmpty = false;
      service = GetInterfacePointerAndCheckError(
        serviceMaps.at(0), type, isSIPointerFoundOrMapEmpty);

      /* If a valid pointer was returned from GetInterfacePointerAndCheckError and
         the pointer was found in the map or a nullptr was returned from
         GetInterfacePointerAndCheckError and it was because it wasn't in the map,
         just return service (whatever it may be).

         The case where service != nullptr and isSIPointerFoundOrMapEmpty = false
         is not possible.
      
         If a nullptr was returned AND it was found in the map, then something
         went wrong during the service's construction or activation and proceed
         with error handling.
      */
      bool isValid = (static_cast<bool>(service) == isSIPointerFoundOrMapEmpty);
      if (isValid) {
        return service;
      } else { // Checking for error condition and throwing if necessary
        ValidateExtractedService(
          configManagerPtr, name, type, service, isSIPointerFoundOrMapEmpty);
      }
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
    std::for_each(
      serviceMaps.begin(),
      serviceMaps.end(),
      [&configManagerPtr, &services, &name, &type](
        const InterfaceMapConstPtr& iMap) {
        bool isSIPointerFoundOrMapEmpty = false;
        std::shared_ptr<void> service = GetInterfacePointerAndCheckError(
          iMap, type, isSIPointerFoundOrMapEmpty);

        bool isValid =
          (static_cast<bool>(service) == isSIPointerFoundOrMapEmpty);
        if (isValid) {
          services.push_back(service);
        } else { // Checking for error condition and throwing if necessary
          ValidateExtractedService(
            configManagerPtr, name, type, service, isSIPointerFoundOrMapEmpty);
        }
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
