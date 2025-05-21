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

#ifndef SINGLETONCOMPONENTCONFIGURATION_HPP
#define SINGLETONCOMPONENTCONFIGURATION_HPP

#include "ComponentConfigurationImpl.hpp"
#include "ConfigurationNotifier.hpp"
#include "cppmicroservices/GuardedObject.h"

namespace cppmicroservices
{
    namespace scrimpl
    {

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
            explicit SingletonComponentConfigurationImpl(
                std::shared_ptr<const metadata::ComponentMetadata> metadata,
                cppmicroservices::Bundle const& bundle,
                std::shared_ptr<ComponentRegistry> registry,
                std::shared_ptr<cppmicroservices::logservice::LogService> logger,
                std::shared_ptr<ConfigurationNotifier> configNotifier);
            SingletonComponentConfigurationImpl(SingletonComponentConfigurationImpl const&) = delete;
            SingletonComponentConfigurationImpl(SingletonComponentConfigurationImpl&&) = delete;
            SingletonComponentConfigurationImpl& operator=(SingletonComponentConfigurationImpl const&) = delete;
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
            std::shared_ptr<ComponentInstance> CreateAndActivateComponentInstance(
                cppmicroservices::Bundle const& bundle) /* noexcept */ override;

            /**
             * Method called to modify the configuration properties for this component configuration.
             * @return false if the component instance has not provided a Modified method.
             */
            bool ModifyComponentInstanceProperties() override;

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
            cppmicroservices::InterfaceMapConstPtr GetService(
                cppmicroservices::Bundle const& bundle,
                cppmicroservices::ServiceRegistrationBase const& registration) override;

            /**
             * Implements the {@link ServiceFactory#UngetService} interface. No-op since the
             * service is a \c shared_ptr
             */
            void UngetService(cppmicroservices::Bundle const& bundle,
                              cppmicroservices::ServiceRegistrationBase const& registration,
                              cppmicroservices::InterfaceMapConstPtr const& service) override;

            /**
             * Calls the service component's bind method while performing a dynamic rebind.
             *
             * \param refName is the name of the reference as defined in the SCR JSON
             * \param ref is the service reference to the target service to bind. A default
             *  constructed \c ServiceReferenceBase denotes that there is no service to bind.
             */
            void BindReference(std::string const& refName, ServiceReferenceBase const& ref) override;

            /**
             * Calls the service component's unbind method while performing a dynamic rebind.
             *
             * \param refName is the name of the reference as defined in the SCR JSON
             * \param ref is the service reference to the target service to unbind. A default
             *  constructed \c ServiceReferenceBase denotes that there is no service to unbind.
             */
            void UnbindReference(std::string const& refName, ServiceReferenceBase const& ref) override;

          private:
            FRIEND_TEST(SingletonComponentConfigurationTest, TestConcurrentCreateAndActivateComponentInstance);
            FRIEND_TEST(SingletonComponentConfigurationTest, TestCreateAndActivateComponentInstance);
            FRIEND_TEST(SingletonComponentConfigurationTest, TestDestroyComponentInstances);
            FRIEND_TEST(SingletonComponentConfigurationTest, TestModifiedMethodExceptionLogging);
            FRIEND_TEST(SingletonComponentConfigurationTest, TestGetService);
            FRIEND_TEST(SingletonComponentConfigurationTest, TestDestroyComponentInstances_DeactivateFailure);
            FRIEND_TEST(ConfigAdminComponentCreationRace, TestModifiedIsNeverCalled);
            FRIEND_TEST(ConfigAdminComponentCreationRace, TestMultipleConfigsNoModifiedCall);
            FRIEND_TEST(ConfigAdminComponentCreationRace, TestMultipleConfigsModifiedCalled);
            FRIEND_TEST(ConfigAdminComponentCreationRace, TestConfigNotifierSafeWithNoListenersForPid);

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

            cppmicroservices::Guarded<InstanceContextPair>
                data; ///< singleton pair of component instance and context associated with this configuration
        };
    } // namespace scrimpl
} // namespace cppmicroservices

#endif /* SINGLETONCOMPONENTCONFIGURATION_HPP */
