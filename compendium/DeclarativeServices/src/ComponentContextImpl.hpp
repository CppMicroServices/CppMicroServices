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

#ifndef __COMPONENT_CONTEXT_IMPL_HPP__
#define __COMPONENT_CONTEXT_IMPL_HPP__

#include <string>
#include <memory>
#include <unordered_map>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4503)
#endif

#include "cppmicroservices/Any.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "manager/ComponentConfiguration.hpp"

namespace cppmicroservices {
namespace scrimpl {

class ComponentContextImpl
  : public cppmicroservices::service::component::ComponentContext
{
public:
  /**
   * Constructor used for singleton component configuration
   */
  explicit ComponentContextImpl(std::weak_ptr<ComponentConfiguration> cm);

  /**
   * Constructor used for bundle or prototype component configurations
   */
  ComponentContextImpl(std::weak_ptr<ComponentConfiguration> cm,
                       cppmicroservices::Bundle usingBundle);
  ComponentContextImpl(const ComponentContextImpl&) = delete;
  ComponentContextImpl(ComponentContextImpl&&) = delete;
  ComponentContextImpl& operator=(const ComponentContextImpl&) = delete;
  ComponentContextImpl& operator=(ComponentContextImpl&&) = delete;
  ~ComponentContextImpl() override = default;

  /**
   * Returns the component properties for this Component Context.
   *
   * \return a map of string and cppmicroservices::Any key-value pairs
   * \throws {@link ComponentException} if this {@link ComponentContext} is invalid
   */
  std::unordered_map<std::string, cppmicroservices::Any> GetProperties() const override;

  /**
   * Returns the service object for the specified reference name.
   *
   * <p>
   * If the cardinality of the reference is \c 0..n or \c 1..n and
   * multiple services are bound to the reference, the service with the
   * highest ranking (as specified in its {@link Constants.SERVICE_RANKING}
   * property) is returned. If there is a tie in ranking, the service with the
   * lowest service id (as specified in its {@link Constants.SERVICE_ID}
   * property); that is, the service that was registered first is returned.
   *
   * \param name The name of a reference as specified in a \c reference
   *        element in this component's description.
   * \param type The fully qualified interface name of the service being located.
   * \return A service object for the referenced service or \c nullptr if
   *         the reference cardinality is \c 0..1 or \c 0..n and no
   *         bound service is available.
   * \throws {@link ComponentException} if Service Component Runtime catches an
   *         exception while activating the bound service or if this
   *         {@link ComponentContext} is invalid
   */
  std::shared_ptr<void>  LocateService(const std::string& name, const std::string& type) const override;

  /**
   * Returns the service objects for the specified reference name.
   *
   * \param name The name of a reference as specified in a \c reference
   *        element in this component's description.
   * \param type is the fully qualified interface name of the service being located.
   * \return A vector of service objects for the referenced service. The
   *         returned vector is empty if the reference cardinality is \c 0..1
   *         or \c 0..n and no bound service is available. If the reference
   *         cardinality is \c 0..1 or \c 1..1 and a bound service
   *         is available, the vector will have exactly one element.
   * \throws {@link ComponentException} if Service Component Runtime catches an
   *         exception while activating a bound service or if this
   *         {@link ComponentContext} is invalid
   */
  std::vector<std::shared_ptr<void>> LocateServices(const std::string& name, const std::string& type) const override;

  /**
   * Returns the {@link BundleContext} of the bundle which contains this
   * component. Returns an invalid object if this {@link ComponentContext} is invalid.
   */
  cppmicroservices::BundleContext GetBundleContext() const override;

  /**
   * If the component instance is registered as a service using the
   * \c servicescope="bundle" or \c servicescope="prototype"
   * attribute, then this method returns the bundle using the service provided
   * by the component instance.
   * <p>
   * This method will return invalid {@link Bundle} object if:
   * <ul>
   * <li>The component instance is not a service, then no bundle can be using
   * it as a service.</li>
   * <li>The component instance is a service but did not specify the
   * \c servicescope="bundle" or \c servicescope="prototype"
   * attribute, then all bundles using the service provided by the component
   * instance will share the same component instance.</li>
   * <li>The service provided by the component instance is not currently being
   * used by any bundle.</li>
   * </ul>
   *
   * \return The bundle using the component instance as a service or
   *         invalid {@link Bundle}.
   */
  cppmicroservices::Bundle GetUsingBundle() const override;

  /**
   * Enables the specified component name. The specified component name must
   * be in the same bundle as this component.
   *
   * <p>
   * This method must return after changing the enabled state of the specified
   * component name. Any actions that result from this, such as activating or
   * deactivating a component configuration, must occur asynchronously to this
   * method call.
   *
   * If the \c name is empty, all components in this bundle associated with
   * the context object are enabled
   *
   * \param name The name of a component.
   * \throws std::out_of_range exception if the component with \c name is
   *         not found in component registry
   *         {@link ComponentException} if this {@link ComponentContext} is invalid
   */
  void EnableComponent(const std::string& name) override;

  /**
   * Disables the specified component name. The specified component name must
   * be in the same bundle as this component.
   *
   * <p>
   * This method must return after changing the enabled state of the specified
   * component name. Any actions that result from this, such as activating or
   * deactivating a component configuration, must occur asynchronously to this
   * method call.
   *
   * \param name The name of a component.
   * \throws std::out_of_range exception if the component with \c name is
   *         not found in component registry
   *         {@link ComponentException} if this {@link ComponentContext} is invalid
   */
  void DisableComponent(const std::string& name) override;

  /**
   * If the component instance is registered as a service using the
   * \c service element, then this method returns the service reference
   * of the service provided by this component instance.
   * <p>
   * This method will return an invalid {@link ServiceReference} if the component
   * instance is not registered as a service.
   *
   * \return The {@link ServiceReference} object for the component instance or
   *         an invalid {@link ServiceReference} if the component instance is not
   *         registered as a service.
   */
  cppmicroservices::ServiceReferenceBase GetServiceReference() const override;

  /**
   * Removes the link to Declarative Services Runtime. Any attempt to call
   * methods on this object after it has been invalidated will result in an
   * exception.
   */
  void Invalidate();

private:
  /**
   * Returns the Id of the bundle containing the component
   *
   * \throws {@link ComponentException} if this {@link ComponentContext} is invalid
   */
  unsigned long GetBundleId() const;

  void InitializeServicesCache();

  std::weak_ptr<ComponentConfiguration> configManager;
  cppmicroservices::Bundle usingBundle;
  std::unordered_map<std::string, std::vector<cppmicroservices::InterfaceMapConstPtr>> boundServicesCache;
};
}
}
#endif
