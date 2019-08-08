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

#ifndef CCActiveState_hpp
#define CCActiveState_hpp

#include "CCSatisfiedState.hpp"
#include "../ConcurrencyUtil.hpp"

using cppmicroservices::service::component::runtime::dto::ComponentState;

namespace cppmicroservices {
namespace scrimpl {
    
/**
 * This class represents the {\code ComponentState::ACTIVE} state of a
 * component configuration.
 */
class CCActiveState final
  : public CCSatisfiedState
{
public:
  CCActiveState();
  ~CCActiveState() override = default;
  CCActiveState(const CCActiveState&) = delete;
  CCActiveState& operator=(const CCActiveState&) = delete;
  CCActiveState(CCActiveState&&) = delete;
  CCActiveState& operator=(CCActiveState&&) = delete;

  void Register(ComponentConfigurationImpl&) override
  {
    // no-op, already resolved
  };
  std::shared_ptr<ComponentInstance> Activate(ComponentConfigurationImpl& mgr,
                                              const cppmicroservices::Bundle& clientBundle) override;

  /**
   * Returns {@link ComponentState::ACTIVE} to indicate the 
   * state represented by this object
   */
  ComponentState GetValue() const override { return ComponentState::ACTIVE; }

  void WaitForTransitionTask() override
  {
    latch.Wait();
  }
private:
  CounterLatch latch;
};
}
}
#endif // CCActiveState_hpp
