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

#include "CCActiveState.hpp"
#include "../ComponentConfigurationImpl.hpp"
#include "CCUnsatisfiedReferenceState.hpp"
#include "cppmicroservices/SharedLibraryException.h"

namespace cppmicroservices {
namespace scrimpl {

class LatchScopeGuard
{
public:
  LatchScopeGuard(std::function<void()> cleanupFcn)
    : _cleanupFcn(std::move(cleanupFcn))
  {}
  ~LatchScopeGuard() { _cleanupFcn(); }

private:
  std::function<void()> _cleanupFcn;
};

CCActiveState::CCActiveState() = default;

std::shared_ptr<ComponentInstance> CCActiveState::Activate(ComponentConfigurationImpl& mgr,
                                                           const cppmicroservices::Bundle& clientBundle)
{
  // no state change, already in active state. create and return a ComponentInstance object
  std::shared_ptr<ComponentInstance> instance;
  auto logger = mgr.GetLogger();
  if(latch.CountUp())
  {
    {
      LatchScopeGuard sg([this, logger]() {
        // By using try/catch here, we ensure that this lambda function doesn't
        // throw inside LatchScopeGuard's dtor.
        try {
          latch.CountDown();
        } catch (...) {
          logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                      "latch.CountDown() threw an exception during "
                      "LatchScopeGuard cleanup.",
                      std::current_exception());
        }
      });

      // This could throw; a scope guard is put in place to call
      // latch.CountDown().
      instance = mgr.CreateAndActivateComponentInstance(clientBundle);
    }

    if(!instance)
    {
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, "Component configuration activation failed");
    }
    return instance;
  }

  // do not allow any new component instances to be created if Deactivate was called
  logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG, "Component configuration activation failed because component is not in active state");
  return nullptr;
      
}
}
}
