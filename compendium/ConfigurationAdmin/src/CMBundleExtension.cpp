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

#include "cppmicroservices/Bundle.h"

#include "CMBundleExtension.hpp"
#include "CMConstants.hpp"
#include "metadata/ConfigurationMetadata.hpp"
#include "metadata/MetadataParser.hpp"
#include "metadata/MetadataParserFactory.hpp"

namespace cppmicroservices::cmimpl
{
    CMBundleExtension::CMBundleExtension(cppmicroservices::BundleContext context,
                                         cppmicroservices::AnyMap const& cmMetadata,
                                         std::shared_ptr<ConfigurationAdminPrivate> configAdmin,
                                         std::shared_ptr<cppmicroservices::logservice::LogService> lggr)
        : bundleContext(std::move(context))
        , configAdminImpl(std::move(configAdmin))
        , logger(std::move(lggr))
    {
        if (!bundleContext || !configAdminImpl || !logger || cmMetadata.empty())
        {
            throw std::invalid_argument("Invalid parameters passed to CMBundleExtension constructor");
        }

        if (0u == cmMetadata.count(CMConstants::CM_VERSION))
        {
            throw std::runtime_error(std::string("Metadata is missing mandatory '") + CMConstants::CM_VERSION
                                     + "' property");
        }
        auto version = cppmicroservices::any_cast<int>(cmMetadata.at(CMConstants::CM_VERSION));
        auto metadataParser = metadata::MetadataParserFactory::Create(version, logger);
        auto configurationMetadata = metadataParser->ParseAndGetConfigurationMetadata(cmMetadata);

        pidsAndChangeCountsAndIDs = configAdminImpl->AddConfigurations(std::move(configurationMetadata));

        logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                    "Created instance of CMBundleExtension for " + bundleContext.GetBundle().GetSymbolicName());
    }

    CMBundleExtension::~CMBundleExtension()
    {
        try
        {
            logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                        "Deleting instance of CMBundleExtension for " + bundleContext.GetBundle().GetSymbolicName());
            configAdminImpl->RemoveConfigurations(pidsAndChangeCountsAndIDs);
        }
        catch (std::exception const&)
        {
            logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                        "Exception thrown while destroying CMBundleExtension object",
                        std::current_exception());
        }
    };
} // namespace cppmicroservices::cmimpl
