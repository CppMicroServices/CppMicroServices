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

#include "usTestBundleBService.h"

#include <usBundleActivator.h>
#include <usBundleContext.h>
#include "usLog_p.h"

US_BEGIN_NAMESPACE

struct TestBundleImportedByB : public TestBundleBService
{

  TestBundleImportedByB(BundleContext* mc)
  {
    US_INFO << "Registering TestBundleImportedByB";
    mc->RegisterService<TestBundleBService>(this);
  }

};

class TestBundleImportedByBActivator : public BundleActivator
{
public:

  TestBundleImportedByBActivator() : s(0) {}
  ~TestBundleImportedByBActivator() { delete s; }

  void Start(BundleContext* context)
  {
    s = new TestBundleImportedByB(context);
  }

  void Stop(BundleContext*)
  {
  }

private:

  TestBundleImportedByB* s;
};

US_END_NAMESPACE

US_EXPORT_BUNDLE_ACTIVATOR(US_PREPEND_NAMESPACE(TestBundleImportedByBActivator))
