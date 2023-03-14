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

#ifndef CONFIGURATIONMETADATA_HPP
#define CONFIGURATIONMETADATA_HPP

#include <string>

#include "cppmicroservices/AnyMap.h"

namespace cppmicroservices
{
    namespace cmimpl
    {
        namespace metadata
        {
            /**
             * Stores configuration information parsed from the configuration properties
             */
            struct ConfigurationMetadata final
            {
                ConfigurationMetadata() : pid(), properties(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS) {}

                ConfigurationMetadata(std::string thePid, AnyMap props)
                    : pid(std::move(thePid))
                    , properties(std::move(props))
                {
                }

                std::string pid;
                AnyMap properties;
            };
        } // namespace metadata
    }     // namespace cmimpl
} // namespace cppmicroservices

#endif // CONFIGURATIONMETADATA_HPP
