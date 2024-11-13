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

#include "MetadataParserImpl.hpp"
#include "ComponentMetadata.hpp"
#include "Util.hpp"
#include "cppmicroservices/Bundle.h"

#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include <iterator>

using cppmicroservices::scrimpl::util::ObjectValidator;
using cppmicroservices::service::component::ComponentConstants::CONFIG_POLICY_IGNORE;

namespace cppmicroservices
{
    namespace scrimpl
    {
        namespace metadata
        {

            std::string
            removeLeadingNamespacing(std::string const& className)
            {
                return className.substr(className.find_first_not_of(':'));
            }

            ServiceMetadata
            MetadataParserImplV1::CreateServiceMetadata(AnyMap const& metadata) const
            {
                ServiceMetadata serviceMetadata {};

                // service.interfaces (Mandatory)
                auto const interfaces
                    = ObjectValidator(metadata, "interfaces").GetValue<std::vector<cppmicroservices::Any>>();
                std::transform(std::begin(interfaces),
                               std::end(interfaces),
                               std::back_inserter(serviceMetadata.interfaces),
                               [](auto const& interface)
                               { return removeLeadingNamespacing(ObjectValidator(interface).GetValue<std::string>()); });

                // service.scope
                auto const object = ObjectValidator(metadata, "scope", /*isOptional=*/true);
                object.AssignValueTo(serviceMetadata.scope,
                                     /*choices=*/ServiceMetadata::Scopes);

                return serviceMetadata;
            }

            ReferenceMetadata
            MetadataParserImplV1::CreateReferenceMetadata(AnyMap const& metadata) const
            {
                ReferenceMetadata refMetadata {};

                // reference.interface (Mandatory)
                ObjectValidator(metadata, "interface").AssignValueTo(refMetadata.interfaceName);
                refMetadata.interfaceName = removeLeadingNamespacing(refMetadata.interfaceName);
                // reference.name (Mandatory)
                ObjectValidator(metadata, "name").AssignValueTo(refMetadata.name);

                // reference.cardinality
                auto object = ObjectValidator(metadata, "cardinality", /*isOptional=*/true);
                object.AssignValueTo(refMetadata.cardinality,
                                     /*choices=*/ReferenceMetadata::Cardinalities);
                std::tie(refMetadata.minCardinality, refMetadata.maxCardinality)
                    = GetReferenceCardinalityExtents(refMetadata.cardinality);

                // reference.policy
                object = ObjectValidator(metadata, "policy", /*isOptional=*/true);
                object.AssignValueTo(refMetadata.policy,
                                     /*choices=*/ReferenceMetadata::Policies);

                // reference.policy-option
                object = ObjectValidator(metadata, "policy-option", /*isOptional=*/true);
                object.AssignValueTo(refMetadata.policyOption,
                                     /*choices=*/ReferenceMetadata::PolicyOptions);

                // reference.target
                ObjectValidator(metadata, "target", /*isOptional=*/true).AssignValueTo(refMetadata.target);

                return refMetadata;
            }

            std::vector<ReferenceMetadata>
            MetadataParserImplV1::CreateReferenceMetadatas(std::vector<cppmicroservices::Any> const& refs) const
            {
                std::vector<ReferenceMetadata> refMetadatas;
                for (auto const& ref : refs)
                {
                    auto metadata = ObjectValidator(ref).GetValue<AnyMap>();
                    refMetadatas.emplace_back(CreateReferenceMetadata(metadata));
                }
                return refMetadatas;
            }

