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

#ifndef METADATAPARSER_HPP
#define METADATAPARSER_HPP

#include "ConfigurationMetadata.hpp"

namespace cppmicroservices
{
    namespace cmimpl
    {
        namespace metadata
        {

            /*
             * @brief Represents an abstract metadata parser.
             *
             * Whenever there is a change in the service description JSON file-format:
             *  1. The "version" field in the manifest should be bumped-up by one.
             *  2. A new concrete class that implements the following
             *     interface is implemented, which would contain the logic to parse
             *     that particular change of the manifest file-format.
             *  3. The MetadataParserFactory is changed to create and return
             *     that implementation for the corresponding version of the manifest.
             * This way, because the CM Bundle Extension depends on the
             * following interface instead of any concrete implementation, it
             * supports multiple versions of the manifest.
             */
            class MetadataParser
            {
              public:
                virtual ~MetadataParser() = default;

                /*
                 * @brief Parses and returns a vector of the configuration metadata
                 * @param anymap An @c AnyMap representation of the configuration properties
                 * @returns a vector of parsed @c ConfigurationMetadata
                 */
                virtual std::vector<ConfigurationMetadata> ParseAndGetConfigurationMetadata(AnyMap const& anymap) const
                    = 0;

              protected:
                MetadataParser() = default;
            };
        } // namespace metadata
    }     // namespace cmimpl
} // namespace cppmicroservices

#endif // METADATAPARSER_HPP
