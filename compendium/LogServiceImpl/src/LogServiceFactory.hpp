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

#ifndef CPPMICROSERVICES_LOGSERVICEFACTORY_HPP
#define CPPMICROSERVICES_LOGSERVICEFACTORY_HPP

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/ServiceFactory.h"

#include <string>

namespace cppmicroservices::logservice::impl
{
    /**
     * Creates one LogService per bundle that obtains the service. The instance logs to the
     * logger named "LogService.<bsn>", where <bsn> is the symbolic name of that bundle.
     */
    class LogServiceFactory final : public cppmicroservices::ServiceFactory
    {
      public:
        /**
         * Returns the name of the logger used by the LogService handed to \c bundle.
         */
        static std::string GetLoggerName(cppmicroservices::Bundle const& bundle);

        InterfaceMapConstPtr GetService(cppmicroservices::Bundle const& bundle,
                                        cppmicroservices::ServiceRegistrationBase const& registration) override;

        void UngetService(cppmicroservices::Bundle const& bundle,
                          cppmicroservices::ServiceRegistrationBase const& registration,
                          InterfaceMapConstPtr const& service) override;
    };
} // namespace cppmicroservices::logservice::impl

#endif // CPPMICROSERVICES_LOGSERVICEFACTORY_HPP
