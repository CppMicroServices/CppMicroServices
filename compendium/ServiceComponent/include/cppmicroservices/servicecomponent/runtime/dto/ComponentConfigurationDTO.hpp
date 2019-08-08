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

#ifndef ComponentConfigurationDTO_hpp
#define ComponentConfigurationDTO_hpp

#include <iostream>

#include "ComponentDescriptionDTO.hpp"
#include "SatisfiedReferenceDTO.hpp"
#include "UnsatisfiedReferenceDTO.hpp"
#include "cppmicroservices/servicecomponent/ServiceComponentExport.h"

namespace cppmicroservices {
namespace service {
namespace component {
namespace runtime {
namespace dto {
enum US_ServiceComponent_EXPORT ComponentState {
  /**
   * The component configuration is unsatisfied due to an unsatisfied
   * reference.
   */
  UNSATISFIED_REFERENCE,
  /**
   * The component configuration is satisfied.
   *
   * <p>
   * Any {@link ComponentDescriptionDTO#serviceInterfaces services} declared
   * by the component description are registered.
   */
  SATISFIED,
  /**
   * The component configuration is active.
   *
   * <p>
   * This is the normal operational state of a component configuration.
   */
  ACTIVE
};

/**
 * A representation of an actual instance of a declared component description
 * parameterized by component properties.
 */
struct US_ServiceComponent_EXPORT ComponentConfigurationDTO {
  /**
   * The representation of the component configuration's component
   * description.
   */
  ComponentDescriptionDTO	description;

  /**
   * The current state of the component configuration.
   *
   * <p>
   * This is one of
   * {@link #UNSATISFIED_REFERENCE}, {@link #SATISFIED} or {@link #ACTIVE}.
   */
  ComponentState state;

  /**
   * The id of the component configuration.
   *
   * <p>
   * The id is a non-persistent, unique value assigned at runtime. The id is
   * also available as the {@code component.id} component property. The value
   * of this field is unspecified if the state of this component configuration
   * is unsatisfied.
   */
  unsigned long id;

  /**
   * The component properties for the component configuration.
   *
   * @see ComponentContext#GetProperties()
   */
  std::unordered_map<std::string, cppmicroservices::Any>	properties;

  /**
   * The satisfied references.
   *
   * <p>
   * Each {@link SatisfiedReferenceDTO} in the vector represents a satisfied
   * reference of the component configuration. The vector must be empty if the
   * component configuration has no satisfied references.
   */
  std::vector<SatisfiedReferenceDTO>		satisfiedReferences;

  /**
   * The unsatisfied references.
   *
   * <p>
   * Each {@link UnsatisfiedReferenceDTO} in the vector represents an
   * unsatisfied reference of the component configuration. The vector must be
   * empty if the component configuration has no unsatisfied references.
   */
  std::vector<UnsatisfiedReferenceDTO>	unsatisfiedReferences;
};
}
}
}
}
}

#endif /* ComponentConfigurationDTO_hpp */
