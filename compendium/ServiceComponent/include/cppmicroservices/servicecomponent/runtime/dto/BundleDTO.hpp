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

#ifndef BundleDTO_hpp
#define BundleDTO_hpp

#include <string>

#include <cppmicroservices/servicecomponent/ServiceComponentExport.h>

namespace cppmicroservices {
namespace framework {
namespace dto {

struct US_ServiceComponent_EXPORT BundleDTO
{
  /**
   * The bundle's unique identifier.
   *
   * @see Bundle#GetBundleId()
   */
  unsigned long id;

  /**
   * The time when the bundle was last modified.
   *
   * @see Bundle#GetLastModified()
   */
  unsigned long lastModified;

  /**
   * The bundle's state.
   *
   * @see Bundle#GetState()
   */
  uint32_t state;

  /**
   * The bundle's symbolic name.
   *
   * @see Bundle#GetSymbolicName()
   */
  std::string symbolicName;

  /**
   * The bundle's version.
   *
   * @see Bundle#GetVersion()
   */
  std::string version;
};
}
}
}

#endif /* BundleDTO_hpp */
