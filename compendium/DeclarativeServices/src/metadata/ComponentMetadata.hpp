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

#ifndef COMPONENTMETADATA_HPP
#define COMPONENTMETADATA_HPP

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "ReferenceMetadata.hpp"
#include "ServiceMetadata.hpp"

namespace cppmicroservices {
namespace scrimpl {
namespace metadata {

/**
 * Stores component metadata information parsed from the Service Component
 * Runtime description.
 */
struct ComponentMetadata
{
  ComponentMetadata()
    : activateMethodName("Activate")
    , deactivateMethodName("Deactivate")
    , modifiedMethodName("Modified")
  {}

  std::string name;
  bool enabled{ true };
  bool immediate{ false };
  std::string implClassName;
  std::string activateMethodName;
  std::string deactivateMethodName;
  std::string modifiedMethodName;
  std::vector<ReferenceMetadata> refsMetadata;
  ServiceMetadata serviceMetadata;
  std::unordered_map<std::string, cppmicroservices::Any> properties;
  std::string configurationPolicy;
  std::vector<std::string> configurationPids;
  std::string factoryComponentID;
  std::unordered_map<std::string, cppmicroservices::Any> factoryComponentProperties;

  friend std::ostream& operator<<(std::ostream& os, const ComponentMetadata& metadata);
};

inline std::ostream& operator<<(std::ostream& os, const ComponentMetadata& metadata)
{
  os << "ComponentMetadata[name = " << metadata.name << "]" << std::endl
     << "\tenabled: " << metadata.enabled << std::endl
     << "\timmediate: " << metadata.immediate << std::endl
     << "\timplClassName: " << metadata.implClassName << std::endl
     << "\tactivateMethodName: " << metadata.activateMethodName << std::endl
     << "\tdeactivateMethodName: " << metadata.deactivateMethodName << std::endl
     << "\tmodifiedMethodName: " << metadata.modifiedMethodName << std::endl;

  os << "\trefsMetaData[size = " << metadata.refsMetadata.size() << "]: [" << std::endl;
  for (const auto& rMeta : metadata.refsMetadata) {
    os << "\t\tReferenceMetadata[name = " << rMeta.name << "]" << std::endl
       << "\t\t\ttarget: " << rMeta.target << std::endl
       << "\t\t\tinterfaceName: " << rMeta.interfaceName << std::endl
       << "\t\t\tcardinality: " << rMeta.cardinality << std::endl
       << "\t\t\tpolicy: " << rMeta.policy << std::endl
       << "\t\t\tpolicyOption: " << rMeta.policyOption << std::endl
       << "\t\t\tscope: " << rMeta.scope << std::endl
       << "\t\t\tminCardinality: " << rMeta.minCardinality << std::endl
       << "\t\t\tmaxCardinality: " << rMeta.maxCardinality << std::endl;
  }
  os << "\t]" << std::endl;

  os << "\tserviceMetadata:" << std::endl
     << "\t\tServiceMetadata: [" << std::endl
     << "\t\t\tinterfaces[size = " << metadata.serviceMetadata.interfaces.size() << "]: [" << std::endl;
  for (const auto& interface : metadata.serviceMetadata.interfaces) {
    bool isLast = interface != metadata.serviceMetadata.interfaces.back();
    os << "\t\t\t\t" << interface << (isLast ? "" : ",") << std::endl;
  }
  os << "\t\t\t]," << std::endl
     << "\t\t\tscope: " << metadata.serviceMetadata.scope << std::endl
     << "\t\t]" << std::endl;

  os << "\tproperties: [size = " << metadata.properties.size() << "] not shown" << std::endl;
  os << "\tconfigurationPolicy: " << metadata.configurationPolicy << std::endl;
  os << "\tconfigurationPids[size = " << metadata.configurationPids.size() << "]: [" << std::endl;
  for (const auto& pid : metadata.configurationPids) {
    bool isLast = pid != metadata.configurationPids.back();
    os << "\t\t" << pid << (isLast ? "" : ",") << std::endl;
  }
  os << "\t" << "]" << std::endl
     << "\t" << "factoryComponentID: " << metadata.factoryComponentID << std::endl;
  os << "\tfactoryComponentProperties: [size = " << metadata.factoryComponentProperties.size() << "] not shown" << std::endl;

  return os;
}
}
}
}

#endif //COMPONENTMETADATA_HPP
