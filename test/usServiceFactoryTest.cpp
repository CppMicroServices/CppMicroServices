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

#include "usTestingMacros.h"

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModule.h>
#include <usModuleRegistry.h>

#include "usTestUtilSharedLibrary.h"

US_USE_NAMESPACE

void TestServiceFactoryModuleScope()
{

  // Install and start test module H, a service factory and test that the methods
  // in that interface works.

  SharedLibraryHandle target("TestModuleH");

  try
  {
    target.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG( << "Failed to load module, got exception: " << e.what())
  }

#ifdef US_BUILD_SHARED_LIBS
  Module* moduleH = ModuleRegistry::GetModule("TestModuleH Module");
  US_TEST_CONDITION_REQUIRED(moduleH != 0, "Test for existing module TestModuleH")
#endif

  ModuleContext* mc = GetModuleContext();
  // Check that a service reference exist
  const ServiceReferenceU sr1 = mc->GetServiceReference("org.cppmicroservices.TestModuleH");
  US_TEST_CONDITION_REQUIRED(sr1, "Service shall be present.")

  void* service = mc->GetService(sr1);
  US_TEST_CONDITION_REQUIRED(service != NULL, "GetService()")

  void* service2 = mc->GetService(sr1);
  US_TEST_CONDITION(service == service2, "Same service pointer")

#ifdef US_BUILD_SHARED_LIBS
  void* service3 = moduleH->GetModuleContext()->GetService(sr1);
  US_TEST_CONDITION(service != service3, "Different service pointer")
  US_TEST_CONDITION(moduleH->GetModuleContext()->UngetService(sr1), "UngetService()")
#endif

  US_TEST_CONDITION_REQUIRED(mc->UngetService(sr1), "ungetService()")

  target.Unload();
}

int usServiceFactoryTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceFactoryTest");

  TestServiceFactoryModuleScope();

  US_TEST_END()
}
