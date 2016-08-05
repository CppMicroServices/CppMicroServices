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

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/LDAPFilter.h"
#include "cppmicroservices/ServiceInterface.h"

#include "TestingMacros.h"

#include <stdexcept>

using namespace cppmicroservices;

struct ITestServiceA
{
  virtual ~ITestServiceA() {}
};

struct ITestServiceB
{
  virtual ~ITestServiceB() {};
};
// Test the optional macro to provide custom name for a service interface class
US_DECLARE_SERVICE_INTERFACE(ITestServiceB, "com.mycompany.ITestService/1.0");

void TestServiceInterfaceId()
{
  US_TEST_CONDITION(us_service_interface_iid<int>() == "int", "Service interface id int")
  US_TEST_CONDITION(us_service_interface_iid<ITestServiceA>() == "ITestServiceA", "Service interface id ITestServiceA")
  US_TEST_CONDITION(us_service_interface_iid<ITestServiceB>() == "com.mycompany.ITestService/1.0", "Service interface id com.mycompany.ITestService/1.0")
}

void TestMultipleServiceRegistrations(BundleContext context)
{
  struct TestServiceA : public ITestServiceA
  {
  };

  auto s1 = std::make_shared<TestServiceA>();
  auto s2 = std::make_shared<TestServiceA>();

  ServiceRegistration<ITestServiceA> reg1 = context.RegisterService<ITestServiceA>(s1);
  ServiceRegistration<ITestServiceA> reg2 = context.RegisterService<ITestServiceA>(s2);

  std::vector<ServiceReference<ITestServiceA> > refs = context.GetServiceReferences<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(refs.size() == 2, "Testing for two registered ITestServiceA services")

  reg2.Unregister();
  refs = context.GetServiceReferences<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(refs.size() == 1, "Testing for one registered ITestServiceA services")

  reg1.Unregister();
  refs = context.GetServiceReferences<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(refs.empty(), "Testing for no ITestServiceA services")

  ServiceReference<ITestServiceA> ref = context.GetServiceReference<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(!ref, "Testing for invalid service reference")
}

void TestServicePropertiesUpdate(BundleContext context)
{
  struct TestServiceA : public ITestServiceA
  {
  };

  auto s1 = std::make_shared<TestServiceA>();
  ServiceProperties props;
  props["string"] = std::string("A std::string");
  props["bool"] = false;
  const char* str = "A const char*";
  props["const char*"] = str;

  ServiceRegistration<ITestServiceA> reg1 = context.RegisterService<ITestServiceA>(s1, props);
  ServiceReference<ITestServiceA> ref1 = context.GetServiceReference<ITestServiceA>();

  US_TEST_CONDITION_REQUIRED(context.GetServiceReferences<ITestServiceA>().size() == 1, "Testing service count")
  US_TEST_CONDITION_REQUIRED(any_cast<bool>(ref1.GetProperty("bool")) == false, "Testing bool property")

  // register second service with higher rank
  auto s2 = std::make_shared<TestServiceA>();
  ServiceProperties props2;
  props2[Constants::SERVICE_RANKING] = 50;

  ServiceRegistration<ITestServiceA> reg2 = context.RegisterService<ITestServiceA>(s2, props2);

  // Get the service with the highest rank, this should be s2.
  ServiceReference<ITestServiceA> ref2 = context.GetServiceReference<ITestServiceA>();
  auto service = std::dynamic_pointer_cast<TestServiceA>(context.GetService(ref2));
  US_TEST_CONDITION_REQUIRED(service == s2, "Testing highest service rank")

  props["bool"] = true;
  // change the service ranking
  props[Constants::SERVICE_RANKING] = 100;
  reg1.SetProperties(props);

  US_TEST_CONDITION_REQUIRED(context.GetServiceReferences<ITestServiceA>().size() == 2, "Testing service count")
  US_TEST_CONDITION_REQUIRED(any_cast<bool>(ref1.GetProperty("bool")) == true, "Testing bool property")
  US_TEST_CONDITION_REQUIRED(any_cast<int>(ref1.GetProperty(Constants::SERVICE_RANKING)) == 100, "Testing updated ranking")

  // Service with the highest ranking should now be s1
  service = std::dynamic_pointer_cast<TestServiceA>(context.GetService<ITestServiceA>(ref1));
  US_TEST_CONDITION_REQUIRED(service == s1, "Testing highest service rank")

  reg1.Unregister();
  US_TEST_CONDITION_REQUIRED(context.GetServiceReferences<ITestServiceA>("").size() == 1, "Testing service count")

  service = std::dynamic_pointer_cast<TestServiceA>(context.GetService<ITestServiceA>(ref2));
  US_TEST_CONDITION_REQUIRED(service == s2, "Testing highest service rank")

  reg2.Unregister();
  US_TEST_CONDITION_REQUIRED(context.GetServiceReferences<ITestServiceA>().empty(), "Testing service count")
}


int ServiceRegistryTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceRegistryTest");

  FrameworkFactory factory;
  auto framework = factory.NewFramework();
  framework.Start();

  auto context = framework.GetBundleContext();

  TestServiceInterfaceId();
  TestMultipleServiceRegistrations(context);
  TestServicePropertiesUpdate(context);

  US_TEST_END()
}
