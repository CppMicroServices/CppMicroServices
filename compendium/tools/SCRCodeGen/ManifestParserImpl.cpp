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
#include "ManifestParserImpl.hpp"
#include "Util.hpp"

#include <unordered_map>

using codegen::datamodel::ComponentInfo;
using codegen::datamodel::ReferenceInfo;
using codegen::util::JsonValueValidator;

std::vector<ComponentInfo>
ManifestParserImplV1::ParseAndGetComponentInfos(Json::Value const& scr) const
{
    std::vector<ComponentInfo> componentInfos;
    auto const jsonComponents = JsonValueValidator(scr, "components", Json::ValueType::arrayValue)();
    for (auto const& jsonComponent : jsonComponents)
    {
        ComponentInfo componentInfo;
        componentInfo.implClassName
            = JsonValueValidator(jsonComponent, "implementation-class", Json::ValueType::stringValue).GetString();

        // The component name is optional and may not exist in the component info metadata.
        try
        {
            componentInfo.name = JsonValueValidator(jsonComponent, "name", Json::ValueType::stringValue).GetString();
        }
        catch (std::exception const&)
        {
        }

        // inject-references
        componentInfo.injectReferences = true;
        if (jsonComponent.isMember("inject-references"))
        {
            auto const injectReferences
                = JsonValueValidator(jsonComponent, "inject-references", Json::ValueType::booleanValue)();
            componentInfo.injectReferences = injectReferences.asBool();
        }

        // configuration-policy and configuration-pid
        bool configPolicy = false;
        bool configPid = false;
        componentInfo.configurationPolicy = codegen::datamodel::ComponentInfo::CONFIG_POLICY_IGNORE;
        if (jsonComponent.isMember("configuration-policy"))
        {
            configPolicy = true;
            JsonValueValidator::ValidChoices<3> policyChoices = {
                {"require", "optional", "ignore"}
            };
            componentInfo.configurationPolicy
                = JsonValueValidator(jsonComponent, "configuration-policy", policyChoices).GetString();
        }
        if (jsonComponent.isMember("configuration-pid"))
        {
            configPid = true;
            // make sure there are no duplicates in the configuration-pid array.
            auto const configurationPids
                = JsonValueValidator(jsonComponent, "configuration-pid", Json::ValueType::arrayValue)();
            std::unordered_map<std::string, std::string> duplicatePids;
            for (auto const& pid : configurationPids)
            {
                if (duplicatePids.find(pid.asString()) != duplicatePids.end())
                {
                    std::string msg = "configuration-pid error in the manifest. "
                                      "Duplicate pid detected.";
                    msg.append(pid.asString());
                    throw std::runtime_error(msg);
                }
                duplicatePids.emplace(pid.asString(), pid.asString());
            }
        }
        // Both configuration-policy and configuration-pid must be present in the manifest.json
        // file to participate in Configuration Admin.
        if ((configPolicy && !configPid) || (!configPolicy && configPid))
        {
            componentInfo.configurationPolicy = codegen::datamodel::ComponentInfo::CONFIG_POLICY_IGNORE;
            throw std::runtime_error("Error: Both configuration-policy and configuration-pid must be "
                                     "present in the manifest.json file to participate in Configuration "
                                     "Admin.");
        }
        // factory
        if (jsonComponent.isMember("factory"))
        {
            auto factoryComponentID
                = JsonValueValidator(jsonComponent, "factory", Json::ValueType::stringValue).GetString();
        }

        // service
        if (jsonComponent.isMember("service"))
        {
            auto const jsonServiceInfo = JsonValueValidator(jsonComponent, "service", Json::ValueType::objectValue)();
            JsonValueValidator::ValidChoices<3> scopeChoices = {
                {"singleton", "bundle", "prototype"}
            };
            componentInfo.service.scope = JsonValueValidator(jsonServiceInfo, "scope", scopeChoices).GetString();

            // interfaces
            auto const jsonServiceInterfaces
                = JsonValueValidator(jsonServiceInfo, "interfaces", Json::ValueType::arrayValue)();
            for (auto const& jsonServiceInterface : jsonServiceInterfaces)
            {
                if (!jsonServiceInterface.isString() || jsonServiceInterface.asString() == "")
                {
                    std::string msg = "Invalid array value for the name 'interfaces'. ";
                    msg += "Expected non-empty string";
                    throw std::runtime_error(msg);
                }
                componentInfo.service.interfaces.push_back(jsonServiceInterface.asString());
            }
        }

        // references
        if (jsonComponent.isMember("references"))
        {
            auto const jsonRefInfos = JsonValueValidator(jsonComponent, "references", Json::ValueType::arrayValue)();

            std::unordered_map<std::string, std::size_t> duplicateRefs;
            duplicateRefs.reserve(jsonRefInfos.size());
            for (auto const& jsonRefInfo : jsonRefInfos)
            {
                ReferenceInfo refInfo;
                refInfo.name = JsonValueValidator(jsonRefInfo, "name", Json::ValueType::stringValue).GetString();
                // reference names for a service component must be unique.
                // track all duplicates and produce an error message afterwards.
                duplicateRefs[refInfo.name]++;

                refInfo.interface = JsonValueValidator(jsonRefInfo, "interface", Json::ValueType::stringValue)
                                        .GetString();
                JsonValueValidator::ValidChoices<4> cardinalityChoices = {
                    {"1..1", "0..1", "1..n", "0..n"}
                };
                refInfo.cardinality = JsonValueValidator(jsonRefInfo, "cardinality", cardinalityChoices).GetString();
                JsonValueValidator::ValidChoices<2> policyChoices = {
                    {"static", "dynamic"}
                };
                refInfo.policy = JsonValueValidator(jsonRefInfo, "policy", policyChoices).GetString();
                JsonValueValidator::ValidChoices<2> optionChoices = {
                    {"reluctant", "greedy"}
                };
                refInfo.policy_option = JsonValueValidator(jsonRefInfo, "policy-option", optionChoices).GetString();
                if (jsonRefInfo.isMember("target"))
                {
                    refInfo.target
                        = JsonValueValidator(jsonRefInfo, "target", Json::ValueType::stringValue).GetString();
                }
                componentInfo.references.push_back(refInfo);
            }

            std::string listOfDuplicateRefNames;
            for (auto const& dupRef : duplicateRefs)
            {
                if (dupRef.second > 1)
                {
                    listOfDuplicateRefNames += dupRef.first + " ";
                }
            }
            if (!listOfDuplicateRefNames.empty())
            {
                std::string exceptionMessage("Duplicate service reference names found. "
                                             "Reference names must be unique. ");
                exceptionMessage += "Duplicate names: ";
                exceptionMessage += listOfDuplicateRefNames;
                throw std::invalid_argument(exceptionMessage);
            }
        }
        componentInfos.push_back(componentInfo);
    }
    return componentInfos;
}
