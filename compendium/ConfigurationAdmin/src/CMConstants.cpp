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

#include <string>

#include "CMConstants.hpp"

        /**
         * Defines standard names for CM Constants.
         */
namespace cppmicroservices::cmimpl::CMConstants
{
    /**
     * Manifest key specifying the CM configuration object.
     * <p>
     * The attribute value may be retrieved from the {@code cppmicroservices::AnyMap}
     * object returned by the {@code Bundle.GetHeaders} method using {@code AnyMap.at}
     */
    const std::string CM_KEY = "cm";

    /**
     * Manifest key specifying the version of the CM configuration.
     * <p>
     * The attribute value may be retrieved from the {@code cppmicroservices::AnyMap}
     * object associated with the CM_KEY entry.
     */
    const std::string CM_VERSION = "version";

    /**
     * Top level service property to declare the PID of a ManagedService or
     * ManagedServiceFactory. If found, CM_SERVICE_SUBKEY could yield the PID.
     */
    const std::string CM_SERVICE_KEY = "service";

    /**
     * Subkey to obtain the PID
     */
    const std::string CM_SERVICE_SUBKEY = "pid";

    /**
     * Top level property added by DeclarativeServices, use as fallback if no service.pid
     * property can be found for a ManagedService or ManagedServiceFactory. If found,
     * CM_COMPONENT_SUBKEY could yield an alternative PID.
     */
    const std::string CM_COMPONENT_KEY = "component";

    /**
     * Subkey to obtain an alternative PID
     */
    const std::string CM_COMPONENT_SUBKEY = "name";

} // namespace cppmicroservices::cmimpl::CMConstants
