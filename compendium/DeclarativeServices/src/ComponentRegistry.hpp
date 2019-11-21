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

#ifndef __COMPONENT_REGISTRY_HPP__
#define __COMPONENT_REGISTRY_HPP__

#include <memory>
#include <vector>
#include "manager/ComponentManager.hpp"

namespace cppmicroservices {
namespace scrimpl {
/**
 * This class provides a thread-safe store for ComponentManager objects
 * created by the runtime.
 */
class ComponentRegistry
{
public:
  ComponentRegistry() = default;
  virtual ~ComponentRegistry() = default;
  ComponentRegistry(const ComponentRegistry&) = delete;
  ComponentRegistry& operator=(const ComponentRegistry&) = delete;
  ComponentRegistry(ComponentRegistry&&) = delete;
  ComponentRegistry& operator=(ComponentRegistry&&) = delete;

  /**
   * Method returns all the component manager objects stored in the registry
   *
   * \return a vector of {@link ComponentManager} objects that are stored in
   * the registry
   */
  virtual std::vector<std::shared_ptr<ComponentManager>> GetComponentManagers() const;

  /**
   * Method returns all the component manager objects from a bundle, stored
   * in the registry
   *
   * \param bundleId is the id of the {@link Bundle} whose component managers
   *        are to be returned
   * \return a vector of {@link ComponentManager} objects that are stored in
   *         the registry and which belong to the {@link Bundle} with the
   *         given id.
   */
  virtual std::vector<std::shared_ptr<ComponentManager>> GetComponentManagers(unsigned long bundleId) const;

  /**
   * Method returns a component manager object with the given name and from the given bundle
   *
   * \param bundleId is the id of the {@link Bundle} which contains the
   *        component description
   * \param compName is the name of the component as specified in the
   *        component description
   * \return a {@link ComponentManager} object that are stored in
   *         the registry and which belong to the {@link Bundle} with the
   *         given id.
   */
  virtual std::shared_ptr<ComponentManager> GetComponentManager(unsigned long bundleId,
                                                                const std::string& compName) const;

  /**
   * Method removes a component manager object from the component registry
   * if it exists in the registry. If a component manager with the provided
   * bundle id and component name is not in the registry, the registry is
   * left unchanged.
   *
   * \param bundleId is the id of the {@link Bundle} which contains the
   *        component description
   * \param compName is the name of the component as specified in the
   *        component description
   */
  void RemoveComponentManager(unsigned long bundleId,
                              const std::string& compName);

  /**
   * Method to add a component manager object into the component registry
   *
   * \param cm is the {@link ComponentManager} object as specified in the
   *        component description
   * \return \c true if the component manager was inserted into the component
   *         registry, \c false otherwise
   */
  virtual bool AddComponentManager(const std::shared_ptr<ComponentManager>& cm);

  /**
   * Method to remove a component manager object from the component registry
   * if it exists in the registry. If the provided component manager is not
   * in the registry, the registry is left unchanged.
   *
   * \param cm is the {@link ComponentManager} object which will be removed
   *        from the registry
   */
  virtual void RemoveComponentManager(const std::shared_ptr<ComponentManager>& cm);

  /**
   * Removes all entries from the component registry
   */
  void Clear();

  /**
   * Returns the number of elements in the component regsitry
   */
  size_t Count() const;
private:
  std::map<std::pair<unsigned long,std::string>,std::shared_ptr<ComponentManager>> mComponentsByName;
  mutable std::mutex mMapsMutex;
};
} // scrimpl
} // cppmicroservices

#endif // __COMPONENT_REGISTRY_HPP__
