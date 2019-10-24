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

#ifndef __COMPONENTMANAGER_HPP__
#define __COMPONENTMANAGER_HPP__

#include <memory>
#include <future>
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/ServiceFactory.h"
#include "../metadata/ComponentMetadata.hpp"

namespace cppmicroservices {
namespace scrimpl {

class ComponentConfiguration;
/**
 * This interface provides the information about the current state of a component.
 * It is used by ServiceComponentRuntimeImpl and ComponentContextImpl classes.
 */
class ComponentManager
{
public:
  ComponentManager() = default;
  ComponentManager(const ComponentManager&) = delete;
  ComponentManager(ComponentManager&&) = delete;
  ComponentManager& operator=(const ComponentManager&) = delete;
  ComponentManager& operator=(ComponentManager&&) = delete;
  virtual ~ComponentManager() = default;

  /**
   * Returns the name of the component managed by this object. The name is the same
   * as specified in the component description.
   */
  virtual std::string GetName() const = 0;

  /**
   * Returns the Id of the Bundle this component belongs to.
   */
  virtual unsigned long GetBundleId() const = 0;

  /**
   * Returns true if the component is enabled, false otherwise
   */
  virtual bool IsEnabled() const = 0;

  /**
   * This method changes the state of the ComponentManager to ENABLED. The method returns
   * immediately after changing the state. Any configurations created as a result of the
   * state change will happen asynchronously on a separate thread.
   */
  virtual std::shared_future<void> Enable() = 0;

  /**
   * This method changes the state of the ComponentManager to DISABLED. The method returns
   * immediately after changing the state. Any configurations deleted as a result of the
   * state change will happen asynchronously on a separate thread.
   */
  virtual std::shared_future<void> Disable() = 0;

  /**
   * Returns a vector of ComponentConfiguration objects representing each of the configurations
   * created for the component.
   */
  virtual std::vector<std::shared_ptr<ComponentConfiguration>> GetComponentConfigurations() const = 0;

  /**
   * Returns the metadata object representing the component description for the
   * component managed by this object.
   */
  virtual std::shared_ptr<const metadata::ComponentMetadata> GetMetadata() const = 0;
};
} // scrimpl
} // cppmicroservices
#endif // __COMPONENTMANAGER_HPP__
