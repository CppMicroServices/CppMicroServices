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

#ifndef CppMicroServices_service_CM_ConfigurationListener_hpp
#define CppMicroServices_service_CM_ConfigurationListener_hpp

#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            /**
             \defgroup gr_configurationlistener ConfigurationListener
             \brief Groups ConfigurationListener class related symbols.
             */
            /**
             * \addtogroup gr_configurationlistener
             * @{
             * The ConfigurationEventType is passed to a Configuration Listener to
             * identify the type of ConfigurationEvent that has occurred.
             */
            enum class ConfigurationEventType
            {
                CM_UPDATED = 1, ///< The ConfigurationEvent type for when a Configuration object has been updated
                CM_DELETED = 2  ///< The ConfigurationEvent type for when a Configuration object has been removed
            };
            /** @}*/

            /**
             * \ingroup gr_configurationlistener
             *
             * The ConfigurationEvent object is passed to the ConfigurationListener when
             * the configuration object for any service is updated or removed by ConfigurationAdmin
             */
            class ConfigurationEvent
            {
              public:
                ConfigurationEvent(ServiceReference<ConfigurationAdmin> configAdmin,
                                   const ConfigurationEventType type,
                                   std::string factoryPid,
                                   std::string pid)
                    : configAdmin(std::move(configAdmin))
                    , type(type)
                    , factoryPid(std::move(factoryPid))
                    , pid(std::move(pid))
                {
                }

                /**
                 * Get the ServiceReference object of the Configuration Admin Service that created
                 * this event.
                 *
                 * @return the service reference of this ConfigurationEvent
                 */
                ServiceReference<ConfigurationAdmin> const&
                getReference() const noexcept
                {
                    return configAdmin;
                }
                /**
                 * Get the PID of this ConfigurationEvent.
                 *
                 * @return the PID of this ConfigurationEvent
                 */
                std::string const&
                getPid() const noexcept
                {
                    return pid;
                }
                /**
                 * Get the Factory PID which is responsible for this Configuration.
                 *
                 * @return the FactoryPID of this ConfigurationEvent
                 */
                std::string const&
                getFactoryPid() const noexcept
                {
                    return factoryPid;
                }
                /**
                 * Get the type of this Configuration.
                 *
                 * @return the ConfigurationEventType of this ConfigurationEvent
                 */
                ConfigurationEventType
                getType() const noexcept
                {
                    return type;
                }

              private:
                ServiceReference<ConfigurationAdmin> const configAdmin;
                const ConfigurationEventType type;
                const std::string factoryPid;
                const std::string pid;
            };

            /**
             * \ingroup gr_configurationlistener
             *
             * Listener for Configuration Events. When a ConfigurationEvent is fired, it is asynchronously
             * delivered to all ConfigurationListeners.
             * 
             * ConfigurationListener objects are registered with the Framework service registry and are
             * notified with a ConfigurationEvent object when an event is fired.
             * 
             * ConfigurationListener objects can inspect the received ConfigurationEvent object to determine
             * its type, the pid of the Configuration object with which it is associated, and the
             * Configuration Admin service that fired the event.
             * 
             * ConfigurationAdmin sends updates for all configuration object updates.
             */
            class ConfigurationListener
            {
              public:
                /**
                 * Called whenever the Configuration for any service is updated or removed from ConfigurationAdmin.
                 *
                 * @param ConfigurationEvent object containing the ConfigurationAdmin service reference
                 * of the ConfigurationAdmin service that updated the configuration object,
                 * the PID or FactoryPid for the configuration object and the type of the update
                 * operation (update or remove)
                 *
                 * @remarks This class is threadsafe
                 */
                virtual void configurationEvent(ConfigurationEvent const& event) = 0;
                virtual ~ConfigurationListener() noexcept = default;
            };
        } // namespace cm
    }     // namespace service
} // namespace cppmicroservices
#endif /* CppMicroServices_service_CM_ConfigurationListener_hpp */
