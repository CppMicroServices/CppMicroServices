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

#ifndef __BUNDLEORPROTOTYPECOMPONENTCONFIGURATIONIMPL_HPP__
#define __BUNDLEORPROTOTYPECOMPONENTCONFIGURATIONIMPL_HPP__

#include "ComponentConfigurationImpl.hpp"
#include "ConcurrencyUtil.hpp"
#include <cppmicroservices/ServiceFactory.h>

namespace cppmicroservices {
namespace scrimpl {
/**
 * This class implements the ComponentConfigurationImpl interface. It is responsible for managing the
 * singleton instance of the {@link ComponentInstance}. It also implements the ServiceFactory pattern
 * to handle {@link ServiceFactory#GetService} calls from the user.
 */
class BundleOrPrototypeComponentConfigurationImpl final
  : public ComponentConfigurationImpl
  , public cppmicroservices::ServiceFactory
{
public:
  explicit BundleOrPrototypeComponentConfigurationImpl(std::shared_ptr<const metadata::ComponentMetadata> metadata,
                                                       const cppmicroservices::Bundle& bundle,
                                                       std::shared_ptr<const ComponentRegistry> registry,
                                                       std::shared_ptr<cppmicroservices::logservice::LogService> logger);
  BundleOrPrototypeComponentConfigurationImpl(const BundleOrPrototypeComponentConfigurationImpl&) = delete;
  BundleOrPrototypeComponentConfigurationImpl(BundleOrPrototypeComponentConfigurationImpl&&) = delete;
  BundleOrPrototypeComponentConfigurationImpl& operator=(const BundleOrPrototypeComponentConfigurationImpl&) = delete;
  BundleOrPrototypeComponentConfigurationImpl& operator=(BundleOrPrototypeComponentConfigurationImpl&&) = delete;
  ~BundleOrPrototypeComponentConfigurationImpl() override;

  /**
   * Returns the this object as a ServiceFactory object
   */
  std::shared_ptr<cppmicroservices::ServiceFactory> GetFactory() override;

  /**
   * Returns a newly created {@link ComponentInstance} object after activating it
   *
   * \param the bundle making the service request
   */
  std::shared_ptr<ComponentInstance> CreateAndActivateComponentInstance(const cppmicroservices::Bundle& bundle) override;


  /**
   * Method removes all instances of {@link ComponentInstance} object created by this object.
   */
  void DestroyComponentInstances() override;

  /**
   * Implements the {@link ServiceFactory#GetService} interface. This method
   * wraps the service implementation object in an {@link InterfaceMapConstPtr}
   * This method always returns the same service implementation object.
   * A nullptr is returned if a service instance cannot be created or activated.
   */
  cppmicroservices::InterfaceMapConstPtr GetService(const cppmicroservices::Bundle& bundle,
                                                    const cppmicroservices::ServiceRegistrationBase& registration) override;

  /**
   * Implements the {@link ServiceFactory#UngetService} interface.
   */
  void UngetService(const cppmicroservices::Bundle& bundle,
                    const cppmicroservices::ServiceRegistrationBase& registration,
                    const cppmicroservices::InterfaceMapConstPtr& service) override;
private:

  /**
   * Helper method to deactivate the component instance and invalidate the
   * associated context object
   */
  void DeactivateComponentInstance(const InstanceContextPair& instCtxt);

  Guarded<std::vector<InstanceContextPair>> compInstanceMap; ///< map of component instance and context objects associated with this configuration
};
}
}
#endif /* __BUNDLEORPROTOTYPECOMPONENTCONFIGURATIONIMPL_HPP__ */
