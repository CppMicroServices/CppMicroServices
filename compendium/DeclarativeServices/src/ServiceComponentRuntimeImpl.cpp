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

#include <chrono>
#include "ServiceComponentRuntimeImpl.hpp"
#include "cppmicroservices/servicecomponent/runtime/dto/ComponentDescriptionDTO.hpp"
#include "cppmicroservices/servicecomponent/runtime/dto/ComponentConfigurationDTO.hpp"
#include "cppmicroservices/servicecomponent/runtime/dto/ReferenceDTO.hpp"
#include "manager/ComponentManager.hpp"
#include "manager/ReferenceManager.hpp"
#include "manager/ComponentConfiguration.hpp"

using cppmicroservices::framework::dto::BundleDTO;
using cppmicroservices::framework::dto::ServiceReferenceDTO;
using cppmicroservices::service::component::runtime::dto::ReferenceDTO;
using cppmicroservices::scrimpl::metadata::ReferenceMetadata;

using std::chrono::steady_clock;
using std::chrono::system_clock;

namespace cppmicroservices {
namespace scrimpl {
/**
 * Utility Methods used in ServiceComponentRuntimeImpl
 */
time_t steady_clock_to_time_t( steady_clock::time_point t )
{
  return system_clock::to_time_t(system_clock::now() + std::chrono::duration_cast<system_clock::duration>(t - steady_clock::now()));
}

BundleDTO ToDTO(const cppmicroservices::Bundle& bundle)
{
  BundleDTO bundleDTO = {};
  bundleDTO.id = bundle.GetBundleId();
  bundleDTO.symbolicName = bundle.GetSymbolicName();
  bundleDTO.lastModified = static_cast<unsigned long>(steady_clock_to_time_t(bundle.GetLastModified()));
  bundleDTO.state = bundle.GetState();
  bundleDTO.version = bundle.GetVersion().ToString();
  return bundleDTO;
}

ServiceReferenceDTO ToDTO(const cppmicroservices::ServiceReferenceBase& sRef)
{
  ServiceReferenceDTO refDTO = {};
  refDTO.id = cppmicroservices::any_cast<long>(sRef.GetProperty(cppmicroservices::Constants::SERVICE_ID));
  refDTO.bundle = sRef ? sRef.GetBundle().GetBundleId() : 0;
  std::vector<std::string> keys;
  sRef.GetPropertyKeys(keys);
  for(auto& key : keys)
  {
    cppmicroservices::Any val = sRef.GetProperty(key);
    refDTO.properties.insert(std::make_pair(key, val));
  }
  std::vector<cppmicroservices::Bundle> bundles = sRef.GetUsingBundles();
  for(auto& bundle : bundles)
  {
    if(bundle)
    {
      refDTO.usingBundles.push_back(bundle.GetBundleId());
    }
  }
  return refDTO;
}

ReferenceDTO ToDTO(const ReferenceMetadata& refData)
{
  ReferenceDTO refDTO = {};
  refDTO.cardinality = refData.cardinality;
  refDTO.interfaceName = refData.interfaceName;
  refDTO.name = refData.name;
  refDTO.policy = refData.policy;
  refDTO.policyOption = refData.policyOption;
  refDTO.scope = refData.scope;
  refDTO.target = refData.target;
  return refDTO;
}

ServiceComponentRuntimeImpl::ServiceComponentRuntimeImpl(cppmicroservices::BundleContext context,
                                                         std::shared_ptr<ComponentRegistry> componentRegistry,
                                                         std::shared_ptr<cppmicroservices::logservice::LogService> logger)
  : scrContext(std::move(context))
  , registry(std::move(componentRegistry))
  , logger(std::move(logger))
{
  if(!scrContext || !registry || !(this->logger))
  {
    throw std::invalid_argument("ServiceComponentRuntimeImpl Constructor provided with invalid arguments");
  }
}

std::vector<ComponentDescriptionDTO> ServiceComponentRuntimeImpl::GetComponentDescriptionDTOs(const std::vector<cppmicroservices::Bundle>& bundles) const
{
  std::vector<std::shared_ptr<ComponentManager>> compMgrs;
  if (bundles.empty())
  {
    compMgrs = registry->GetComponentManagers();
  }
  else
  {
    for (auto& bundle : bundles)
    {
      auto managersInBundle = registry->GetComponentManagers(bundle.GetBundleId());
      compMgrs.insert(std::end(compMgrs), std::begin(managersInBundle), std::end(managersInBundle));
    }
  }
  std::vector<ComponentDescriptionDTO> componentDTOs;
  for (auto holder : compMgrs)
  {
    componentDTOs.push_back(CreateDTO(holder));
  }
  return componentDTOs;
}

ComponentDescriptionDTO ServiceComponentRuntimeImpl::GetComponentDescriptionDTO(const cppmicroservices::Bundle& bundle, const std::string& name) const
{
  ComponentDescriptionDTO compDTO = {};
  try
  {
    std::shared_ptr<ComponentManager> manager = registry->GetComponentManager(bundle.GetBundleId(), name);
    compDTO = CreateDTO(manager);
  }
  catch (const std::exception&)
  {
    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG, "Exception: ", std::current_exception());
  }
  return compDTO;
}

std::vector<ComponentConfigurationDTO> ServiceComponentRuntimeImpl::GetComponentConfigurationDTOs(const ComponentDescriptionDTO& description) const
{
  std::vector<ComponentConfigurationDTO> compConfigDTOs;
  std::shared_ptr<ComponentManager> manager = registry->GetComponentManager(description.bundle.id, description.name);
  if(manager)
  {
    std::vector<std::shared_ptr<ComponentConfiguration>> configs = manager->GetComponentConfigurations();
    for(auto& aConfig : configs)
    {
      auto compConfigDTO = CreateComponentConfigurationDTO(aConfig);
      compConfigDTO.description = CreateDTO(manager);
      compConfigDTOs.push_back(compConfigDTO);
    }
  }
  return compConfigDTOs;
}

bool ServiceComponentRuntimeImpl::IsComponentEnabled(const ComponentDescriptionDTO& description) const
{
  std::shared_ptr<ComponentManager> mgr = registry->GetComponentManager(description.bundle.id, description.name);
  return mgr ? mgr->IsEnabled() : false;
}

std::shared_future<void> ServiceComponentRuntimeImpl::EnableComponent(const ComponentDescriptionDTO& description)
{
  std::shared_ptr<ComponentManager> holder = registry->GetComponentManager(description.bundle.id, description.name);
  return holder->Enable();
}

std::shared_future<void> ServiceComponentRuntimeImpl::DisableComponent(const ComponentDescriptionDTO& description)
{
  std::shared_ptr<ComponentManager> holder = registry->GetComponentManager(description.bundle.id, description.name);
  return holder->Disable();
}

ComponentDescriptionDTO ServiceComponentRuntimeImpl::CreateDTO(const std::shared_ptr<ComponentManager>& compManager) const
{
  ComponentDescriptionDTO compDescription = {};
  auto compMetadata = compManager->GetMetadata();
  if(compMetadata)
  {
    compDescription.name = compMetadata->name;
    auto bundleId = compManager->GetBundleId();
    compDescription.bundle = ToDTO(scrContext.GetBundle(bundleId));
    compDescription.immediate = compMetadata->immediate;
    compDescription.activate = compMetadata->activateMethodName;
    compDescription.deactivate = compMetadata->deactivateMethodName;
    compDescription.modified = compMetadata->modifiedMethodName;
    auto serviceData = compMetadata->serviceMetadata;
    compDescription.scope = serviceData.scope;
    compDescription.serviceInterfaces = serviceData.interfaces;
    compDescription.implementationClass = compMetadata->implClassName;
    compDescription.defaultEnabled = compMetadata->enabled;
    compDescription.properties = compMetadata->properties;
    for (auto oneRef : compMetadata->refsMetadata)
    {
      compDescription.references.push_back(ToDTO(oneRef));
    }
  }
  return compDescription;
}

ComponentConfigurationDTO ServiceComponentRuntimeImpl::CreateComponentConfigurationDTO(const std::shared_ptr<ComponentConfiguration>& config) const
{
  ComponentConfigurationDTO configDTO = {};
  configDTO.id = config->GetId();
  configDTO.properties = config->GetProperties();
  configDTO.state = config->GetConfigState();
  auto refManagers = config->GetAllDependencyManagers();
  for(auto& refManager : refManagers)
  {
    if(refManager->IsSatisfied())
    {
      configDTO.satisfiedReferences.push_back(CreateSatisfiedReferenceDTO(refManager));
    }
    else
    {
      configDTO.unsatisfiedReferences.push_back(CreateUnsatisfiedReferenceDTO(refManager));
    }
  }
  return configDTO;
}

SatisfiedReferenceDTO ServiceComponentRuntimeImpl::CreateSatisfiedReferenceDTO(const std::shared_ptr<ReferenceManager>& refManager) const
{
  SatisfiedReferenceDTO refDTO = {};
  refDTO.name = refManager->GetReferenceName();
  refDTO.target = refManager->GetLDAPString();
  auto sRefs = refManager->GetBoundReferences();
  for(auto& sRef : sRefs)
  {
    refDTO.boundServices.push_back(ToDTO(sRef));
  }
  return refDTO;
}

UnsatisfiedReferenceDTO ServiceComponentRuntimeImpl::CreateUnsatisfiedReferenceDTO(const std::shared_ptr<ReferenceManager>& refManager) const
{
  UnsatisfiedReferenceDTO refDTO = {};
  refDTO.name = refManager->GetReferenceName();
  refDTO.target = refManager->GetLDAPString();
  auto sRefs = refManager->GetTargetReferences();
  for(auto& sRef : sRefs)
  {
    refDTO.targetServices.push_back(ToDTO(sRef));
  }
  return refDTO;
}
}
}
