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

#ifndef ServiceComponentRuntime_hpp
#define ServiceComponentRuntime_hpp

#include <vector>
#include <future>

#include <cppmicroservices/Bundle.h>

#include "dto/ComponentDescriptionDTO.hpp"
#include "dto/ComponentConfigurationDTO.hpp"
#include "cppmicroservices/servicecomponent/ServiceComponentExport.h"

namespace cppmicroservices { namespace service { namespace component { namespace runtime {

/**
 * The {@code ServiceComponentRuntime} service represents the Declarative
 * Services actor, known as Service Component Runtime (SCR), that manages the
 * service components and their life cycle. The {@code ServiceComponentRuntime}
 * service allows introspection of the components managed by Service Component
 * Runtime.
 *
 * <p>
 * This service differentiates between a {@link ComponentDescriptionDTO} and a
 * {@link ComponentConfigurationDTO}. A {@link ComponentDescriptionDTO} is a
 * representation of a declared component description. A
 * {@link ComponentConfigurationDTO} is a representation of an actual instance
 * of a declared component description parameterized by component properties.
 */
class US_ServiceComponent_EXPORT ServiceComponentRuntime {
  public:
  virtual ~ServiceComponentRuntime() noexcept;

  /**
   * Returns the component descriptions declared by the specified active
   * bundles.
   *
   * <p>
   * Only component descriptions from active bundles are returned. If the
   * specified bundles have no declared components or are not active, an empty
   * collection is returned.
   *
   * @param bundles The bundles whose declared component descriptions are to
   *        be returned. Specifying no bundles, or the equivalent of an empty
   *        {@code Bundle} vector, will return the declared component
   *        descriptions from all active bundles.
   * @return The declared component descriptions of the specified active
   *         {@code bundles}. An empty collection is returned if there are no
   *         component descriptions for the specified active bundles.
   */
  virtual std::vector<dto::ComponentDescriptionDTO> GetComponentDescriptionDTOs(const std::vector<cppmicroservices::Bundle>& bundles = {}) const = 0;

  /**
   * Returns the {@link ComponentDescriptionDTO} declared with the specified name
   * by the specified bundle.
   *
   * <p>
   * Only component descriptions from active bundles are returned.
   * An empty object is returned if no such component is declared by the given
   * {@code bundle} or the bundle is not active.
   *
   * @param bundle The bundle declaring the component description.
   * @param name The name of the component description.
   * @return The declared component description or empty object if the
   *         specified bundle is not active or does not declare a component
   *         description with the specified name.
   */
  virtual dto::ComponentDescriptionDTO GetComponentDescriptionDTO(const cppmicroservices::Bundle& bundle, const std::string& name) const = 0;

  /**
   * Returns the component configurations for the specified component
   * description.
   *
   * @param description The component description.
   * @return A vector containing a snapshot of the current component
   *         configurations for the specified component description. An empty
   *         vector is returned if there are none.
   */
  virtual std::vector<dto::ComponentConfigurationDTO> GetComponentConfigurationDTOs(const dto::ComponentDescriptionDTO& description) const = 0;

  /**
   * Returns whether the specified component description is currently enabled.
   *
   * <p>
   * The enabled state of a component description is initially set by the
   * {@link ComponentDescriptionDTO#defaultEnabled enabled} attribute of the
   * component description.
   *
   * @param description The component description.
   * @return {@code true} if the specified component description is currently
   *         enabled. Otherwise, {@code false}.
   * @see #EnableComponent(ComponentDescriptionDTO)
   * @see #DisableComponent(ComponentDescriptionDTO)
   * @see ComponentContext#DisableComponent(std::string)
   * @see ComponentContext#EnableComponent(std::string)
   */
  virtual bool IsComponentEnabled(const dto::ComponentDescriptionDTO& description) const = 0;

  /**
   * Enables the specified component description.
   *
   * <p>
   * If the specified component description is currently enabled, this method
   * has no effect.
   *
   * <p>
   * This method must return after changing the enabled state of the specified
   * component description. Any actions that result from this, such as
   * activating or deactivating a component configuration, must occur
   * asynchronously to this method call.
   *
   * @param description The component description to enable.
   * @return A future that will be ready when the actions that result from
   *         changing the enabled state of the specified component have
   *         completed.
   * @see #IsComponentEnabled(ComponentDescriptionDTO)
   */
  virtual std::shared_future<void> EnableComponent(const dto::ComponentDescriptionDTO& description) = 0;

  /**
   * Disables the specified component description.
   *
   * <p>
   * If the specified component description is currently disabled, this method
   * has no effect.
   *
   * <p>
   * This method must return after changing the enabled state of the specified
   * component description. Any actions that result from this, such as
   * activating or deactivating a component configuration, must occur
   * asynchronously to this method call.
   *
   * @param description The component description to disable.
   * @return A future that will be ready when the actions that result from
   *         changing the enabled state of the specified component have
   *         completed.
   * @see #IsComponentEnabled(ComponentDescriptionDTO)
   */
  virtual std::shared_future<void> DisableComponent(const dto::ComponentDescriptionDTO& description) = 0;
};

}}}} // namespaces


#endif /* ServiceComponentRuntime_hpp */
