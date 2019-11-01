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

#include "CCSatisfiedState.hpp"
#include "CCUnsatisfiedReferenceState.hpp"
#include "../ComponentConfigurationImpl.hpp"

using cppmicroservices::service::component::runtime::dto::UNSATISFIED_REFERENCE;
namespace cppmicroservices {
namespace scrimpl {

CCSatisfiedState::CCSatisfiedState() = default;

void CCSatisfiedState::Deactivate(ComponentConfigurationImpl& mgr)
{
  auto currentState = shared_from_this();
  std::packaged_task<void(void)> task([&mgr](){
                                        mgr.UnregisterService();
                                        mgr.DestroyComponentInstances();
                                      });
  auto unsatisfiedState = std::make_shared<CCUnsatisfiedReferenceState>(task.get_future().share());
  while(currentState->GetValue() != service::component::runtime::dto::UNSATISFIED_REFERENCE)
  {
    if(mgr.CompareAndSetState(&currentState, unsatisfiedState))
    {
      currentState->WaitForTransitionTask(); // wait for the previous transition to finish
      task();
      break;
    }
  }
}
}
}
