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

#ifndef __CONFIGURATIONMANAGER_HPP__
#define __CONFIGURATIONMANAGER_HPP__

#include "ConcurrencyUtil.hpp"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/cm/ConfigurationListener.hpp"
#include "cppmicroservices/logservice/LogService.hpp"
#include "cppmicroservices/servicecomponent/detail/ComponentInstance.hpp"
#include "../metadata/ComponentMetadata.hpp"
#include "states/ComponentConfigurationState.hpp"

namespace cppmicroservices {
namespace scrimpl {
/**
 * Class responsible for managing configuration objects
 * for component configurations and for updating the ComponentInstance
 * when configuration properties change.
 */

class ConfigurationManager final
{
public:
  ConfigurationManager::ConfigurationManager(
    std::shared_ptr<const metadata::ComponentMetadata> metadata,
    const cppmicroservices::BundleContext& bc,
    std::shared_ptr<cppmicroservices::logservice::LogService> logger
    );
  ConfigurationManager(const ConfigurationManager&) = delete;
  ConfigurationManager(ConfigurationManager&&) = delete;
  ConfigurationManager& operator=(const ConfigurationManager&) = delete;
  ConfigurationManager& operator=(ConfigurationManager&&) = delete;
  virtual ~ConfigurationManager() = default;

  /**
  * Method to initialize a newly constructed Configuration Manager. 
  */
  void Initialize();

   /**
   * Returns \c true if the configuration dependencies are satisfied, \c false otherwise. 
   * Whether or not configuration objects are satisfied is determined by whether or not 
   * the configuration object is available and on the configuration-policy (ignore, optional,
   * require)
   */
  bool IsConfigSatisfied(const ComponentState currentState) const noexcept;

  /**
   * If the component has configuration object dependencies then the properties for
   * the component are merged from the component properties and the properties for 
   * the configuration objects. 
   */
  void UpdateMergedProperties(
      const std::string pid,
      std::shared_ptr<cppmicroservices::AnyMap> props,
    const cppmicroservices::service::cm::ConfigurationEventType type) noexcept;
 
  /* To be implemented. */
   void SendModifiedPropertiesToComponent();

  /* Returns the merged properties for the component. These properties 
   * are a merged from the component properties and the properties for 
   * all of the configuration objects on which this component is dependent. 
   */
   cppmicroservices::AnyMap GetProperties() const noexcept;

 protected:
  ConfigurationManager() = default;

  std::shared_ptr<cppmicroservices::logservice::LogService> logger; ///< logger for this runtime
  const std::shared_ptr<const metadata::ComponentMetadata> metadata;
  cppmicroservices::BundleContext bundleContext; ///< context of the bundle which contains the component
  std::unordered_map<std::string, cppmicroservices::AnyMap>  configProperties;  //properties for available configuration objects.
  cppmicroservices::AnyMap mergedProperties;
};
}
}
#endif // __CONFIGURATIONMANAGER_HPP__
