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

#include "PropsCheck.h"

namespace cppmicroservices
{
    namespace props_check
    {
        namespace
        {
            const Any emptyAny;
        }

        /**
         * @brief Validates that the provided AnyMap conforms to the same constraints that
         * those stored in Property objects have.
         *
         * The provided AnyMap is said to be valid if there exists no pairs of two keys
         * which differ in case only (e.g., "service.feature", "Service.feature"). If this
         * condition is not true, this function throws as defined below.
         *
         * @param am The AnyMap to validate
         * @throws std::runtime_error Thrown when `am` is invalid (described above)
         */
        void
        ValidateAnyMap(cppmicroservices::AnyMap const& am)
        {
            std::vector<std::string_view> keys(am.size());
            uint32_t currIndex = 0;
            for (auto& kv_pair : am)
            {
                keys[currIndex++] = kv_pair.first;
            }

            if (am.size() > 1)
            {
                // NOTE: A solution involving iterations rather than "raw for-loops" was previously
                // tested but ended up being slower than the solution below.
                for (uint32_t i = 0; i < keys.size() - 1; ++i)
                {
                    for (uint32_t j = i + 1; j < keys.size(); ++j)
                    {
                        if (keys[i].size() == keys[j].size()
                            && ci_compare(keys[i].data(), keys[j].data(), keys[i].size()) == 0)
                        {
                            std::string msg("Properties contain case variants of the key: ");
                            msg += keys[i];
                            throw std::runtime_error(msg.c_str());
                        }
                    }
                }
            }
        }

        std::string
        ToLower(std::string const& s)
        {
            std::string sNew = s;
            std::transform(sNew.begin(), sNew.end(), sNew.begin(), [](char c) { return ::tolower(c); });
            return sNew;
        }
    } // namespace props_check
} // namespace cppmicroservices
