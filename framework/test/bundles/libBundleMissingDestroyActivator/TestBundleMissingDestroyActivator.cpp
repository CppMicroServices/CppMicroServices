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

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/GlobalConfig.h"

namespace cppmicroservices
{
    class TestBundleMissingDestroyActivator final : public BundleActivator
    {
      public:
        void Start(BundleContext) override {}
        void Stop(BundleContext) override {}
    };
}

//manually make a Create Activator function but not a Destroy Activator function
extern "C" US_ABI_EXPORT cppmicroservices::BundleActivator*
US_CREATE_ACTIVATOR_FUNC(TestBundleMissingDestroyActivator)()
{
    return new cppmicroservices::TestBundleMissingDestroyActivator();
}

