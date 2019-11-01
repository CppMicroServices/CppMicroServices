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

#ifndef REFERENCEMETADATA_HPP
#define REFERENCEMETADATA_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <tuple>

#include "cppmicroservices/Any.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/LDAPFilter.h"
#include "Util.hpp"

namespace cppmicroservices {
namespace scrimpl {
namespace metadata {

/**
 * Stores the reference metadata information parsed from the Service Component
 * Runtime description.
 */
struct ReferenceMetadata
{
  // defaults for the data model
  ReferenceMetadata()
    : cardinality("1..1")
    , policy("static")
    , policyOption("reluctant")
    , scope("bundle")
  {}

  std::string name;
  std::string target;
  std::string interfaceName;
  std::string cardinality;
  std::string policy;
  std::string policyOption;
  std::string scope;
  std::size_t minCardinality{1};
  std::size_t maxCardinality{1};

  static const std::vector<std::string> Cardinalities;
  static const std::vector<std::string> Policies;
  static const std::vector<std::string> PolicyOptions;
  static const std::vector<std::string> Scopes;
};

/**
 * @brief Returns the cardinality information given a string representing the reference
 *        cardinality
 * @param cardinality the reference cardinality string
 * @returns a 2-element tuple, where
 *          the first element is the calculated maximum cardinality
 *          the second element is the calculated minimum cardinality
 * @throws std::out_of_range error if @p cardinality is not found in the
 *         global @c ReferenceMetadata::Cardinalities
 */
std::tuple<std::size_t, std::size_t> GetReferenceCardinalityExtents(const std::string& cardinality);

}
}
}

#endif //REFERENCEMETADATA_HPP
