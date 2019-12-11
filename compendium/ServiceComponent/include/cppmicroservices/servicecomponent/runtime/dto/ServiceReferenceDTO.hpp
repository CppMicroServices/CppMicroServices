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

#ifndef ServiceReferenceDTO_hpp
#define ServiceReferenceDTO_hpp

#include <string>
#include <vector>
#include <unordered_map>

#include "cppmicroservices/Any.h"
#include "cppmicroservices/servicecomponent/ServiceComponentExport.h"

namespace cppmicroservices {
namespace framework {
namespace dto {

/**
 * A representation of a satisfied reference.
 */
struct US_ServiceComponent_EXPORT ServiceReferenceDTO
{
  /**
   * The id of the service.
   *
   * @see Constants#SERVICE_ID
   */
  unsigned long id;

  /**
   * The id of the bundle that registered the service.
   *
   * @see ServiceReference#GetBundle()
   */
  unsigned long bundle;

  /**
   * The properties for the service.
   *
   * The value type must be a numerical type, Boolean, String or a container
   * of any of the former.
   *
   * @see ServiceReference#GetProperty(String)
   */
  std::unordered_map<std::string, cppmicroservices::Any> properties;

  /**
   * The ids of the bundles that are using the service.
   *
   * @see ServiceReference#GetUsingBundles()
   */
  std::vector<unsigned long> usingBundles;
};
}
}
}

#endif /* ServiceReferenceDTO_hpp */
