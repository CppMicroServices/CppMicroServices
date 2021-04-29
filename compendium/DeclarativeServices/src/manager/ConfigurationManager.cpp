
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

#include "ConfigurationManager.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
using cppmicroservices::service::component::ComponentConstants::CONFIG_POLICY_IGNORE;
using cppmicroservices::service::component::ComponentConstants::CONFIG_POLICY_REQUIRE;


namespace cppmicroservices {
namespace scrimpl {
ConfigurationManager::ConfigurationManager(
  const std::shared_ptr<const metadata::ComponentMetadata> metadata,
  const cppmicroservices::BundleContext& bc,
  std::shared_ptr<cppmicroservices::logservice::LogService> logger)
  : logger(std::move(logger))
  , metadata(metadata)
  , bundleContext(bc)
  , mergedProperties (metadata->properties)
{
  if (!this->metadata || !this->bundleContext || !this->logger) {
    throw std::invalid_argument(
      "ConfigurationManagerImpl - Invalid arguments passed to constructor");
  }
}

cppmicroservices::AnyMap ConfigurationManager::GetProperties() const noexcept
{
  std::lock_guard<std::mutex> lock(propertiesMutex);

  return mergedProperties;
}

void ConfigurationManager::Initialize() {
  if ((metadata->configurationPids.empty()) ||
      (metadata->configurationPolicy == CONFIG_POLICY_IGNORE)) {  
    return;
  }
  try { 
    auto sr =
        this->bundleContext
        .GetServiceReference<cppmicroservices::service::cm::ConfigurationAdmin>();
    auto configAdmin =
        this->bundleContext
        .GetService<cppmicroservices::service::cm::ConfigurationAdmin>(sr);

    std::lock_guard<std::mutex> lock(propertiesMutex);

    for (const auto& pid : metadata->configurationPids) {
      if (configProperties.find(pid) == configProperties.end()) {
        auto config = configAdmin->ListConfigurations("(pid = " + pid + ")");
        if (config.size() > 0) {
          auto properties = config.front()->GetProperties();
          configProperties.emplace(pid, properties);
          for (const auto &item : properties) {
            mergedProperties[item.first] = item.second;
          }   
        }      
      } 
    }
  } catch (...) {
    // No ConfigAdmin available
      logger->Log(
          cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
          "Exception while initializing ConfigurationManager object",
          std::current_exception());

    return;
  }
}

void ConfigurationManager::UpdateMergedProperties(const std::string pid, 
  std::shared_ptr<cppmicroservices::AnyMap> props,
  const cppmicroservices::service::cm::ConfigurationEventType type,
  const ComponentState currentState,
  bool& configWasSatisfied,
  bool& configNowSatisfied)
{
  std::lock_guard<std::mutex> lock(propertiesMutex);
  configWasSatisfied = isConfigSatisfied(currentState);

 

  // delete properties for this pid or replace with new properties in configProperties
  
  auto it = configProperties.find(pid);
  if (it != configProperties.end()) {
    configProperties.erase(it);
  }
  if (type == cppmicroservices::service::cm::ConfigurationEventType::CM_UPDATED) {
      configProperties.emplace(pid, *props);
  }
  
  //  recalculate the merged properties maintaining precedence as follows:
  //  lowest precedence properties from the metadata
  //  next precedence each pid in meta-data configuration-pids with first one
  //  in the list having lower precedence than the last one in the list. 

  mergedProperties = metadata->properties;

  for (const auto& pid : metadata->configurationPids) {
    auto it = configProperties.find(pid);
    if (it != configProperties.end()) { 
      for (const auto &item : it->second) {
          mergedProperties[item.first] = item.second;
        }
      }
  } 

   configNowSatisfied = isConfigSatisfied(currentState);
}

/**
   * Returns \c true if the configuration dependencies are satisfied, \c false otherwise
   */
bool ConfigurationManager::IsConfigSatisfied(
  const ComponentState currentState) const noexcept
{
  std::lock_guard<std::mutex> lock(propertiesMutex);

  return isConfigSatisfied(currentState);
  
}

bool ConfigurationManager::isConfigSatisfied(
  const ComponentState /*currentState*/) const noexcept
{
  bool allConfigsAvailable =
    configProperties.size() >= metadata->configurationPids.size();

  if ((metadata->configurationPolicy != CONFIG_POLICY_REQUIRE) ||
      (allConfigsAvailable)) {
    return true;
  }

  return false;
}
}
}

