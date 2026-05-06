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
ManifestParserImplV1::ParseAndGetComponentInfos(rapidjson::Value const& scr) const
{
    std::vector<ComponentInfo> componentInfos;
    auto const componentsValidator = JsonValueValidator(scr, "components", rapidjson::kArrayType);
    auto const& jsonComponents = componentsValidator();
    for (auto const& jsonComponent : jsonComponents.GetArray())
    {
        ComponentInfo componentInfo;
        componentInfo.implClassName
            = JsonValueValidator(jsonComponent, "implementation-class", rapidjson::kStringType).GetString();

        // The component name is optional and may not exist in the component info metadata.
        try
        {
            componentInfo.name = JsonValueValidator(jsonComponent, "name", rapidjson::kStringType).GetString();
        }
        catch (std::exception const&)
        {
        }

        // inject-references
        if (jsonComponent.HasMember("inject-references"))
        {
            bool isBool = false;
            try {
                // kTrueType is used to mean "any boolean" — Validate() checks IsBool()
                // which accepts both true and false.
                auto const injectRefBoolValidator = JsonValueValidator(jsonComponent, "inject-references", rapidjson::kTrueType);
                auto const& injectReferences = injectRefBoolValidator();
                componentInfo.setInjectReferences(injectReferences.GetBool());
                isBool = true;
            } catch(...){
                // do nothing
            }

            // if the object was NOT a bool, try an array
            if (!isBool){
                try {
                    // conversion of value to bool failed, must be array of strings
                    auto const injectRefArrayValidator = JsonValueValidator(jsonComponent, "inject-references", rapidjson::kArrayType);
                    auto const& injectReferences = injectRefArrayValidator();
                    std::unordered_set<std::string> injectedRefNames;
                    for (auto const& refName : injectReferences.GetArray()){
                        injectedRefNames.insert(refName.GetString());
                    }
                    componentInfo.setInjectReferences(false, injectedRefNames);
                } catch (...){
                    throw std::runtime_error("Invalid JSON value for the name 'inject-references'. Expected boolean or non-empty array of strings");
                }
            }
        }

        // configuration-policy and configuration-pid
        bool configPolicy = false;
        bool configPid = false;
        componentInfo.configurationPolicy = codegen::datamodel::ComponentInfo::CONFIG_POLICY_IGNORE;
        if (jsonComponent.HasMember("configuration-policy"))
        {
            configPolicy = true;
            JsonValueValidator::ValidChoices<3> policyChoices = {
                {"require", "optional", "ignore"}
            };
            componentInfo.configurationPolicy
                = JsonValueValidator(jsonComponent, "configuration-policy", policyChoices).GetString();
        }
        std::unordered_map<std::string, std::string> duplicatePids;
        if (jsonComponent.HasMember("configuration-pid"))
        {
            configPid = true;
            // make sure there are no duplicates in the configuration-pid array.
            auto const configPidsValidator = JsonValueValidator(jsonComponent, "configuration-pid", rapidjson::kArrayType);
            auto const& configurationPids = configPidsValidator();

            for (auto const& pid : configurationPids.GetArray())
            {
                if (duplicatePids.find(pid.GetString()) != duplicatePids.end())
                {
                    std::string msg = "configuration-pid error in the manifest. "
                                      "Duplicate pid detected.";
                    msg.append(pid.GetString());
                    throw std::runtime_error(msg);
                }
                duplicatePids.emplace(pid.GetString(), pid.GetString());
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
        if (jsonComponent.HasMember("factory"))
        {
            auto factoryComponentID
                = JsonValueValidator(jsonComponent, "factory", rapidjson::kStringType).GetString();
            if (!factoryComponentID.empty() && (duplicatePids.size() != 1))
            {
                throw std::runtime_error("Error: For factory components, the configuration-pid array "
                        "may only contain one entry");
            }
        }

        // service
        if (jsonComponent.HasMember("service"))
        {
            auto const serviceValidator = JsonValueValidator(jsonComponent, "service", rapidjson::kObjectType);
            auto const& jsonServiceInfo = serviceValidator();
            JsonValueValidator::ValidChoices<3> scopeChoices = {
                {"singleton", "bundle", "prototype"}
            };
            componentInfo.service.scope = JsonValueValidator(jsonServiceInfo, "scope", scopeChoices).GetString();

            // interfaces
            auto const interfacesValidator = JsonValueValidator(jsonServiceInfo, "interfaces", rapidjson::kArrayType);
            auto const& jsonServiceInterfaces = interfacesValidator();
            for (auto const& jsonServiceInterface : jsonServiceInterfaces.GetArray())
            {
                if (!jsonServiceInterface.IsString() || jsonServiceInterface.GetStringLength() == 0)
                {
                    std::string msg = "Invalid array value for the name 'interfaces'. ";
                    msg += "Expected non-empty string";
                    throw std::runtime_error(msg);
                }
                componentInfo.service.interfaces.emplace_back(jsonServiceInterface.GetString());
            }
        }

        // references
        if (jsonComponent.HasMember("references"))
        {
            auto const refsValidator = JsonValueValidator(jsonComponent, "references", rapidjson::kArrayType);
            auto const& jsonRefInfos = refsValidator();

            std::unordered_map<std::string, std::size_t> duplicateRefs;
            duplicateRefs.reserve(jsonRefInfos.Size());
            for (auto const& jsonRefInfo : jsonRefInfos.GetArray())
            {
                ReferenceInfo refInfo;
                refInfo.name = JsonValueValidator(jsonRefInfo, "name", rapidjson::kStringType).GetString();
                // reference names for a service component must be unique.
                // track all duplicates and produce an error message afterwards.
                duplicateRefs[refInfo.name]++;

                refInfo.interface = JsonValueValidator(jsonRefInfo, "interface", rapidjson::kStringType)
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
                if (jsonRefInfo.HasMember("target"))
                {
                    refInfo.target
                        = JsonValueValidator(jsonRefInfo, "target", rapidjson::kStringType).GetString();
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

            // validate that all specified injected references correspond to named references
            componentInfo.validateInjectedRefNames();
        }
        componentInfos.push_back(componentInfo);
    }
    return componentInfos;
}
