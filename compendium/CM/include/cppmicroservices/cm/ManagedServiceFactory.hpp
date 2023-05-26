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

#ifndef CppMicroServices_CM_ManagedServiceFactory_hpp
#define CppMicroServices_CM_ManagedServiceFactory_hpp

#include <string>

#include "cppmicroservices/AnyMap.h"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {

            /**
             \defgroup gr_managedservicefactory ManagedServiceFactory
             \brief Groups ManagedServiceFactory class related symbols.
             */
            /**
             * \ingroup gr_managedservicefactory
             *
             * The ManagedServiceFactory interface is the interface that service factories
             * should implement to receive updates from the ConfigurationAdmin implementation
             * with the Configurations for the services that the Factory will provide. The
             * service.factoryPid ServiceProperty of the ManagedServiceFactory (or the
             * component.name which is injected by DeclarativeServices if no service.factoryPid
             * is provided) will be used to select all Configurations applicable to this factory.
             * Per the OSGi Spec, the ManagedServiceFactory is intended to be used when multiple
             * instances of a given Service will exist in the Framework, but each with different
             * Configurations. For this reason, ManagedServiceFactory implementations are encouraged
             * to mirror any properties (excluding security critical properties) into the
             * ServiceProperty map when publishing the service with the Framework (if it is going to
             * be published). Clients of the service can then filter on the properties they require.
             *
             * <p>
             * The Updated method will be invoked once for each initial Configuration for this
             * factory and then again whenever any of those Configurations change or whenever any
             * new Configurations for this factory are added.</p>
             *
             * <p>
             * The Removed method will be invoked whenever any Configuration which this factory
             * has previously been configured with is removed from the ConfigurationAdmin implementation.</p>
             */
            class ManagedServiceFactory
            {
              public:
                virtual ~ManagedServiceFactory() noexcept = default;

                /**
                 * Called whenever any Configuration for this service factory is updated with ConfigurationAdmin,
                 * and (possibly multiple times) when the ManagedServiceFactory is first registered with the Framework.
                 *
                 * Can throw a ConfigurationException if there's a problem with the properties. This exception will be
                 * logged by the ConfigurationAdminImpl to aid the application author's investigation into the incorrect
                 * configuration.
                 *
                 * Will be called asynchronously from the Service registration or an update to the Configuration.
                 *
                 * The ManagedServiceFactory should update the corresponding service instance with the properties
                 * provided, and potentially update the properties of that service's registration with the Framework (if
                 * it is registered).
                 *
                 * If a corresponding service instance does not exist for this pid, the ManagedServiceFactory should
                 * create one with the properties provided. It should also register that new service with the Framework,
                 * if applicable.
                 *
                 * @throws ConfigurationException if something is wrong with the properties provided.
                 *
                 * @param pid The unique pid for this Configuration, of the form "$FACTORY_PID~$INSTANCE_NAME"
                 * @param properties The properties for this Configuration
                 */
                virtual void Updated(std::string const& pid, AnyMap const& properties) = 0;

                /**
                 * Called whenever one of the Configurations for this service is removed from ConfigurationAdmin.
                 *
                 * Will be called asynchronously from the removal of the Configuration.
                 *
                 * The ManagedServiceFactory should remove the corresponding service instance, and if it is registered
                 * with the Framework, it should be unregistered.
                 *
                 * @param pid The unique pid for the Configuration to remove, of the form "$FACTORY_PID~$INSTANCE_NAME"
                 */
                virtual void Removed(std::string const& pid) = 0;
            };
        } // namespace cm
    }     // namespace service
} // namespace cppmicroservices
#endif /* ManagedServiceFactory_hpp */
