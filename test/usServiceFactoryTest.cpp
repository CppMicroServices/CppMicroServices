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

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModule.h>
#include <usModuleRegistry.h>
#include <usServiceObjects.h>
#include <usSharedLibrary.h>

#include "usTestingMacros.h"
#include "usTestingConfig.h"

US_USE_NAMESPACE

namespace {

#ifdef US_PLATFORM_WINDOWS
  static const std::string LIB_PATH = US_RUNTIME_OUTPUT_DIRECTORY;
#else
  static const std::string LIB_PATH = US_LIBRARY_OUTPUT_DIRECTORY;
#endif

} // end unnamed namespace


US_BEGIN_NAMESPACE

struct TestModuleH
{
  virtual ~TestModuleH() {}
};

struct TestModuleH2
{
  virtual ~TestModuleH2() {}
};

US_END_NAMESPACE

US_DECLARE_SERVICE_INTERFACE(US_PREPEND_NAMESPACE(TestModuleH), "org.cppmicroservices.TestModuleH")
US_DECLARE_SERVICE_INTERFACE(US_PREPEND_NAMESPACE(TestModuleH2), "org.cppmicroservices.TestModuleH2")


void TestServiceFactoryModuleScope()
{

  // Install and start test module H, a service factory and test that the methods
  // in that interface works.

  SharedLibrary target(LIB_PATH, "TestModuleH");

#ifdef US_BUILD_SHARED_LIBS
  try
  {
    target.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG( << "Failed to load module, got exception: " << e.what())
  }

  Module* moduleH = ModuleRegistry::GetModule("TestModuleH Module");
  US_TEST_CONDITION_REQUIRED(moduleH != 0, "Test for existing module TestModuleH")
#endif

  std::vector<ServiceReferenceU> registeredRefs = moduleH->GetRegisteredServices();
  US_TEST_CONDITION_REQUIRED(registeredRefs.size() == 2, "# of registered services")
  US_TEST_CONDITION(registeredRefs[0].GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() == ServiceConstants::SCOPE_MODULE(), "service scope")
  US_TEST_CONDITION(registeredRefs[1].GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() == ServiceConstants::SCOPE_PROTOTYPE(), "service scope")

  ModuleContext* mc = GetModuleContext();
  // Check that a service reference exist
  const ServiceReferenceU sr1 = mc->GetServiceReference("org.cppmicroservices.TestModuleH");
  US_TEST_CONDITION_REQUIRED(sr1, "Service shall be present.")
  US_TEST_CONDITION(sr1.GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() == ServiceConstants::SCOPE_MODULE(), "service scope")

  void* service = mc->GetService(sr1);
  US_TEST_CONDITION_REQUIRED(service != NULL, "GetService()")

  void* service2 = mc->GetService(sr1);
  US_TEST_CONDITION(service == service2, "Same service pointer")

  std::vector<ServiceReferenceU> usedRefs = mc->GetModule()->GetServicesInUse();
  US_TEST_CONDITION_REQUIRED(usedRefs.size() == 1, "services in use")
  US_TEST_CONDITION(usedRefs[0] == sr1, "service ref in use")

#ifdef US_BUILD_SHARED_LIBS
  void* service3 = moduleH->GetModuleContext()->GetService(sr1);
  US_TEST_CONDITION(service != service3, "Different service pointer")
  US_TEST_CONDITION(moduleH->GetModuleContext()->UngetService(sr1), "UngetService()")
#endif

  US_TEST_CONDITION_REQUIRED(mc->UngetService(sr1), "ungetService()")

  target.Unload();
}