            std::shared_ptr<ComponentMetadata>
            MetadataParserImplV1::CreateComponentMetadata(AnyMap const& metadata) const
            {
                auto compMetadata = std::make_shared<ComponentMetadata>();

                // component.implementation-class (Mandatory)
                ObjectValidator(metadata, "implementation-class").AssignValueTo(compMetadata->implClassName);

                // component.immediate
                compMetadata->immediate = true;
                bool const serviceSpecified = ObjectValidator(metadata, "service", /*isOptional=*/true).KeyExists();
                auto object = ObjectValidator(metadata, "immediate", /*isOptional=*/true);
                bool const isImmediate = (object.KeyExists()) ? object.GetValue<bool>() : !serviceSpecified;
                if (!serviceSpecified && !isImmediate)
                {
                    throw std::runtime_error("Invalid value specified for the name 'immediate'.");
                }
                compMetadata->immediate = (!serviceSpecified || (serviceSpecified && isImmediate));

                // component.enabled
                ObjectValidator(metadata, "enabled", /*isOptional=*/true).AssignValueTo(compMetadata->enabled);

                // component.name
                compMetadata->name = compMetadata->implClassName;
                ObjectValidator(metadata, "name", /*isOptional=*/true).AssignValueTo(compMetadata->name);
                compMetadata->instanceName = compMetadata->name;

                // component.configuration-policy (Optional)
                compMetadata->configurationPolicy = CONFIG_POLICY_IGNORE;
                bool configPolicy = false;
                object = ObjectValidator(metadata, "configuration-policy", /*isOptional=*/true);
                if (object.KeyExists())
                {
                    object.AssignValueTo(compMetadata->configurationPolicy);
                    configPolicy = true;
                }

                // component.configuration-pid (Optional)
                bool configPid = false;
                object = ObjectValidator(metadata, "configuration-pid", /*isOptional=*/true);
                if (object.KeyExists())
                {
                    configPid = true;
                    if (configPolicy)
                    {
                        auto const configPids = object.GetValue<std::vector<cppmicroservices::Any>>();
                        std::transform(std::begin(configPids),
                                       std::end(configPids),
                                       std::back_inserter(compMetadata->configurationPids),
                                       [](auto const& configPid)
                                       { return ObjectValidator(configPid).GetValue<std::string>(); });

                        // search for a configuration pid equal to $. If present replace with component name.
                        //  Also search for duplicates pids. These are errors.
                        std::unordered_map<std::string, std::string> duplicatePids;
                        for (auto& pid : compMetadata->configurationPids)
                        {
                            if (pid == "$")
                            {
                                pid = compMetadata->name;
                            }

                            if (duplicatePids.find(pid) != duplicatePids.end())
                            {
                                std::string msg = "configuration-pid error in the manifest. Duplicate pid detected. ";
                                msg.append(pid);
                                throw std::runtime_error(msg);
                            }
                            duplicatePids.emplace(pid, pid);
                        };
                    }
                }
                /* In order to participate in ConfigurationAdmin both the configuration-policy
                 * and the configuration-pid must be present in the manifest.json file.
                 * Otherwise the configuration-policy is set to ignore. If only one is present
                 * a Warning message is logged.
                 */
                if ((configPolicy && !configPid) || (!configPolicy && configPid))
                {
                    compMetadata->configurationPolicy = CONFIG_POLICY_IGNORE;
                    compMetadata->configurationPids.clear();
                    std::string msg = "Warning: configuration-policy has been set to ignore.";
                    msg.append(" Both configuration-policy and configuration-pid must be present");
                    msg.append(" to participate in Configuration Admin. ");
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING, msg);
                }

                if (compMetadata->configurationPolicy == CONFIG_POLICY_IGNORE)
                {
                    compMetadata->configurationPids.clear();
                }
                // component.factory
                ObjectValidator(metadata, "factory", /*isOptional=*/true)
                    .AssignValueTo(compMetadata->factoryComponentID);

                // component.factoryProperties
                object = ObjectValidator(metadata, "factory-properties", /*isOptional=*/true);
                if (object.KeyExists())
                {
                    auto const props = object.GetValue<AnyMap>();
                    for (auto const& prop : props)
                    {
                        compMetadata->factoryComponentProperties.insert(prop);
                    }
                }

                // component.properties
                object = ObjectValidator(metadata, "properties", /*isOptional=*/true);
                if (object.KeyExists())
                {
                    auto const props = object.GetValue<AnyMap>();
                    for (auto const& prop : props)
                    {
                        compMetadata->properties.insert(prop);
                    }
                }

                // component.service
                compMetadata->serviceMetadata = {};
                object = ObjectValidator(metadata, "service", /*isOptional=*/true);
                if (object.KeyExists())
                {
                    compMetadata->serviceMetadata = CreateServiceMetadata(object.GetValue<AnyMap>());
                }

                // component.references
                compMetadata->refsMetadata = {};
                object = ObjectValidator(metadata, "references", /*isOptional=*/true);
                if (object.KeyExists())
                {
                    compMetadata->refsMetadata
                        = CreateReferenceMetadatas(object.GetValue<std::vector<cppmicroservices::Any>>());
                }

                return compMetadata;
            }

            std::vector<std::shared_ptr<ComponentMetadata>>
            MetadataParserImplV1::ParseAndGetComponentsMetadata(AnyMap const& scrmap) const
            {
                std::vector<std::shared_ptr<ComponentMetadata>> componentsMetadata;
                auto const components
                    = ObjectValidator(scrmap, "components").GetValue<std::vector<cppmicroservices::Any>>();
                std::size_t index = 0;
                for (auto const& component : components)
                {
                    auto const componentMap = ObjectValidator(component).GetValue<AnyMap>();
                    // Log the exception if the component can't be parsed and process the next
                    // component
                    try
                    {
                        componentsMetadata.emplace_back(CreateComponentMetadata(componentMap));
                    }
                    catch (std::exception& ex)
                    {
                        std::string msg = std::string(ex.what());
                        msg += " Could not load the component with index: " + std::to_string(index);
                        logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, msg);
                    }
                    ++index;
                }
                return componentsMetadata;
            }

        } // namespace metadata
    } // namespace scrimpl
} // namespace cppmicroservices
