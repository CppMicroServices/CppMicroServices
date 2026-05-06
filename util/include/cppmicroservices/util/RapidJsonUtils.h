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
#ifndef CPPMICROSERVICES_RAPIDJSONUTILS_HPP
#define CPPMICROSERVICES_RAPIDJSONUTILS_HPP

#include <set>
#include <stdexcept>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

namespace cppmicroservices
{
    namespace rapidjsonutils
    {

        // Recursively checks for duplicate keys in JSON objects and arrays.
        // RapidJSON's parser silently keeps duplicate keys, so we detect them
        // manually after parsing.
        inline void
        checkDuplicateKeys(rapidjson::Value const& value)
        {
            if (value.IsObject())
            {
                std::set<std::string> seen;
                for (auto const& m : value.GetObject())
                {
                    if (!seen.insert(m.name.GetString()).second)
                    {
                        throw std::runtime_error(std::string("Duplicate key: '") + m.name.GetString() + "'");
                    }
                    checkDuplicateKeys(m.value);
                }
            }
            else if (value.IsArray())
            {
                for (auto const& elem : value.GetArray())
                {
                    checkDuplicateKeys(elem);
                }
            }
        }

        // Serializes a rapidjson value to a pretty-printed JSON string with a
        // trailing newline.
        inline std::string
        toStyledString(rapidjson::Value const& value)
        {
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            value.Accept(writer);
            std::string out(buffer.GetString(), buffer.GetSize());
            out += '\n';
            return out;
        }

    } // namespace rapidjsonutils
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_RAPIDJSONUTILS_HPP
