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

#include "usTestModuleBService.h"

#include <usModuleImport.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>

US_BEGIN_NAMESPACE

struct TestModuleB : public TestModuleBService
{

  TestModuleB(ModuleContext* mc)
  {
    US_INFO << "Registering TestModuleBService";
    mc->RegisterService<TestModuleBService>(this);
  }

};

class TestModuleBActivator : public ModuleActivator
{
public:

  TestModuleBActivator() : s(0) {}
  ~TestModuleBActivator() { delete s; }

  void Load(ModuleContext* context)
  {
    s = new TestModuleB(context);
  }

  void Unload(ModuleContext*)
  {
  }

private:

  TestModuleB* s;
};

US_END_NAMESPACE

US_EXPORT_MODULE_ACTIVATOR(US_PREPEND_NAMESPACE(TestModuleBActivator))

US_IMPORT_MODULE(TestModuleImportedByB)
