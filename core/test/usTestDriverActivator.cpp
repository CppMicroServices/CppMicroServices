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

#include "usTestDriverActivator.h"
#include "usModuleImport.h"

US_BEGIN_NAMESPACE

TestDriverActivator* TestDriverActivator::m_Instance = 0;

TestDriverActivator::TestDriverActivator()
  : m_LoadCalled(false)
{
}

bool TestDriverActivator::LoadCalled()
{
  return m_Instance ? m_Instance->m_LoadCalled : false;
}

void TestDriverActivator::Load(ModuleContext*)
{
  this->m_Instance = this;
  this->m_LoadCalled = true;
}

void TestDriverActivator::Unload(ModuleContext*)
{
  this->m_Instance = 0;
}

US_END_NAMESPACE

US_EXPORT_MODULE_ACTIVATOR(us::TestDriverActivator)

#ifndef US_BUILD_SHARED_LIBS
US_IMPORT_MODULE(CppMicroServices)
US_IMPORT_MODULE(TestModuleA)
US_IMPORT_MODULE(TestModuleA2)
US_IMPORT_MODULE(TestModuleB)
US_IMPORT_MODULE(TestModuleH)
US_IMPORT_MODULE(TestModuleM)
US_INITIALIZE_STATIC_MODULE(TestModuleR)
US_INITIALIZE_STATIC_MODULE(TestModuleRL)
US_INITIALIZE_STATIC_MODULE(TestModuleRA)
US_IMPORT_MODULE(TestModuleS)
US_IMPORT_MODULE(TestModuleSL1)
US_IMPORT_MODULE(TestModuleSL3)
US_IMPORT_MODULE(TestModuleSL4)
US_IMPORT_MODULE(main)
#endif
