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

#ifndef ComponentDescriptionDTO_hpp
#define ComponentDescriptionDTO_hpp

#include <vector>
#include <string>
#include <unordered_map>

#include "BundleDTO.hpp"
#include "ReferenceDTO.hpp"
#include "cppmicroservices/Any.h"
#include "cppmicroservices/servicecomponent/ServiceComponentExport.h"

namespace cppmicroservices {
namespace service {
namespace component {
namespace runtime {
namespace dto {
/**
 * A representation of a declared component description.
 */
struct US_ServiceComponent_EXPORT ComponentDescriptionDTO
{
  /**
   * The name of the component.
   *
   * <p>
   * This is declared in the {@code name} attribute of the {@code component}
   * element. This must be the default name if the component description does
   * not declare a name.
   */
  std::string name;

  /**
   * The bundle declaring the component description.
   */
  cppmicroservices::framework::dto::BundleDTO bundle;

  /**
   * The service scope.
   *
   * <p>
   * This is declared in the {@code scope} attribute of the {@code service}
   * element. This must be empty string if the component description does not
   * declare any service interfaces.
   */
  std::string scope;

  /**
   * The fully qualified name of the implementation class.
   *
   * <p>
   * This is declared in the {@code implementation-class} element.
   */
  std::string implementationClass;

  /**
   * The initial enabled state.
   *
   * <p>
   * This is declared in the {@code enabled} attribute of the
   * {@code component} element.
   */
  bool defaultEnabled;

  /**
   * The immediate state.
   *
   * <p>
   * This is declared in the {@code immediate} attribute of the
   * {@code component} element.
   */
  bool immediate;

  /**
   * The fully qualified names of the service interfaces.
   *
   * <p>
   * These are declared in the {@code interface} attribute of the
   * {@code provide} elements. The vector must be empty if the component
   * description does not declare any service interfaces.
   */
  std::vector<std::string> serviceInterfaces;

  /**
   * The declared component properties.
   *
   * <p>
   * These are declared in the {@code property} and {@code properties}
   * elements.
   */
  std::unordered_map<std::string, cppmicroservices::Any> properties;

  /**
   * The referenced services.
   *
   * <p>
   * These are declared in the {@code reference} elements. The vector must be
   * empty if the component description does not declare references to any
   * services.
   */
  std::vector<ReferenceDTO> references;

  /**
   * The name of the activate method.
   *
   * <p>
   * This is declared in the {@code activate} attribute of the
   * {@code component} element. This must be the default value which is "Activate",
   * if the component description does not declare an activate method name.
   */
  std::string activate;

  /**
   * The name of the deactivate method.
   *
   * <p>
   * This is declared in the {@code deactivate} attribute of the
   * {@code component} element. This must be the default value which is "Deactivate",
   * if the component description does not declare a deactivate method name.
   */
  std::string deactivate;

  /**
   * The name of the modified method.
   *
   * <p>
   * This is declared in the {@code modified} attribute of the
   * {@code component} element. This must be the default value which is "Modified",
   * if the component description does not declare a modified method name.
   */
  std::string modified;
};
}
}
}
}
}

#endif /* ComponentDescriptionDTO_hpp */
