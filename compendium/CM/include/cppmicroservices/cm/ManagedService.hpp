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

#ifndef CppMicroServices_CM_ManagedService_hpp
#define CppMicroServices_CM_ManagedService_hpp

#include "cppmicroservices/AnyMap.h"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {

            /**
             \defgroup gr_managedservice ManagedService
             \brief Groups ManagedService class related symbols.
             */

            /**
             * \ingroup gr_managedservice
             *
             * The ManagedService interface is the interface that services should implement
             * to receive updates from the ConfigurationAdmin implementation with their
             * Configuration. The Configuration will be provided based on the service.pid
             * ServiceProperty of the Service being registered with the CppMicroServices
             * Framework. If this isn't provided, the component.name property injected by
             * DeclarativeServices will be used instead. If no Configuration is available
             * for either of these, an empty AnyMap will be used to call Updated. Per the
             * OSGi Spec, ManagedService is intended to be used for services which have
             * singleton scope, where there will only be one implementation of a given
             * interface.
             */
            class ManagedService
            {
              public:
                virtual ~ManagedService() noexcept = default;

                /**
                 * Called whenever the Configuration for this service is updated or removed from ConfigurationAdmin,
                 * and when the ManagedService is first registered with the Framework, to provide the initial
                 * Configuration.
                 *
                 * Can throw a ConfigurationException if there's a problem with the properties. This exception will
                 * be logged by the ConfigurationAdminImpl to aid the application author's investigation into the
                 * incorrect configuration.
                 *
                 * Will be called asynchronously from the Service's registration or an update to the Configuration.
                 *
                 * @throws ConfigurationException if something is wrong with the properties provided.
                 *
                 * @param properties The properties from the new/initial Configuration for this ManagedService
                 */
                virtual void Updated(AnyMap const& properties) = 0;
            };
        } // namespace cm
    }     // namespace service
} // namespace cppmicroservices
#endif /* ManagedService_hpp */
