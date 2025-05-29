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

#ifndef BUNDLEORPROTOTYPECOMPONENTCONFIGURATIONIMPL_HPP
#define BUNDLEORPROTOTYPECOMPONENTCONFIGURATIONIMPL_HPP

#include "ComponentConfigurationImpl.hpp"
#include "cppmicroservices/GuardedObject.h"
#include <cppmicroservices/ServiceFactory.h>

namespace cppmicroservices
{
    namespace scrimpl
    {
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
            explicit BundleOrPrototypeComponentConfigurationImpl(
                std::shared_ptr<const metadata::ComponentMetadata> metadata,
                cppmicroservices::Bundle const& bundle,
                std::shared_ptr<ComponentRegistry> registry,
                std::shared_ptr<cppmicroservices::logservice::LogService> logger,
                std::shared_ptr<ConfigurationNotifier> configNotifier);
            BundleOrPrototypeComponentConfigurationImpl(BundleOrPrototypeComponentConfigurationImpl const&) = delete;
            BundleOrPrototypeComponentConfigurationImpl(BundleOrPrototypeComponentConfigurationImpl&&) = delete;
            BundleOrPrototypeComponentConfigurationImpl& operator=(BundleOrPrototypeComponentConfigurationImpl const&)
                = delete;
            BundleOrPrototypeComponentConfigurationImpl& operator=(BundleOrPrototypeComponentConfigurationImpl&&)
                = delete;
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
            std::shared_ptr<ComponentInstance> CreateAndActivateComponentInstance(
                cppmicroservices::Bundle const& bundle) override;

            /**
             * Method called to modify the configuration properties for this component configuration.
             * @return false if the component instance has not provided a Modified method.
             */
            bool ModifyComponentInstanceProperties() override;

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
            cppmicroservices::InterfaceMapConstPtr GetService(
                cppmicroservices::Bundle const& bundle,
                cppmicroservices::ServiceRegistrationBase const& registration) override;

            /**
             * Implements the {@link ServiceFactory#UngetService} interface.
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
            /**
             * Helper method to deactivate the component instance and invalidate the
             * associated context object
             */
            void DeactivateComponentInstance(InstanceContextPair const& instCtxt);

            cppmicroservices::Guarded<std::vector<InstanceContextPair>>
                compInstanceMap; ///< map of component instance and context objects associated with this configuration
        };
    } // namespace scrimpl
} // namespace cppmicroservices
#endif /* BUNDLEORPROTOTYPECOMPONENTCONFIGURATIONIMPL_HPP */
