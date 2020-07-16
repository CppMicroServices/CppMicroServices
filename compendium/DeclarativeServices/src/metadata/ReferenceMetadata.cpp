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

#include "ReferenceMetadata.hpp"
#include "Util.hpp"

namespace cppmicroservices {
namespace scrimpl {
namespace metadata {

const std::vector<std::string> ReferenceMetadata::Cardinalities = {"0..1", "1..1", "0..n", "1..n"};
const std::vector<std::string> ReferenceMetadata::Policies = {"static", "dynamic"};
const std::vector<std::string> ReferenceMetadata::PolicyOptions = {"greedy", "reluctant"};
const std::vector<std::string> ReferenceMetadata::Scopes = {"bundle", "prototype", "prototype-required"};

std::tuple<std::size_t, std::size_t> GetReferenceCardinalityExtents(const std::string& cardinality)
{
  std::size_t minCardinality = 1;
  std::size_t maxCardinality = 1;
  auto cardinalities = ReferenceMetadata::Cardinalities;
  auto it = std::find(std::begin(cardinalities), std::end(cardinalities), cardinality);
  if (it == std::end(cardinalities))
  {
    std::string msg = cardinality + " is not a valid ReferenceCardinality string";
    throw std::out_of_range(msg);
  }
  auto index = std::distance(std::begin(cardinalities), it);
  switch (index)
  {
    case 0:
    {
      minCardinality = 0;
      maxCardinality = 1;
      break;
    }
    case 1:
    {
      minCardinality = 1;
      maxCardinality = 1;
      break;
    }
    case 2:
    {
      minCardinality = 0;
      maxCardinality = std::numeric_limits<std::size_t>::max();
      break;
    }
    case 3:
    {
      minCardinality = 1;
      maxCardinality = std::numeric_limits<std::size_t>::max();
      break;
    }
  }
  return std::make_tuple(minCardinality, maxCardinality);
}
}
}
}
