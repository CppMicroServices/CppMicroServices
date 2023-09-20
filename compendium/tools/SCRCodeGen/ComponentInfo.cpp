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

#include <iterator>
#include <regex>
#include <sstream>

#include "ComponentInfo.hpp"

namespace codegen
{
    namespace datamodel
    {
        const std::string ComponentInfo::CONFIG_POLICY_IGNORE = "ignore";
        const std::string ComponentInfo::CONFIG_POLICY_REQUIRE = "require";
        const std::string ComponentInfo::CONFIG_POLICY_OPTIONAL = "optional";

        std::string
        GetComponentNameStr(ComponentInfo const& compInfo)
        {
            auto const name = compInfo.name.empty() ? compInfo.implClassName : compInfo.name;
            return std::regex_replace(name, std::regex("(::)"), "_");
        }

        std::string
        GetServiceInterfacesStr(ServiceInfo const& serviceInfo)
        {
            auto& interfaces = serviceInfo.interfaces;
            if (interfaces.empty())
            {
                return "";
            }
            std::ostringstream strstream;
            std::copy(std::begin(interfaces),
                      std::end(interfaces) - 1,
                      std::ostream_iterator<std::string>(strstream, ", "));
            strstream << interfaces.back();
            return strstream.str();
        }

        std::string
        GetCtorInjectedRefTypes(ComponentInfo const& compInfo)
        {
            std::string result;
            auto sep = ", ";
            for (auto const& reference : compInfo.references)
            {
                if ((true == compInfo.injectReferences) && (reference.policy == "static"))
                {
                    result += (sep + reference.interface);
                }
            }
            return result;
        }

        std::string
        GetCtorInjectedRefNames(ComponentInfo const& compInfo)
        {
            std::stringstream resultStr;
            auto sep = "";

            resultStr << "{{";
            for (auto const& reference : compInfo.references)
            {
                if ((true == compInfo.injectReferences) && (reference.policy == "static"))
                {
                    resultStr << sep << "\"" << reference.name << "\"";
                    sep = ", ";
                }
            }
            resultStr << "}}";
            return resultStr.str();
        }

        std::string
        GetReferenceBinderStr(ReferenceInfo const& ref, bool injectReferences)
        {
            auto isStatic = (ref.policy == "static");
            std::stringstream binderObjStr;
            if (!isStatic || !injectReferences)
            {
                binderObjStr << "std::make_shared<scd::DynamicBinder<{0}, "
                             << ref.interface << ">>(\"" + ref.name + "\""
                             << ", &{0}::Bind" << ref.name << ", &{0}::Unbind" << ref.name << ")";
            }
            return binderObjStr.str();
        }

    } // namespace datamodel
} // namespace codegen
