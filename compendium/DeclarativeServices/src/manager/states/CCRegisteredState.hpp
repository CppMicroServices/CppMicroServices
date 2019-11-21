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

#ifndef CCRegisteredState_hpp
#define CCRegisteredState_hpp

#include "CCSatisfiedState.hpp"

using cppmicroservices::service::component::runtime::dto::ComponentState;

namespace cppmicroservices {
namespace scrimpl {

/**
 * This class represents the {\code ComponentState::SATISFIED} state of a
 * component configuration. This state indicates that the component's service
 * is registered with the framework
 */
class CCRegisteredState final
  : public CCSatisfiedState
{
public:
  CCRegisteredState();
  CCRegisteredState(std::future<void> blockUntil);
  ~CCRegisteredState() override = default;
  CCRegisteredState(const CCRegisteredState&) = delete;
  CCRegisteredState& operator=(const CCRegisteredState&) = delete;
  CCRegisteredState(CCRegisteredState&&) = delete;
  CCRegisteredState& operator=(CCRegisteredState&&) = delete;

  void Register(ComponentConfigurationImpl& /*mgr*/) override {
    // no-op, already resolved
  };

  /**
   * This method is used to trigger a transition from current state to \c ACTIVE state
   */
  std::shared_ptr<ComponentInstance> Activate(ComponentConfigurationImpl& mgr,
                                              const cppmicroservices::Bundle& clientBundle) override;

  /**
   * Method blocks the current thread until the stored future is ready
   */
  void WaitForTransitionTask() override { ready.get(); }
private:
  std::future<void> ready;
};
}
}
#endif // CCRegisteredState_hpp
