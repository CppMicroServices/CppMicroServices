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
#include "CMDisabledState.hpp"
#include "../ComponentConfiguration.hpp"
#include "../ComponentManagerImpl.hpp"
#include "CMEnabledState.hpp"
#include "cppmicroservices/SharedLibraryException.h"
#include <cassert>

namespace cppmicroservices {
namespace scrimpl {

CMDisabledState::CMDisabledState()
{
  // Initialization with a valid future is required to facilitate a request
  // to DISABLE a ComponentManager whose initial state is DISABLED
  std::packaged_task<void()> task([](){ /*empty task*/ });
  fut = task.get_future().share();
  task();
}

std::shared_future<void> CMDisabledState::Enable(ComponentManagerImpl& cm)
{
  auto currentState = shared_from_this(); // assume this object is the current state object.
  auto metadata = cm.GetMetadata();
  auto bundle = cm.GetBundle();
  auto reg = cm.GetRegistry();
  auto logger = cm.GetLogger();
  std::packaged_task<void(std::shared_ptr<CMEnabledState>, std::exception_ptr&)>
    task([metadata, bundle, reg, logger](std::shared_ptr<CMEnabledState> eState,
                                         std::exception_ptr& ptr) {
      try {
        eState->CreateConfigurations(metadata, bundle, reg, logger);
      } catch (const cppmicroservices::SharedLibraryException&) {
        ptr = std::current_exception();
      }
    });
  auto enabledState = std::make_shared<CMEnabledState>(task.get_future().share());

  // if this object failed to change state and the current state is DISABLED, try again
  auto succeeded = false;
  do
  {
    succeeded = cm.CompareAndSetState(&currentState, enabledState);
  } while(!succeeded && !currentState->IsEnabled(cm));

  if(succeeded) // succeeded in changing the state
  {
    auto futObj =
      std::async(std::launch::async,
                 [enabledState, transition = std::move(task)]() mutable {
                   std::exception_ptr ptr;
                   transition(enabledState, ptr);
                   if (ptr) {
                     std::rethrow_exception(ptr);
                   }
                 })
        .share();
    return futObj;
  }
  // return the stored future in the current enabled state object
  return currentState->GetFuture();
}

// if already in disabled state, simply return the existing future object. Equivalent to a no-op.
std::shared_future<void> CMDisabledState::Disable(ComponentManagerImpl& /*cm*/)
{
  return GetFuture();
}

// There are no configurations for a disabled state. Equivalent to a no-op.
std::vector<std::shared_ptr<ComponentConfiguration>> CMDisabledState::GetConfigurations(const ComponentManagerImpl&) const
{
  return {};
}
}
}
