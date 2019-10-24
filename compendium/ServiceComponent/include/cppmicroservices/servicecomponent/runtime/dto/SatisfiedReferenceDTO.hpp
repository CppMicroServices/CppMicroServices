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

#ifndef SatisfiedReferenceDTO_hpp
#define SatisfiedReferenceDTO_hpp

#include <string>
#include <vector>

#include "ServiceReferenceDTO.hpp"

namespace cppmicroservices {
namespace service {
namespace component {
namespace runtime {
namespace dto {

/**
 * A representation of a satisfied reference.
 */
struct SatisfiedReferenceDTO
{
  /**
   * The name of the declared reference.
   *
   * <p>
   * This is declared in the {@code name} attribute of the {@code reference}
   * element of the component description.
   *
   * @see ReferenceDTO#name
   */
  std::string name;

  /**
   * The target property of the satisfied reference.
   *
   * <p>
   * This is the value of the {@link ComponentConfigurationDTO#properties
   * component property} whose name is the concatenation of the
   * {@link ReferenceDTO#name declared reference name} and
   * &quot;.target&quot;. This must be empty string if no target property is
   * set for the reference.
   */
  std::string target;

  /**
   * The bound services.
   *
   * <p>
   * Each {@link ServiceReferenceDTO} in the vector represents a service bound
   * to the satisfied reference. The vector must be empty if there are no bound
   * services.
   */
  std::vector<cppmicroservices::framework::dto::ServiceReferenceDTO> boundServices;
};
}
}
}
}
}

#endif /* SatisfiedReferenceDTO_hpp */
