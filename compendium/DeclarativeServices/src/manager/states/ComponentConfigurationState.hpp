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

#ifndef ComponentConfigurationState_hpp
#define ComponentConfigurationState_hpp

#include <string>
#include <vector>
#include <memory>
#include <future>
#include "cppmicroservices/Bundle.h"

#include "cppmicroservices/servicecomponent/runtime/dto/ComponentConfigurationDTO.hpp"
#include "cppmicroservices/servicecomponent/detail/ComponentInstance.hpp"

using cppmicroservices::service::component::runtime::dto::ComponentState;
using cppmicroservices::service::component::detail::ComponentInstance;

namespace cppmicroservices {
namespace scrimpl {
class ComponentConfigurationImpl;

/**
 * Interface for state objects used in {@link ComponentConfigurationImpl} class
 */
class ComponentConfigurationState : public std::enable_shared_from_this<ComponentConfigurationState>
{
public:
  ComponentConfigurationState() = default;
  virtual ~ComponentConfigurationState() = default;
  ComponentConfigurationState(const ComponentConfigurationState&) = delete;
  ComponentConfigurationState& operator=(const ComponentConfigurationState&) = delete;
  ComponentConfigurationState(ComponentConfigurationState&&) = delete;
  ComponentConfigurationState& operator=(ComponentConfigurationState&&) = delete;

  /**
   * Implementation must handle the transition from \c UNSATISFIED_REFERENCE to \c SATISFIED
   *
   * \param mgr is the {@link ComponentConfigurationImpl} object whose state needs to change
   */
  virtual void Register(ComponentConfigurationImpl& mgr) = 0;

  /**
   * Implementation must handle the transition from \c SATISFIED to \c ACTIVE
   *
   * \param mgr is the {@link ComponentConfigurationImpl} object whose state needs to change
   * \param clientBundle is the bundle responsible for triggering the state change
   */
  virtual std::shared_ptr<ComponentInstance> Activate(ComponentConfigurationImpl& mgr,
                                                      const cppmicroservices::Bundle& clientBundle) = 0;

  /**
   * Implementation must handle the transition from \c SATISFIED or \c ACTIVE to \c UNSATISFIED_REFERENCE
   *
   * \param mgr is the {@link ComponentConfigurationImpl} object whose state needs to change
   */
  virtual void Deactivate(ComponentConfigurationImpl& mgr) = 0;

  /**
   * Returns the state as a {@link ComponentState} enum value
   */
  virtual ComponentState GetValue() const = 0;

  /**
   * Implementation must wait until the transition task is finished
   */
  virtual void WaitForTransitionTask() = 0;
};
}
}
#endif // ComponentConfigurationState_hpp
