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

#include <cassert>

#include "CCUnsatisfiedReferenceState.hpp"
#include "CCRegisteredState.hpp"
#include "../ComponentConfigurationImpl.hpp"

namespace cppmicroservices {
namespace scrimpl {

CCUnsatisfiedReferenceState::CCUnsatisfiedReferenceState()
{
  std::promise<void> prom;
  ready = prom.get_future();
  prom.set_value();
}

CCUnsatisfiedReferenceState::CCUnsatisfiedReferenceState(std::shared_future<void> blockUntil)
  : ready(std::move(blockUntil))
{
}

void CCUnsatisfiedReferenceState::Register(ComponentConfigurationImpl& mgr)
{
  auto currState = shared_from_this(); // assume this is the current state object
  std::promise<void> transitionAction;
  auto fut = transitionAction.get_future();
  auto registeredState = std::make_shared<CCRegisteredState>(std::move(fut));
  while(currState->GetValue() == ComponentState::UNSATISFIED_REFERENCE)
  {
    if(mgr.CompareAndSetState(&currState, registeredState))
    {
      currState->WaitForTransitionTask(); // wait for the previous transition to finish
      if(!mgr.IsServiceProvider() || mgr.RegisterService())
      {
        transitionAction.set_value(); // unblock the next transition
        if(mgr.GetMetadata()->immediate)
        {
          mgr.Activate(cppmicroservices::Bundle());
        }
      }
      else
      {
        auto logger = mgr.GetLogger();
        logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, "Component registration failed");
        auto expectedState = std::dynamic_pointer_cast<ComponentConfigurationState>(registeredState);
        mgr.CompareAndSetState(&expectedState, std::make_shared<CCUnsatisfiedReferenceState>());
        transitionAction.set_value(); // unblock the next transition
      }
      break;
    }
  }
}
}
}
