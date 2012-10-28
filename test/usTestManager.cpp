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

#include "usTestManager.h"

#include "usModuleImport.h"

US_BEGIN_NAMESPACE

TestManager& TestManager::GetInstance()
{
  static TestManager instance;
  return instance;
}

void TestManager::Initialize()
{
  m_FailedTests = 0;
  m_PassedTests = 0;
}

int TestManager::NumberOfFailedTests()
{
  return m_FailedTests;
}

int TestManager::NumberOfPassedTests()
{
  return m_PassedTests;
}

void TestManager::TestFailed()
{
  m_FailedTests++;
}

void TestManager::TestPassed()
{
  m_PassedTests++;
}

US_END_NAMESPACE

#ifndef US_BUILD_SHARED_LIBS
US_IMPORT_MODULE(TestModuleA)
US_IMPORT_MODULE(TestModuleA2)
US_IMPORT_MODULE(TestModuleB)
US_IMPORT_MODULE(TestModuleS)
US_IMPORT_MODULE(TestModuleSL1)
US_IMPORT_MODULE(TestModuleSL3)
US_IMPORT_MODULE(TestModuleSL4)

US_LOAD_IMPORTED_MODULES_INTO_MAIN(TestModuleA TestModuleA2 TestModuleB TestModuleImportedByB TestModuleS TestModuleSL1 TestModuleSL3 TestModuleSL4)
#endif
