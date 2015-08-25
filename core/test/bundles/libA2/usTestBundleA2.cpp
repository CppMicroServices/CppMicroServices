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
#include "usLog_p.h"

US_BEGIN_NAMESPACE

struct TestBundleA2 : public TestBundleA2Service
{

  TestBundleA2(BundleContext* mc)
  {
    US_INFO << "Registering TestBundleA2Service";
    sr = mc->RegisterService<TestBundleA2Service>(this);
  }

  void Unregister()
  {
    if (sr)
    {
      sr.Unregister();
    }
  }

private:

  ServiceRegistration<TestBundleA2Service> sr;
};

class TestBundleA2Activator : public BundleActivator
{
public:

  TestBundleA2Activator() : s(0) {}

  ~TestBundleA2Activator() { delete s; }

  void Start(BundleContext* context)
  {
    s = new TestBundleA2(context);
  }

  void Stop(BundleContext* /*context*/)
  {
    s->Unregister();
  }

private:

  TestBundleA2* s;
};

US_END_NAMESPACE

US_EXPORT_BUNDLE_ACTIVATOR(US_PREPEND_NAMESPACE(TestBundleA2Activator))
