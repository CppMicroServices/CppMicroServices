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

#include "BundleUninstalledState.hpp"

#include "../BundlePrivate.h"

namespace cppmicroservices
{

    void
    BundleUninstalledState::Start(BundlePrivate& bundle, uint32_t /*options*/)
    {
        throw std::logic_error("Bundle " + bundle.symbolicName + " (location=" + bundle.location
                               + ") is in UNINSTALLED state");
    }

    std::exception_ptr
    BundleUninstalledState::Stop(BundlePrivate& bundle, uint32_t /*options*/)
    {
        throw std::logic_error("Bundle " + bundle.symbolicName + " (location=" + bundle.location + ") is uninstalled");
    }

    void
    BundleUninstalledState::Uninstall(BundlePrivate& bundle)
    {
        throw std::logic_error("Bundle " + bundle.symbolicName + " (location=" + bundle.location
                               + ") is in BUNDLE_UNINSTALLED state");
    }

} // namespace cppmicroservices
