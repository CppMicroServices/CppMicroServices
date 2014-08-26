/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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

#include "usTestModuleA2Service.h"

#include <usModuleActivator.h>
#include <usModuleContext.h>

US_BEGIN_NAMESPACE

struct TestModuleA2 : public TestModuleA2Service
{

  TestModuleA2(ModuleContext* mc)
  {
    US_INFO << "Registering TestModuleA2Service";
    sr = mc->RegisterService<TestModuleA2Service>(this);
  }

  void Unregister()
  {
    if (sr)
    {
      sr.Unregister();
    }
  }

private:

  ServiceRegistration<TestModuleA2Service> sr;
};

class TestModuleA2Activator : public ModuleActivator
{
public:

  TestModuleA2Activator() : s(0) {}

  ~TestModuleA2Activator() { delete s; }

  void Load(ModuleContext* context)
  {
    s = new TestModuleA2(context);
  }

  void Unload(ModuleContext* /*context*/)
  {
    s->Unregister();
  }

private:

  TestModuleA2* s;
};

US_END_NAMESPACE

US_EXPORT_MODULE_ACTIVATOR(US_PREPEND_NAMESPACE(TestModuleA2Activator))
