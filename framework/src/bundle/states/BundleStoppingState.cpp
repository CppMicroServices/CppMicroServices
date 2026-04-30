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

#include "BundleStoppingState.hpp"

#include "../BundlePrivate.h"

namespace cppmicroservices
{

    BundleStoppingState::BundleStoppingState()
    {
        std::promise<void> prom;
        ready = prom.get_future();
        prom.set_value();
    }

    BundleStoppingState::BundleStoppingState(std::shared_future<void> blockUntil)
        : ready(std::move(blockUntil))
    {
    }

    void
    BundleStoppingState::Start(BundlePrivate& bundle, uint32_t /*options*/)
    {
        // Cannot start while stopping
        throw std::runtime_error("Bundle " + bundle.symbolicName + " (location=" + bundle.location
                                 + "), start called from BundleActivator::Stop");
    }

    std::exception_ptr
    BundleStoppingState::Stop(BundlePrivate& /*bundle*/, uint32_t /*options*/)
    {
        // Already stopping -- no-op
        return nullptr;
    }

    void
    BundleStoppingState::Uninstall(BundlePrivate& bundle)
    {
        // Signal abort to the in-progress stop, then uninstall
        bundle.aborted = static_cast<uint8_t>(BundlePrivate::Aborted::YES);
        bundle.RemoveAndCleanupBundle();
    }

} // namespace cppmicroservices
