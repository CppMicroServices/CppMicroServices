/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

#include "usTestBundleA2Service.h"

#include <usBundleActivator.h>
#include <usBundleContext.h>

namespace us {

struct TestBundleA2 : public TestBundleA2Service
{
  TestBundleA2() {}
  virtual ~TestBundleA2() {}
};

class TestBundleA2Activator : public BundleActivator
{
public:

  TestBundleA2Activator() {}

  ~TestBundleA2Activator() {}

  void Start(BundleContext context)
  {
    s = std::make_shared<TestBundleA2>();
    sr = context.RegisterService<TestBundleA2Service>(s);
  }

  void Stop(BundleContext /*context*/)
  {
    sr.Unregister();
    s.reset();
  }

private:

  std::shared_ptr<TestBundleA2> s;
  ServiceRegistration<TestBundleA2Service> sr;
};

}

US_EXPORT_BUNDLE_ACTIVATOR(us::TestBundleA2Activator)
