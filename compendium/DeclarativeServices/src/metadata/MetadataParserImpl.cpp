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

#include "cppmicroservices/Bundle.h"
#include "ComponentMetadata.hpp"
#include "MetadataParserImpl.hpp"
#include "Util.hpp"

#include <iterator>

using cppmicroservices::scrimpl::util::ObjectValidator;

namespace cppmicroservices {
namespace scrimpl {
namespace metadata {

ServiceMetadata MetadataParserImplV1::CreateServiceMetadata(const AnyMap& metadata) const
{
  ServiceMetadata serviceMetadata{};

  // service.interfaces (Mandatory)
  const auto interfaces = ObjectValidator(metadata, "interfaces").GetValue<std::vector<cppmicroservices::Any>>();
  std::transform(std::begin(interfaces),
                 std::end(interfaces),
                 std::back_inserter(serviceMetadata.interfaces),
                 [](const auto& interface) { return ObjectValidator(interface).GetValue<std::string>(); });

  // service.scope
  const auto object = ObjectValidator(metadata, "scope", /*isOptional=*/true);
  object.AssignValueTo(serviceMetadata.scope, /*choices=*/ ServiceMetadata::Scopes);

  return serviceMetadata;
}

ReferenceMetadata MetadataParserImplV1::CreateReferenceMetadata(const AnyMap& metadata) const
{
  ReferenceMetadata refMetadata{};

  // reference.interface (Mandatory)
  ObjectValidator(metadata, "interface").AssignValueTo(refMetadata.interfaceName);

  // reference.name (Mandatory)
  ObjectValidator(metadata, "name").AssignValueTo(refMetadata.name);

  // reference.cardinality
  auto object = ObjectValidator(metadata, "cardinality", /*isOptional=*/true);
  object.AssignValueTo(refMetadata.cardinality, /*choices=*/ ReferenceMetadata::Cardinalities);
  std::tie(refMetadata.minCardinality, refMetadata.maxCardinality) =
    GetReferenceCardinalityExtents(refMetadata.cardinality);

  // reference.policy
  object = ObjectValidator(metadata, "policy", /*isOptional=*/true);
  object.AssignValueTo(refMetadata.policy, /*choices=*/ ReferenceMetadata::Policies);

  // reference.policy-option
  object = ObjectValidator(metadata, "policy-option", /*isOptional=*/true);
  object.AssignValueTo(refMetadata.policyOption, /*choices=*/ ReferenceMetadata::PolicyOptions);

  // reference.target
  ObjectValidator(metadata, "target", /*isOptional=*/true).AssignValueTo(refMetadata.target);

  return refMetadata;
}

std::vector<ReferenceMetadata>
MetadataParserImplV1::CreateReferenceMetadatas(const std::vector<cppmicroservices::Any>& refs) const
{
  std::vector<ReferenceMetadata> refMetadatas;
  for (const auto& ref : refs)
  {
    auto metadata = ObjectValidator(ref).GetValue<AnyMap>();
    refMetadatas.emplace_back(CreateReferenceMetadata(metadata));
  }
  return refMetadatas;
}

std::shared_ptr<ComponentMetadata>
MetadataParserImplV1::CreateComponentMetadata(const AnyMap& metadata) const
{
  auto compMetadata = std::make_shared<ComponentMetadata>();

  // component.implementation-class (Mandatory)
  ObjectValidator(metadata, "implementation-class").AssignValueTo(compMetadata->implClassName);

  // component.immediate
  compMetadata->immediate = true;
  const bool serviceSpecified = ObjectValidator(metadata, "service", /*isOptional=*/true).KeyExists();
  auto object = ObjectValidator(metadata, "immediate", /*isOptional=*/true);
  const bool isImmediate = (object.KeyExists()) ? object.GetValue<bool>() : !serviceSpecified;
  if(!serviceSpecified && !isImmediate)
  {
    throw std::runtime_error("Invalid value specified for the name 'immediate'.");
  }
  compMetadata->immediate = (!serviceSpecified || (serviceSpecified && isImmediate));

  // component.enabled
  ObjectValidator(metadata, "enabled", /*isOptional=*/true).AssignValueTo(compMetadata->enabled);

  // component.name
  compMetadata->name = compMetadata->implClassName;
  ObjectValidator(metadata, "name", /*isOptional=*/true).AssignValueTo(compMetadata->name);

  // component.properties
  object = ObjectValidator(metadata, "properties", /*isOptional=*/true);
  if (object.KeyExists())
  {
    const auto props = object.GetValue<AnyMap>();
    for(const auto& prop : props)
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
    compMetadata->refsMetadata = CreateReferenceMetadatas(object.GetValue<std::vector<cppmicroservices::Any>>());
  }

  return compMetadata;
}

std::vector<std::shared_ptr<ComponentMetadata>>
MetadataParserImplV1::ParseAndGetComponentsMetadata(const AnyMap& scrmap) const
{
  std::vector<std::shared_ptr<ComponentMetadata>> componentsMetadata;
  const auto components = ObjectValidator(scrmap, "components").GetValue<std::vector<cppmicroservices::Any>>();
  std::size_t index = 0;
  for (const auto& component : components)
  {
    const auto componentMap = ObjectValidator(component).GetValue<AnyMap>();
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

}
}
}
