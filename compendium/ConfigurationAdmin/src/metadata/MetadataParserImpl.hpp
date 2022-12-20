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

#ifndef METADATAPARSERIMPL_HPP
#define METADATAPARSERIMPL_HPP

#include "cppmicroservices/logservice/LogService.hpp"

#include "MetadataParser.hpp"

namespace cppmicroservices
{
    namespace cmimpl
    {
        namespace metadata
        {
            /*
             * Represents a concrete implementation (Version 1) of the MetadataParser
             */
            class MetadataParserImplV1 final : public MetadataParser
            {
              public:
                MetadataParserImplV1(std::shared_ptr<cppmicroservices::logservice::LogService> logger);

                ~MetadataParserImplV1() override = default;
                MetadataParserImplV1(MetadataParserImplV1 const&) = delete;
                MetadataParserImplV1& operator=(MetadataParserImplV1 const&) = delete;
                MetadataParserImplV1(MetadataParserImplV1&&) = delete;
                MetadataParserImplV1& operator=(MetadataParserImplV1&&) = delete;

                /*
                 * @brief Parse and return the vector of ComponentMetadata
                 * @param metadata The value of the key "cm" in the manifest
                 * @returns the vector of @ConfigurationMetadata objects
                 */
                std::vector<ConfigurationMetadata> ParseAndGetConfigurationMetadata(
                    AnyMap const& scrmap) const override;

              private:
                std::shared_ptr<cppmicroservices::logservice::LogService> logger;
            };
        } // namespace metadata
    }     // namespace cmimpl
} // namespace cppmicroservices

#endif // METADATAPARSERIMPL_HPP
