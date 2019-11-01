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

#ifndef __SINGLETONCOMPONENTCONFIGURATION_HPP__
#define __SINGLETONCOMPONENTCONFIGURATION_HPP__

#include "ComponentConfigurationImpl.hpp"
#include "ConcurrencyUtil.hpp"

namespace cppmicroservices {
namespace scrimpl {

/**
 * This class implements the ComponentConfigurationImpl interface. It is responsible for managing the
 * singleton instance of the {@link ComponentInstance}. It also implements the ServiceFactory pattern
 * to handle {@link ServiceFactory#GetService} calls from the user.
 */
class SingletonComponentConfigurationImpl final
  : public ComponentConfigurationImpl
  , public cppmicroservices::ServiceFactory
{
public:
  explicit SingletonComponentConfigurationImpl(std::shared_ptr<const metadata::ComponentMetadata> metadata,
                                               const cppmicroservices::Bundle& bundle,
                                               std::shared_ptr<const ComponentRegistry> registry,
                                               std::shared_ptr<cppmicroservices::logservice::LogService> logger);
  SingletonComponentConfigurationImpl(const SingletonComponentConfigurationImpl&) = delete;
  SingletonComponentConfigurationImpl(SingletonComponentConfigurationImpl&&) = delete;
  SingletonComponentConfigurationImpl& operator=(const SingletonComponentConfigurationImpl&) = delete;
  SingletonComponentConfigurationImpl& operator=(SingletonComponentConfigurationImpl&&) = delete;
  ~SingletonComponentConfigurationImpl() override;

  /**
   * Returns the this object as a ServiceFactory object
   */
  std::shared_ptr<cppmicroservices::ServiceFactory> GetFactory() override;

  /**
   * Returns a new created {@link ComponentInstance} object after activating it
   * Calling this method multiple times, will return the same object.
   *
   * \param the bundle making the service request
   */
  std::shared_ptr<ComponentInstance> CreateAndActivateComponentInstance(const cppmicroservices::Bundle& bundle) /* noexcept */ override;

  /**
   * Method removes the singleton {@link ComponentInstance} object created by this object.
   */
  void DestroyComponentInstances() /* noexcept */ override;

  /**
   * Implements the {@link ServiceFactory#GetService} interface. This method
   * wraps the service implementation object in an {@link InterfaceMapConstPtr}
   * This method always returns the same service implementation object.
   * A nullptr is returned if a service instance cannot be created or activated.
   */
  cppmicroservices::InterfaceMapConstPtr GetService(const cppmicroservices::Bundle& bundle,
                                                    const cppmicroservices::ServiceRegistrationBase& registration) override;

  /**
   * Implements the {@link ServiceFactory#UngetService} interface. No-op since the
   * service is a \c shared_ptr
   */
  void UngetService(const cppmicroservices::Bundle& bundle,
                    const cppmicroservices::ServiceRegistrationBase& registration,
                    const cppmicroservices::InterfaceMapConstPtr& service) override;
private:
  FRIEND_TEST(SingletonComponentConfigurationTest, TestConcurrentCreateAndActivateComponentInstance);
  FRIEND_TEST(SingletonComponentConfigurationTest, TestCreateAndActivateComponentInstance);
  FRIEND_TEST(SingletonComponentConfigurationTest, TestDestroyComponentInstances);
  FRIEND_TEST(SingletonComponentConfigurationTest, TestGetService);
  FRIEND_TEST(SingletonComponentConfigurationTest, TestDestroyComponentInstances_DeactivateFailure);

  /**
   * Set the member data, only used in tests
   */
  void SetComponentInstancePair(InstanceContextPair instCtxtPair);

  /**
   * Get the component context associated with this configuration
   */
  std::shared_ptr<ComponentContextImpl> GetComponentContext();

  /**
   * Get the component instance associated with this configuration
   */
  std::shared_ptr<ComponentInstance> GetComponentInstance();

  Guarded<InstanceContextPair> data; ///< singleton pair of component instance and context associated with this configuration
};
}
}

#endif /* __SINGLETONCOMPONENTCONFIGURATION_HPP__ */