void TestServiceFactoryPrototypeScope()
{

  // Install and start test module H, a service factory and test that the methods
  // in that interface works.

  SharedLibrary target(LIB_PATH, "TestModuleH");

#ifdef US_BUILD_SHARED_LIBS
  try
  {
    target.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG( << "Failed to load module, got exception: " << e.what())
  }

  Module* moduleH = ModuleRegistry::GetModule("TestModuleH Module");
  US_TEST_CONDITION_REQUIRED(moduleH != 0, "Test for existing module TestModuleH")
#endif

  ModuleContext* mc = GetModuleContext();
  // Check that a service reference exist
  const ServiceReference<TestModuleH2> sr1 = mc->GetServiceReference<TestModuleH2>();
  US_TEST_CONDITION_REQUIRED(sr1, "Service shall be present.")
  US_TEST_CONDITION(sr1.GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() == ServiceConstants::SCOPE_PROTOTYPE(), "service scope")

  ServiceObjects<TestModuleH2> svcObjects = mc->GetServiceObjects(sr1);
  TestModuleH2* prototypeServiceH2 = svcObjects.GetService();

  // There should be only one service in use
  US_TEST_CONDITION_REQUIRED(mc->GetModule()->GetServicesInUse().size() == 1, "services in use")


  TestModuleH2* moduleScopeService = mc->GetService(sr1);
  US_TEST_CONDITION_REQUIRED(moduleScopeService && moduleScopeService != prototypeServiceH2, "GetService()")

  TestModuleH2* moduleScopeService2 = mc->GetService(sr1);
  US_TEST_CONDITION(moduleScopeService == moduleScopeService2, "Same service pointer")

  std::vector<ServiceReferenceU> usedRefs = mc->GetModule()->GetServicesInUse();
  US_TEST_CONDITION_REQUIRED(usedRefs.size() == 1, "services in use")
  US_TEST_CONDITION(usedRefs[0] == sr1, "service ref in use")

  std::string filter = "(" + ServiceConstants::SERVICE_ID() + "=" + sr1.GetProperty(ServiceConstants::SERVICE_ID()).ToString() + ")";
  const ServiceReference<TestModuleH> sr2 = mc->GetServiceReferences<TestModuleH>(filter).front();
  US_TEST_CONDITION_REQUIRED(sr2, "Service shall be present.")
  US_TEST_CONDITION(sr2.GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() == ServiceConstants::SCOPE_PROTOTYPE(), "service scope")
  US_TEST_CONDITION(any_cast<long>(sr2.GetProperty(ServiceConstants::SERVICE_ID())) == any_cast<long>(sr1.GetProperty(ServiceConstants::SERVICE_ID())), "same service id")

  try
  {
    svcObjects.UngetService(moduleScopeService2);
    US_TEST_FAILED_MSG(<< "std::invalid_argument exception expected")
  }
  catch (const std::invalid_argument&r)
  {
    // this is expected
  }

  // There should still be only one service in use
  usedRefs = mc->GetModule()->GetServicesInUse();
  US_TEST_CONDITION_REQUIRED(usedRefs.size() == 1, "services in use")

  ServiceObjects<TestModuleH2> svcObjects2 = svcObjects;
  ServiceObjects<TestModuleH2> svcObjects3 = mc->GetServiceObjects(sr1);
  try
  {
    svcObjects3.UngetService(prototypeServiceH2);
    US_TEST_FAILED_MSG(<< "std::invalid_argument exception expected")
  }
  catch (const std::invalid_argument&)
  {
    // this is expected
  }

  svcObjects2.UngetService(prototypeServiceH2);
  prototypeServiceH2 = svcObjects2.GetService();
  TestModuleH2* prototypeServiceH2_2 = svcObjects3.GetService();

  US_TEST_CONDITION_REQUIRED(prototypeServiceH2_2 && prototypeServiceH2_2 != prototypeServiceH2, "prototype service")

  svcObjects2.UngetService(prototypeServiceH2);
  svcObjects3.UngetService(prototypeServiceH2_2);
  US_TEST_CONDITION_REQUIRED(mc->UngetService(sr1), "ungetService()")

  target.Unload();
}

int usServiceFactoryTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceFactoryTest");

  TestServiceFactoryModuleScope();
  TestServiceFactoryPrototypeScope();

  US_TEST_END()
}
