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

#include "../TestUtils.hpp"
#include "ConfigurationAdminTestingConfig.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/util/FileSystem.h"

namespace test
{

    cppmicroservices::Bundle
    InstallAndStartBundle(::cppmicroservices::BundleContext frameworkCtx, std::string const& libName)
    {
        std::vector<cppmicroservices::Bundle> bundles;

#if defined(US_BUILD_SHARED_LIBS)
        bundles = frameworkCtx.InstallBundles(cppmicroservices::testing::LIB_PATH + cppmicroservices::util::DIR_SEP
                                              + US_LIB_PREFIX + libName + US_LIB_POSTFIX + US_LIB_EXT);
#else
        bundles = frameworkCtx.GetBundles();
#endif

        for (auto b : bundles)
        {
            if (b.GetSymbolicName() == libName)
            {
                b.Start();
                return b;
            }
        }
        return {};
    }

} // namespace test
