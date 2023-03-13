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

#ifndef CppMicroServices_CM_ConfigurationAdmin_hpp
#define CppMicroServices_CM_ConfigurationAdmin_hpp

#include "cppmicroservices/cm/Configuration.hpp"

#include <memory>
#include <string>
#include <vector>

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {

            /**
             \defgroup gr_configurationadmin ConfigurationAdmin
             \brief Groups ConfigurationAdmin class related symbols.
             */

            /**
             * \ingroup gr_configurationadmin
             * The ConfigurationAdmin interface is the means by which applications and services can
             * interract with the Configuration objects at runtime. It can be used to create or obtain
             * Configuration objects, which can in turn be queried for their properties or be used to
             * update the properties of a given service or service factory.
             */
            class ConfigurationAdmin
            {
              public:
                virtual ~ConfigurationAdmin() noexcept = default;

                /**
                 * Get an existing or new Configuration object. If the Configuration object for this
                 * PID does not exist, create a new Configuration object for that PID with empty
                 * properties.
                 *
                 * @param pid The PID to get the Configuration for
                 * @return the Configuration object for this PID
                 */
                virtual std::shared_ptr<Configuration> GetConfiguration(std::string const& pid) = 0;

                /**
                 * Create a new Configuration object for a ManagedServiceFactory. The factoryPid is the PID of the
                 * ManagedServiceFactory (which must be different from the PIDs of any services it manages) and the
                 * instanceName will be a randomly-generated, unique name. The Configuration object's properties
                 * are empty. The returned Configuration will have a PID of the form $factoryPid~$instanceName.
                 *
                 * @param factoryPid The Factory PID to create a new Configuration for
                 * @return a new Configuration object for this Factory PID with a randomly-generated unique name
                 */
                virtual std::shared_ptr<Configuration> CreateFactoryConfiguration(std::string const& factoryPid) = 0;

                /**
                 * Get an existing or new Configuration object for a ManagedServiceFactory. The factoryPid is the PID of
                 * the ManagedServiceFactory (which must be different from the PIDs of any services it manages) and the
                 * instanceName is the unique name of one of those managed services. If the Configuration object for
                 * this combination of factoryPid and instanceName does not exist, create a new Configuration object for
                 * that combination, where properties are empty. The returned Configuration will have a PID of the form
                 * $factoryPid~$instanceName.
                 *
                 * @param factoryPid The Factory PID to use to get an existing Configuration or create a new
                 * Configuration for
                 * @param instanceName The unique name of an instance of a serivce managed by the ManagedServiceFactory
                 * @return the Configuration object for this combination of Factory PID and instance name
                 */
                virtual std::shared_ptr<Configuration> GetFactoryConfiguration(std::string const& factoryPid,
                                                                               std::string const& instanceName)
                    = 0;

                /**
                 * Used to list all of the available configurations. An LDAP filter expression can be used to filter
                 * based on any property of the configuration, including service.pid and service.factoryPid
                 *
                 * @param filter An optional filter expression to limit the Configurations which are returned, or empty
                 * for all.
                 * @return a vector of Configurations matching the filter.
                 */
                virtual std::vector<std::shared_ptr<Configuration>> ListConfigurations(std::string const& filter = {})
                    = 0;
            };
        } // namespace cm
    }     // namespace service
} // namespace cppmicroservices
#endif // ConfigurationAdmin_hpp
