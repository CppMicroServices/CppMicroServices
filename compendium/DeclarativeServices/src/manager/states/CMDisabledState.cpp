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

namespace cppmicroservices
{
    namespace scrimpl
    {

        CMDisabledState::CMDisabledState()
        {
            // Initialization with a valid future is required to facilitate a request
            // to DISABLE a ComponentManager whose initial state is DISABLED
            std::packaged_task<void()> task([]() { /*empty task*/ });
            fut = task.get_future().share();
            task();
        }

        std::shared_future<void>
        CMDisabledState::Enable(ComponentManagerImpl& cm, std::shared_ptr<std::atomic<bool>> asyncStarted)
        {
            auto currentState = shared_from_this(); // assume this object is the current state object.
            return cm.PostAsyncDisabledToEnabled(currentState, asyncStarted);
        }

        // if already in disabled state, simply return the existing future object. Equivalent to a no-op.
        std::shared_future<void>
        CMDisabledState::Disable(ComponentManagerImpl& /*cm*/, std::shared_ptr<std::atomic<bool>> /*asyncStarted*/)
        {
            return GetFuture();
        }

        // There are no configurations for a disabled state. Equivalent to a no-op.
        std::vector<std::shared_ptr<ComponentConfiguration>>
        CMDisabledState::GetConfigurations(ComponentManagerImpl const&) const
        {
            return {};
        }
    } // namespace scrimpl
} // namespace cppmicroservices
