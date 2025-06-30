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

#include <vector>

#include "cppmicroservices/Any.h"

#include "ConfigurationMetadata.hpp"
#include "MetadataParserImpl.hpp"

namespace cppmicroservices::cmimpl::metadata
{

    MetadataParserImplV1::MetadataParserImplV1(std::shared_ptr<cppmicroservices::logservice::LogService> lggr)
        : logger(std::move(lggr))
    {
    }

    std::vector<ConfigurationMetadata>
    MetadataParserImplV1::ParseAndGetConfigurationMetadata(cppmicroservices::AnyMap const& cmMap) const
    {
        std::vector<ConfigurationMetadata> configurationMetadata;

        // Will throw if the object doesn't exist or has the wrong type
        if (0u == cmMap.count("configurations"))
        {
            throw std::runtime_error("Metadata is missing mandatory 'configurations' property");
        }
        auto& configurations
            = cppmicroservices::ref_any_cast<std::vector<cppmicroservices::Any>>(cmMap.at("configurations"));

        std::size_t index = 0;
        for (auto const& configurationAny : configurations)
        {
            // Allow exception to escape if any Configuration isn't an AnyMap
            auto& config = cppmicroservices::ref_any_cast<cppmicroservices::AnyMap>(configurationAny);
            try
            {
                auto pid = cppmicroservices::any_cast<std::string>(config.at("pid"));
                auto properties = cppmicroservices::any_cast<cppmicroservices::AnyMap>(config.at("properties"));
                configurationMetadata.emplace_back(std::move(pid), std::move(properties));
            }
            catch (std::exception const& ex)
            {
                auto msg = std::string(ex.what());
                msg += " Could not load the configuration with index: " + std::to_string(index);
                logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, msg);
            }
            ++index;
        }
        return configurationMetadata;
    }
} // namespace cppmicroservices::cmimpl::metadata
