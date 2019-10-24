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

#ifndef ReferenceDTO_hpp
#define ReferenceDTO_hpp

#include <string>

#include <cppmicroservices/servicecomponent/ServiceComponentExport.h>

namespace cppmicroservices {
namespace service {
namespace component {
namespace runtime {
namespace dto {
/**
 * A representation of a declared reference to a service.
 */
struct US_ServiceComponent_EXPORT ReferenceDTO {
  /**
   * The name of the reference.
   *
   * <p>
   * This is declared in the {@code name} attribute of the {@code reference}
   * element. This must be the default name if the component description does
   * not declare a name for the reference.
   */
  std::string name;

  /**
   * The service interface of the reference.
   *
   * <p>
   * This is declared in the {@code interface} attribute of the
   * {@code reference} element.
   */
  std::string interfaceName;

  /**
   * The cardinality of the reference.
   *
   * <p>
   * This is declared in the {@code cardinality} attribute of the
   * {@code reference} element. This must be the default cardinality if the
   * component description does not declare a cardinality for the reference.
   */
  std::string cardinality;

  /**
   * The policy of the reference.
   *
   * <p>
   * This is declared in the {@code policy} attribute of the {@code reference}
   * element. This must be the default policy if the component description
   * does not declare a policy for the reference.
   */
  std::string policy;

  /**
   * The policy option of the reference.
   *
   * <p>
   * This is declared in the {@code policy-option} attribute of the
   * {@code reference} element. This must be the default policy option if the
   * component description does not declare a policy option for the reference.
   */
  std::string policyOption;

  /**
   * The target of the reference.
   *
   * <p>
   * This is declared in the {@code target} attribute of the {@code reference}
   * element. This must be an empty string if the component description does not
   * declare a target for the reference.
   */
  std::string target;

  /**
   * The name of the bind method of the reference.
   *
   * <p>
   * This is declared in the {@code bind} attribute of the {@code reference}
   * element. This must be an empty string if the component description does not
   * declare a bind method for the reference.
   */
  std::string bind;

  /**
   * The name of the unbind method of the reference.
   *
   * <p>
   * This is declared in the {@code unbind} attribute of the {@code reference}
   * element. This must be an empty string if the component description does not
   * declare an unbind method for the reference.
   */
  std::string unbind;

  /**
   * The name of the updated method of the reference.
   *
   * <p>
   * This is declared in the {@code updated} attribute of the
   * {@code reference} element. This must be an empty string if the component
   * description does not declare an updated method for the reference.
   */
  std::string updated;

  /**
   * The scope of the reference.
   *
   * <p>
   * This is declared in the {@code scope} attribute of the {@code reference}
   * element. This must be the default scope if the component description does
   * not declare a scope for the reference.
   */
  std::string scope;
};
}
}
}
}
}

#endif /* ReferenceDTO_hpp */
