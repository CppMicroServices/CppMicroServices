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

#include "TestBundleBService.h"

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleImport.h"

#include <iostream>

namespace cppmicroservices
{

    struct TestBundleB : public TestBundleBService
    {

        TestBundleB() {}
        virtual ~TestBundleB() {}
    };

    class TestBundleBActivator : public BundleActivator
    {
      public:
        TestBundleBActivator() {}
        ~TestBundleBActivator() {}

        void
        Start(BundleContext context)
        {
            s = std::make_shared<TestBundleB>();
            sr = context.RegisterService<TestBundleBService>(s);
        }

        void
        Stop(BundleContext)
        {
            sr.Unregister();
        }

      private:
        std::shared_ptr<TestBundleB> s;
        ServiceRegistration<TestBundleBService> sr;
    };
} // namespace cppmicroservices

CPPMICROSERVICES_IMPORT_BUNDLE(TestBundleImportedByB)

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(cppmicroservices::TestBundleBActivator)
