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

#ifndef CMBUNDLEEXTENSION_HPP
#define CMBUNDLEEXTENSION_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/logservice/LogService.hpp"

#include "ConfigurationAdminPrivate.hpp"

namespace cppmicroservices
{
    namespace cmimpl
    {
        /**
         * The CMBundleExtension is a helper class to load and unload configurations from
         * a single bundle. It pushes the configurations it finds to the ConfigurationAdmin
         * implementation. On destruction, it removes the configurations created during
         * construction.
         */
        class CMBundleExtension final
        {
          public:
            CMBundleExtension(cppmicroservices::BundleContext bundleContext,
                              cppmicroservices::AnyMap const& cmMetadata,
                              std::shared_ptr<ConfigurationAdminPrivate> configAdminImpl,
                              std::shared_ptr<cppmicroservices::logservice::LogService> logger);
            CMBundleExtension(CMBundleExtension const&) = delete;
            CMBundleExtension(CMBundleExtension&&) = delete;
            CMBundleExtension& operator=(CMBundleExtension const&) = delete;
            CMBundleExtension& operator=(CMBundleExtension&&) = delete;
            ~CMBundleExtension();

          private:
            cppmicroservices::BundleContext bundleContext;
            std::shared_ptr<ConfigurationAdminPrivate> configAdminImpl;
            std::shared_ptr<cppmicroservices::logservice::LogService> logger;
            std::vector<ConfigurationAddedInfo> pidsAndChangeCountsAndIDs;
        };
    } // namespace cmimpl
} // namespace cppmicroservices

#endif // CMBUNDLEEXTENSION_HPP
