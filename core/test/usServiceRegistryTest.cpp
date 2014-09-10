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

#include <stdexcept>

US_USE_NAMESPACE

struct ITestServiceA
{
  virtual ~ITestServiceA() {}
};


void TestServiceInterfaceId()
{
  US_TEST_CONDITION(us_service_interface_iid<int>() == "int", "Service interface id int")
  US_TEST_CONDITION(us_service_interface_iid<ITestServiceA>() == "ITestServiceA", "Service interface id ITestServiceA")
}

void TestMultipleServiceRegistrations()
{
  struct TestServiceA : public ITestServiceA
  {
  };

  ModuleContext* context = GetModuleContext();

  TestServiceA s1;
  TestServiceA s2;

  ServiceRegistration<ITestServiceA> reg1 = context->RegisterService<ITestServiceA>(&s1);
  ServiceRegistration<ITestServiceA> reg2 = context->RegisterService<ITestServiceA>(&s2);

  std::vector<ServiceReference<ITestServiceA> > refs = context->GetServiceReferences<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(refs.size() == 2, "Testing for two registered ITestServiceA services")

  reg2.Unregister();
  refs = context->GetServiceReferences<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(refs.size() == 1, "Testing for one registered ITestServiceA services")

  reg1.Unregister();
  refs = context->GetServiceReferences<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(refs.empty(), "Testing for no ITestServiceA services")

  ServiceReference<ITestServiceA> ref = context->GetServiceReference<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(!ref, "Testing for invalid service reference")
}

void TestServicePropertiesUpdate()
{
  struct TestServiceA : public ITestServiceA
  {
  };

  ModuleContext* context = GetModuleContext();

  TestServiceA s1;
  ServiceProperties props;
  props["string"] = std::string("A std::string");
  props["bool"] = false;
  const char* str = "A const char*";
  props["const char*"] = str;

  ServiceRegistration<ITestServiceA> reg1 = context->RegisterService<ITestServiceA>(&s1, props);
  ServiceReference<ITestServiceA> ref1 = context->GetServiceReference<ITestServiceA>();

  US_TEST_CONDITION_REQUIRED(context->GetServiceReferences<ITestServiceA>().size() == 1, "Testing service count")
  US_TEST_CONDITION_REQUIRED(any_cast<bool>(ref1.GetProperty("bool")) == false, "Testing bool property")

  // register second service with higher rank
  TestServiceA s2;
  ServiceProperties props2;
  props2[ServiceConstants::SERVICE_RANKING()] = 50;

  ServiceRegistration<ITestServiceA> reg2 = context->RegisterService<ITestServiceA>(&s2, props2);

  // Get the service with the highest rank, this should be s2.
  ServiceReference<ITestServiceA> ref2 = context->GetServiceReference<ITestServiceA>();
  TestServiceA* service = dynamic_cast<TestServiceA*>(context->GetService(ref2));
  US_TEST_CONDITION_REQUIRED(service == &s2, "Testing highest service rank")

  props["bool"] = true;
  // change the service ranking
  props[ServiceConstants::SERVICE_RANKING()] = 100;
  reg1.SetProperties(props);

  US_TEST_CONDITION_REQUIRED(context->GetServiceReferences<ITestServiceA>().size() == 2, "Testing service count")
  US_TEST_CONDITION_REQUIRED(any_cast<bool>(ref1.GetProperty("bool")) == true, "Testing bool property")
  US_TEST_CONDITION_REQUIRED(any_cast<int>(ref1.GetProperty(ServiceConstants::SERVICE_RANKING())) == 100, "Testing updated ranking")

  // Service with the highest ranking should now be s1
  service = dynamic_cast<TestServiceA*>(context->GetService<ITestServiceA>(ref1));
  US_TEST_CONDITION_REQUIRED(service == &s1, "Testing highest service rank")

  reg1.Unregister();
  US_TEST_CONDITION_REQUIRED(context->GetServiceReferences<ITestServiceA>("").size() == 1, "Testing service count")

  service = dynamic_cast<TestServiceA*>(context->GetService<ITestServiceA>(ref2));
  US_TEST_CONDITION_REQUIRED(service == &s2, "Testing highest service rank")

  reg2.Unregister();
  US_TEST_CONDITION_REQUIRED(context->GetServiceReferences<ITestServiceA>().empty(), "Testing service count")
}


int usServiceRegistryTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceRegistryTest");

  TestServiceInterfaceId();
  TestMultipleServiceRegistrations();
  TestServicePropertiesUpdate();

  US_TEST_END()
}
