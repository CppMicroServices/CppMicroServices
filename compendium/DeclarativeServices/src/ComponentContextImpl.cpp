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
      service = ExtractInterface(serviceMaps.at(0), type);
    }
  }
  return service;
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
                  [&services, &type](const InterfaceMapConstPtr& iMap) {
                    services.push_back(ExtractInterface(iMap, type));
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
  (*boundServicesCacheHandle)[refName].emplace_back(
    interfaceMap);
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
