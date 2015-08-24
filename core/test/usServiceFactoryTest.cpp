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

#include <usFrameworkFactory.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModule.h>
#include <usServiceObjects.h>

#include "usTestUtils.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

US_USE_NAMESPACE

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


void TestServiceFactoryModuleScope(ModuleContext* mc)
{

  // Install and start test module H, a service factory and test that the methods
  // in that interface works.

  InstallTestBundle(mc, "TestModuleH");

  Module* moduleH = mc->GetModule("TestModuleH");
  US_TEST_CONDITION_REQUIRED(moduleH != 0, "Test for existing module TestModuleH")

  moduleH->Start();

  std::vector<ServiceReferenceU> registeredRefs = moduleH->GetRegisteredServices();
  US_TEST_CONDITION_REQUIRED(registeredRefs.size() == 2, "# of registered services")
  US_TEST_CONDITION(registeredRefs[0].GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() == ServiceConstants::SCOPE_MODULE(), "service scope")
  US_TEST_CONDITION(registeredRefs[1].GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() == ServiceConstants::SCOPE_PROTOTYPE(), "service scope")

  // Check that a service reference exist
  const ServiceReferenceU sr1 = mc->GetServiceReference("us::TestModuleH");
  US_TEST_CONDITION_REQUIRED(sr1, "Service shall be present.")
  US_TEST_CONDITION(sr1.GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() == ServiceConstants::SCOPE_MODULE(), "service scope")

  InterfaceMap service = mc->GetService(sr1);
  US_TEST_CONDITION_REQUIRED(service.size() >= 1, "GetService()")
  InterfaceMap::const_iterator serviceIter = service.find("us::TestModuleH");
  US_TEST_CONDITION_REQUIRED(serviceIter != service.end(), "GetService()")
  US_TEST_CONDITION_REQUIRED(serviceIter->second != NULL, "GetService()")

  InterfaceMap service2 = mc->GetService(sr1);
  US_TEST_CONDITION(service == service2, "Same service pointer")

  std::vector<ServiceReferenceU> usedRefs = mc->GetModule()->GetServicesInUse();
  US_TEST_CONDITION_REQUIRED(usedRefs.size() == 1, "services in use")
  US_TEST_CONDITION(usedRefs[0] == sr1, "service ref in use")

  InterfaceMap service3 = moduleH->GetModuleContext()->GetService(sr1);
  US_TEST_CONDITION(service != service3, "Different service pointer")
  US_TEST_CONDITION(moduleH->GetModuleContext()->UngetService(sr1), "UngetService()")

  US_TEST_CONDITION_REQUIRED(mc->UngetService(sr1) == false, "ungetService()")
  US_TEST_CONDITION_REQUIRED(mc->UngetService(sr1) == true, "ungetService()")

  moduleH->Stop();
}

void TestServiceFactoryPrototypeScope(ModuleContext* mc)
{

  // Install and start test module H, a service factory and test that the methods
  // in that interface works.

  InstallTestBundle(mc, "TestModuleH");

  Module* moduleH = mc->GetModule("TestModuleH");
  US_TEST_CONDITION_REQUIRED(moduleH != 0, "Test for existing module TestModuleH")

  moduleH->Start();

  // Check that a service reference exist
  const ServiceReference<TestModuleH2> sr1 = mc->GetServiceReference<TestModuleH2>();
  US_TEST_CONDITION_REQUIRED(sr1, "Service shall be present.")
  US_TEST_CONDITION(sr1.GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() == ServiceConstants::SCOPE_PROTOTYPE(), "service scope")

  ServiceObjects<TestModuleH2> svcObjects = mc->GetServiceObjects(sr1);
  TestModuleH2* prototypeServiceH2 = svcObjects.GetService();

  const ServiceReferenceU sr1void = mc->GetServiceReference(us_service_interface_iid<TestModuleH2>());
  ServiceObjects<void> svcObjectsVoid = mc->GetServiceObjects(sr1void);
  InterfaceMap prototypeServiceH2Void = svcObjectsVoid.GetService();
  US_TEST_CONDITION_REQUIRED(prototypeServiceH2Void.find(us_service_interface_iid<TestModuleH2>()) != prototypeServiceH2Void.end(),
                             "ServiceObjects<void>::GetService()")

#ifdef US_BUILD_SHARED_LIBS
  // There should be only one service in use
  US_TEST_CONDITION_REQUIRED(mc->GetModule()->GetServicesInUse().size() == 1, "services in use")
#endif

  TestModuleH2* moduleScopeService = mc->GetService(sr1);
  US_TEST_CONDITION_REQUIRED(moduleScopeService && moduleScopeService != prototypeServiceH2, "GetService()")

  US_TEST_CONDITION_REQUIRED(prototypeServiceH2 != prototypeServiceH2Void.find(us_service_interface_iid<TestModuleH2>())->second,
                             "GetService()")

  svcObjectsVoid.UngetService(prototypeServiceH2Void);

  TestModuleH2* moduleScopeService2 = mc->GetService(sr1);
  US_TEST_CONDITION(moduleScopeService == moduleScopeService2, "Same service pointer")

#ifdef US_BUILD_SHARED_LIBS
  std::vector<ServiceReferenceU> usedRefs = mc->GetModule()->GetServicesInUse();
  US_TEST_CONDITION_REQUIRED(usedRefs.size() == 1, "services in use")
  US_TEST_CONDITION(usedRefs[0] == sr1, "service ref in use")
#endif

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
  catch (const std::invalid_argument&)
  {
    // this is expected
  }

#ifdef US_BUILD_SHARED_LIBS
  // There should still be only one service in use
  usedRefs = mc->GetModule()->GetServicesInUse();
  US_TEST_CONDITION_REQUIRED(usedRefs.size() == 1, "services in use")
#endif

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
  US_TEST_CONDITION_REQUIRED(mc->UngetService(sr1) == false, "ungetService()")
  US_TEST_CONDITION_REQUIRED(mc->UngetService(sr1) == true, "ungetService()")

  moduleH->Stop();
}

int usServiceFactoryTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceFactoryTest");

  FrameworkFactory factory;
  Framework* framework = factory.newFramework(std::map<std::string, std::string>());
  framework->init();
  framework->Start();

  TestServiceFactoryModuleScope(framework->GetModuleContext());
  TestServiceFactoryPrototypeScope(framework->GetModuleContext());

  delete framework;

  US_TEST_END()
}
