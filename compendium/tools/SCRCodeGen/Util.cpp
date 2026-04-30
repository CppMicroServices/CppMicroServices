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
#include "Util.hpp"

#include <set>

namespace codegen
{
    namespace util
    {

        // Recursively checks for duplicate keys in JSON objects and arrays.
        // Replaces jsoncpp's rbuilder["rejectDupKeys"] = true option, which
        // rejected duplicate keys during parsing. RapidJSON's parser silently
        // keeps duplicate keys, so we detect them manually after parsing.
        void
        checkDuplicateKeys(rapidjson::Value const& value)
        {
            if (value.IsObject())
            {
                std::set<std::string> seen;
                for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it)
                {
                    std::string key = it->name.GetString();
                    if (!seen.insert(key).second)
                    {
                        throw std::runtime_error("Duplicate key '" + key + "' found in manifest");
                    }
                    checkDuplicateKeys(it->value);
                }
            }
            else if (value.IsArray())
            {
                for (auto it = value.Begin(); it != value.End(); ++it)
                {
                    checkDuplicateKeys(*it);
                }
            }
        }

        
        rapidjson::Document
        ParseManifestOrThrow(std::istream& jsonStream)
        {
            rapidjson::IStreamWrapper stream(jsonStream);

            rapidjson::Document root;
            // RapidJSON sets an error flag on the Document instead of returning bool.
            root.ParseStream(stream);

            if (root.HasParseError())
            {
                // GetParseError_En converts rapidjson's error code to English text.
                // GetErrorOffset gives the byte position where parsing failed.
                std::string errs = "JSON parse error at offset "
                                   + std::to_string(root.GetErrorOffset()) + ": "
                                   + rapidjson::GetParseError_En(root.GetParseError());
                throw std::runtime_error(errs);
            }

            checkDuplicateKeys(root);

            return root;
        }

        void
        WriteToFile(std::string const& filePath, std::string const& content)
        {
            auto fileStream = std::ofstream(filePath, std::ofstream::binary | std::ofstream::out);
            if (!fileStream.is_open())
            {
                throw std::runtime_error("Could not open out file at " + filePath);
            }
            fileStream << content;
            fileStream.close();
        }
    } // namespace util
} // namespace codegen
