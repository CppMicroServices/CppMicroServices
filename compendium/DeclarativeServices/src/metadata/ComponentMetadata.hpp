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

#include <string>
#include <vector>
#include <unordered_map>

#include "ServiceMetadata.hpp"
#include "ReferenceMetadata.hpp"

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
  bool enabled{true};
  bool immediate{false};
  std::string implClassName;
  std::string activateMethodName;
  std::string deactivateMethodName;
  std::string modifiedMethodName;
  std::vector<ReferenceMetadata> refsMetadata;
  ServiceMetadata serviceMetadata;
  std::unordered_map<std::string, cppmicroservices::Any> properties;
};
}
}
}

#endif //COMPONENTMETADATA_HPP
