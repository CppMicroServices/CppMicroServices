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

#include <string>

#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"

namespace cppmicroservices {
namespace service {
namespace component {

/**
 * Defines standard names for Service Component constants.
 */
namespace ComponentConstants {
/**
 * Name of the object that contains Service Component descriptions in
 * the bundle's metadata.
 * <p>
 * The value may be retrieved from the {@code cppmicroservices::AnyMap} object
 * returned by the {@code Bundle.GetHeaders} method.
 */
const std::string SERVICE_COMPONENT = "scr";

/**
 * A component property for a component configuration that contains the name
 * of the component as specified in the {@code name} attribute of the
 * {@code component} element. The value of this property must be of type
 * {@code std::string}.
 */
const std::string COMPONENT_NAME = "component.name";

/**
 * A component property that contains the generated id for a component
 * configuration. The value of this property must be of type {@code unsigned long}.
 *
 * <p>
 * The value of this property is assigned by Service Component Runtime when
 * a component configuration is created. Service Component Runtime assigns a
 * unique value that is larger than all previously assigned values since
 * Service Component Runtime was started. These values are NOT persistent
 * across restarts of Service Component Runtime.
 */
const std::string COMPONENT_ID = "component.id";

/**
 * A service registration property for a Component Factory that contains the
 * value of the {@code factory} attribute. The value of this property must
 * be of type {@code std::string}.
 */
const std::string COMPONENT_FACTORY = "component.factory";

/**
 * The suffix for reference target properties. These properties contain the
 * filter to select the target services for a reference. The value of this
 * property must be of type {@code std::string}.
 */
const std::string REFERENCE_TARGET_SUFFIX = ".target";

/**
 * Scope to indicate the reference must be a servcie registered with PROTOTYPE scope.
 */
const std::string REFERENCE_SCOPE_PROTOTYPE_REQUIRED = "prototype_required";
}
}
}
}
