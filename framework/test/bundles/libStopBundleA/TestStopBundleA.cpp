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

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/GlobalConfig.h"

#include <algorithm>
#include <iostream>

namespace cppmicroservices {

class TestStopBundleAActivator : public BundleActivator
{
public:
  TestStopBundleAActivator() {}
  ~TestStopBundleAActivator() {}

  void Start(BundleContext) { }

  void Stop(BundleContext context) 
  {
    auto bundles = context.GetBundles();
    auto bundleA = std::find_if(bundles.begin(),
                                bundles.end(),
                                [](const cppmicroservices::Bundle& b) { return "TestBundleA" == b.GetSymbolicName(); });

    if (std::end(bundles) != bundleA)
    {
      (*bundleA).Stop();
    }      
  }
};

}

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(cppmicroservices::TestStopBundleAActivator)
