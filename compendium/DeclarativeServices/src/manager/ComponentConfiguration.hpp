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

#ifndef __COMPONENTCONFIGURATION_HPP__
#define __COMPONENTCONFIGURATION_HPP__
#include <unordered_map>
#include "cppmicroservices/Any.h"
#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/servicecomponent/runtime/dto/ComponentConfigurationDTO.hpp"

using cppmicroservices::service::component::runtime::dto::ComponentState;

namespace cppmicroservices {
namespace scrimpl {

class ComponentRegistry;
class ReferenceManager;
class RegistrationManager;

/**
 * This interface represents a component configuration. The implementations of
 * this interface are responsible for managing the lifecycle of a component
 * configuration. This interface is used by ServiceComponentRuntimeImpl to
 * support the runtime introspection API.
 */
class ComponentConfiguration
{
public:
  ComponentConfiguration() = default;
  ComponentConfiguration(const ComponentConfiguration&) = delete;
  ComponentConfiguration(ComponentConfiguration&&) = delete;
  ComponentConfiguration& operator=(const ComponentConfiguration&) = delete;
  ComponentConfiguration& operator=(ComponentConfiguration&&) = delete;
  virtual ~ComponentConfiguration() = default;

  /**
   * Returns a list of all the reference manager objects used to track
   * this configuration's dependencies
   */
  virtual std::vector<std::shared_ptr<ReferenceManager>> GetAllDependencyManagers() const = 0;

  /**
   * Returns the reference manager object used to track a service dependency
   * with a specific name
   */
  virtual std::shared_ptr<ReferenceManager> GetDependencyManager(const std::string& refName) const = 0;

  /**
   * Returns a valid ServiceReference object if this component is registered in
   * the framework service registry
   */
  virtual ServiceReferenceBase GetServiceReference() const = 0;

  /**
   * Returns the component registry object of this runtime. This method is used by
   * ComponentContextImpl objects to retrieve ComponentManager objects.
   */
  virtual std::shared_ptr<const ComponentRegistry> GetRegistry() const = 0;

  /**
   * Returns a map with properties specific to this component configuration
   */
  virtual std::unordered_map<std::string, cppmicroservices::Any> GetProperties() const = 0;

  /**
   * Returns the bundle that contains this component
   */
  virtual cppmicroservices::Bundle GetBundle() const = 0;

  /**
   * Returns the unique ID of this component configuration
   */
  virtual unsigned long GetId() const = 0;

  /**
   * Returns the current {@code ComponentState} of this component configuration
   */
  virtual ComponentState GetConfigState() const = 0;
};
} // scrimpl
} // cppmicroservices
#endif /* __COMPONENTCONFIGURATION_HPP__ */
