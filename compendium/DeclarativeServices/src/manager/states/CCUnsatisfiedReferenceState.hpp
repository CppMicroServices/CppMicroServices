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

#ifndef CCUnsatisfiedReferenceState_hpp
#define CCUnsatisfiedReferenceState_hpp

#include "ComponentConfigurationState.hpp"

using cppmicroservices::service::component::runtime::dto::ComponentState;

namespace cppmicroservices {
namespace scrimpl {

/**
 * This class is a subclass of {\code ComponentConfigurationState} which
 * represents the {\code ComponentState::UNSATISFIED_REFERENCE} state of a
 * component configuration.
 */
class CCUnsatisfiedReferenceState final : public ComponentConfigurationState
{
public:
  CCUnsatisfiedReferenceState();
  explicit CCUnsatisfiedReferenceState(std::shared_future<void> blockUntil);
  ~CCUnsatisfiedReferenceState() override = default;
  CCUnsatisfiedReferenceState(const CCUnsatisfiedReferenceState&) = delete;
  CCUnsatisfiedReferenceState& operator=(const CCUnsatisfiedReferenceState&) = delete;
  CCUnsatisfiedReferenceState(CCUnsatisfiedReferenceState&&) = delete;
  CCUnsatisfiedReferenceState& operator=(CCUnsatisfiedReferenceState&&) = delete;

  /**
   * This method will set handle the operations for transitioning the state
   * from current state to SATISFIED state.
   */
  void Register(ComponentConfigurationImpl& mgr) override;

  /**
   * Calling an Activate transition on UNSATISFIED_REFERENCE state is a no-op
   */
  std::shared_ptr<ComponentInstance> Activate(ComponentConfigurationImpl& /*mgr*/,
                                              const cppmicroservices::Bundle& /*clientBundle*/) override
  {
    return nullptr;
  };

  /**
   * This method does not result in a state change since the component configuration is already in
   * UNSATISFIED_REFERENCE state. This method does wait for the state transition (possibly triggered
   * by another thread) to finish. 
   */
  void Deactivate(ComponentConfigurationImpl& /*mgr*/) override {
    // wait for the transition to finish
    WaitForTransitionTask();
  };

  /**
   * Returns {\code ComponentState::UNSATISFIED_REFERENCE} to indicate the
   * state represented by this object
   */
  ComponentState GetValue() const override { return ComponentState::UNSATISFIED_REFERENCE; }

  void WaitForTransitionTask() override { ready.get(); }
private:
  std::shared_future<void> ready;
};
}
}
#endif // CCUnsatisfiedReferenceState_hpp
