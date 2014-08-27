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

#include "usTestModuleAService.h"

#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usGlobalConfig.h>

US_BEGIN_NAMESPACE

struct TestModuleA : public TestModuleAService
{

  TestModuleA(ModuleContext* mc)
  {
    US_INFO << "Registering TestModuleAService";
    sr = mc->RegisterService<TestModuleAService>(this);
  }

  void Unregister()
  {
    if (sr)
    {
      sr.Unregister();
      sr = 0;
    }
  }

private:

  ServiceRegistration<TestModuleAService> sr;

};

class TestModuleAActivator : public ModuleActivator
{
public:

  TestModuleAActivator() : s(0) {}
  ~TestModuleAActivator() { delete s; }

  void Load(ModuleContext* context)
  {
    s = new TestModuleA(context);
  }

  void Unload(ModuleContext*)
  {
  }

private:

  TestModuleA* s;
};

US_END_NAMESPACE

US_EXPORT_MODULE_ACTIVATOR(US_PREPEND_NAMESPACE(TestModuleAActivator))
