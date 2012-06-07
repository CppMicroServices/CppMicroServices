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

#include <usLDAPFilter.h>

#include "usTestingMacros.h"
#include <usServiceInterface.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

#include US_BASECLASS_HEADER

#include <stdexcept>

US_USE_NAMESPACE

struct ITestServiceA
{
  virtual ~ITestServiceA() {}
};

US_DECLARE_SERVICE_INTERFACE(ITestServiceA, "org.cppmicroservices.testing.ITestServiceA")

int TestMultipleServiceRegistrations()
{
  struct TestServiceA : public US_BASECLASS_NAME, public ITestServiceA
  {
  };

  ModuleContext* context = GetModuleContext();

  TestServiceA s1;
  TestServiceA s2;

  ServiceRegistration reg1 = context->RegisterService<ITestServiceA>(&s1);
  ServiceRegistration reg2 = context->RegisterService<ITestServiceA>(&s2);

  std::list<ServiceReference> refs = context->GetServiceReferences<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(refs.size() == 2, "Testing for two registered ITestServiceA services")

  reg2.Unregister();
  refs = context->GetServiceReferences<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(refs.size() == 1, "Testing for one registered ITestServiceA services")

  reg1.Unregister();
  refs = context->GetServiceReferences<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(refs.empty(), "Testing for no ITestServiceA services")

  return EXIT_SUCCESS;
}


int usServiceRegistryTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceRegistryTest");

  US_TEST_CONDITION(TestMultipleServiceRegistrations() == EXIT_SUCCESS, "Testing service registrations: ")

  US_TEST_END()
}

