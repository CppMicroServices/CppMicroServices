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

#ifndef ComponentConstants_hpp
#define ComponentConstants_hpp

#include <string>

#include <cppmicroservices/servicecomponent/ServiceComponentExport.h>

namespace cppmicroservices { namespace service { namespace component {

/**
 * Defines standard names for Service Component constants.
 */
namespace ComponentConstants {
/**
 * Manifest key specifying the Service Component descriptions.
 * <p>
 * The attribute value may be retrieved from the {@code cppmicroservices::AnyMap}
 * object returned by the {@code Bundle.GetHeaders} method.
 */
US_ServiceComponent_EXPORT extern const std::string SERVICE_COMPONENT;

/**
 * A component property for a component configuration that contains the name
 * of the component as specified in the {@code name} attribute of the
 * {@code component} element. The value of this property must be of type
 * {@code std::string}.
 */
US_ServiceComponent_EXPORT extern const std::string COMPONENT_NAME;

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
US_ServiceComponent_EXPORT extern const std::string COMPONENT_ID;

/**
 * The suffix for reference target properties. These properties contain the
 * filter to select the target services for a reference. The value of this
 * property must be of type {@code std::string}.
 */
US_ServiceComponent_EXPORT extern const std::string REFERENCE_TARGET_SUFFIX;

/**
 * Reference scope is "prototype_required". The reference is satisfied
 * only if the service is registered with PROTOTYPE scope. Each component
 * instance receives a distinct service object.
 */
US_ServiceComponent_EXPORT extern const std::string REFERENCE_SCOPE_PROTOTYPE_REQUIRED;
}

}}} // namespaces

#endif // ComponentConstants_hpp
