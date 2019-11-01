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

#ifndef CCSatisfiedState_hpp
#define CCSatisfiedState_hpp

#include "ComponentConfigurationState.hpp"

using cppmicroservices::service::component::runtime::dto::ComponentState;

namespace cppmicroservices {
namespace scrimpl {

/**
 * Abstract class that implements the #Deactivate transition method for \c SATISFIED states.
 */
class CCSatisfiedState : public ComponentConfigurationState
{
public:
  CCSatisfiedState();
  ~CCSatisfiedState() override = default;
  CCSatisfiedState(const CCSatisfiedState&) = delete;
  CCSatisfiedState& operator=(const CCSatisfiedState&) = delete;
  CCSatisfiedState(CCSatisfiedState&&) = delete;
  CCSatisfiedState& operator=(CCSatisfiedState&&) = delete;
  /**
   * This method is used by both {@link CCRegisteredState} and {@link CCActiveState}
   * to handle the deactivation of the component
   */
  void Deactivate(ComponentConfigurationImpl& mgr) override;

  /**
   * Returns {@link ComponentState::SATISFIED} to indicate the
   * state represented by this object
   */
  ComponentState GetValue() const override { return ComponentState::SATISFIED; }
};
}
}
#endif // CCSatisfiedState_hpp
