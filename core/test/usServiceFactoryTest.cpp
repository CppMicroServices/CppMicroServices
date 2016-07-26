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
#include <usFramework.h>
#include <usGetBundleContext.h>
#include <usBundleContext.h>
#include <usBundle.h>
#include <usConstants.h>
#include <usServiceObjects.h>

#include "usTestUtils.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

using namespace us;

namespace us {

struct TestBundleH
{
  virtual ~TestBundleH() {}
};

struct TestBundleH2
{
  virtual ~TestBundleH2() {}
};

}


void TestServiceFactoryBundleScope(BundleContext context)
{

  // Install and start test bundle H, a service factory and test that the methods
  // in that interface works.

  auto bundle = testing::InstallLib(context, "TestBundleH");
  US_TEST_CONDITION_REQUIRED(bundle, "Test for existing bundle TestBundleH")

  auto bundleH = testing::GetBundle("TestBundleH", context);
  US_TEST_CONDITION_REQUIRED(bundleH, "Test for existing bundle TestBundleH")

  bundleH.Start();

  std::vector<ServiceReferenceU> registeredRefs = bundleH.GetRegisteredServices();
  US_TEST_CONDITION_REQUIRED(registeredRefs.size() == 2, "# of registered services")
  US_TEST_CONDITION(registeredRefs[0].GetProperty(Constants::SERVICE_SCOPE).ToString() == Constants::SCOPE_BUNDLE, "service scope")
  US_TEST_CONDITION(registeredRefs[1].GetProperty(Constants::SERVICE_SCOPE).ToString() == Constants::SCOPE_PROTOTYPE, "service scope")

  // Check that a service reference exist
  const ServiceReferenceU sr1 = context.GetServiceReference("us::TestBundleH");
  US_TEST_CONDITION_REQUIRED(sr1, "Service shall be present.")
  US_TEST_CONDITION(sr1.GetProperty(Constants::SERVICE_SCOPE).ToString() == Constants::SCOPE_BUNDLE, "service scope")

  InterfaceMapConstPtr service = context.GetService(sr1);
  US_TEST_CONDITION_REQUIRED(service->size() >= 1, "GetService()")
  InterfaceMap::const_iterator serviceIter = service->find("us::TestBundleH");
  US_TEST_CONDITION_REQUIRED(serviceIter != service->end(), "GetService()")
  US_TEST_CONDITION_REQUIRED(serviceIter->second != nullptr, "GetService()")

  InterfaceMapConstPtr service2 = context.GetService(sr1);
  US_TEST_CONDITION(*(service.get()) == *(service2.get()), "Same service interface maps")

  std::vector<ServiceReferenceU> usedRefs = context.GetBundle().GetServicesInUse();
  US_TEST_CONDITION_REQUIRED(usedRefs.size() == 1, "services in use")
  US_TEST_CONDITION(usedRefs[0] == sr1, "service ref in use")

  InterfaceMapConstPtr service3 = bundleH.GetBundleContext().GetService(sr1);
  US_TEST_CONDITION(service.get() != service3.get(), "Different service pointer")

  // release any service objects before stopping the bundle
  service.reset();
  service2.reset();
  service3.reset();

  bundleH.Stop();
}

void TestServiceFactoryPrototypeScope(BundleContext context)
{

  // Install and start test bundle H, a service factory and test that the methods
  // in that interface works.
  auto bundle = testing::InstallLib(context, "TestBundleH");
  US_TEST_CONDITION_REQUIRED(bundle, "Test for existing bundle TestBundleH")

  auto bundleH = testing::GetBundle("TestBundleH", context);
  US_TEST_CONDITION_REQUIRED(bundleH, "Test for existing bundle TestBundleH")

  bundleH.Start();

  // Check that a service reference exist
  const ServiceReference<TestBundleH2> sr1 = context.GetServiceReference<TestBundleH2>();
  US_TEST_CONDITION_REQUIRED(sr1, "Service shall be present.")
  US_TEST_CONDITION(sr1.GetProperty(Constants::SERVICE_SCOPE).ToString() == Constants::SCOPE_PROTOTYPE, "service scope")

  ServiceObjects<TestBundleH2> svcObjects = context.GetServiceObjects(sr1);
  auto prototypeServiceH2 = svcObjects.GetService();

  const ServiceReferenceU sr1void = context.GetServiceReference(us_service_interface_iid<TestBundleH2>());
  ServiceObjects<void> svcObjectsVoid = context.GetServiceObjects(sr1void);
  InterfaceMapConstPtr prototypeServiceH2Void = svcObjectsVoid.GetService();
  US_TEST_CONDITION_REQUIRED(prototypeServiceH2Void->find(us_service_interface_iid<TestBundleH2>()) != prototypeServiceH2Void->end(),
                             "ServiceObjects<void>::GetService()")

#ifdef US_BUILD_SHARED_LIBS
  // There should be only one service in use
  US_TEST_CONDITION_REQUIRED(context.GetBundle().GetServicesInUse().size() == 1, "services in use")
#endif

  auto bundleScopeService = context.GetService(sr1);
  US_TEST_CONDITION_REQUIRED(bundleScopeService && bundleScopeService != prototypeServiceH2, "GetService()")

  US_TEST_CONDITION_REQUIRED(prototypeServiceH2 != prototypeServiceH2Void->find(us_service_interface_iid<TestBundleH2>())->second,
                             "GetService()")

  auto bundleScopeService2 = context.GetService(sr1);
  US_TEST_CONDITION(bundleScopeService == bundleScopeService2, "Same service pointer")

#ifdef US_BUILD_SHARED_LIBS
  std::vector<ServiceReferenceU> usedRefs = context.GetBundle().GetServicesInUse();
  US_TEST_CONDITION_REQUIRED(usedRefs.size() == 1, "services in use")
  US_TEST_CONDITION(usedRefs[0] == sr1, "service ref in use")
#endif

  std::string filter = "(" + Constants::SERVICE_ID + "=" + sr1.GetProperty(Constants::SERVICE_ID).ToString() + ")";
  const ServiceReference<TestBundleH> sr2 = context.GetServiceReferences<TestBundleH>(filter).front();
  US_TEST_CONDITION_REQUIRED(sr2, "Service shall be present.")
  US_TEST_CONDITION(sr2.GetProperty(Constants::SERVICE_SCOPE).ToString() == Constants::SCOPE_PROTOTYPE, "service scope")
  US_TEST_CONDITION(any_cast<long>(sr2.GetProperty(Constants::SERVICE_ID)) == any_cast<long>(sr1.GetProperty(Constants::SERVICE_ID)), "same service id")

#ifdef US_BUILD_SHARED_LIBS
  // There should still be only one service in use
  usedRefs = context.GetBundle().GetServicesInUse();
  US_TEST_CONDITION_REQUIRED(usedRefs.size() == 1, "services in use")
#endif

  ServiceObjects<TestBundleH2> svcObjects2 = std::move(svcObjects);
  ServiceObjects<TestBundleH2> svcObjects3 = context.GetServiceObjects(sr1);

  prototypeServiceH2 = svcObjects2.GetService();
  auto prototypeServiceH2_2 = svcObjects3.GetService();

  US_TEST_CONDITION_REQUIRED(prototypeServiceH2_2 && prototypeServiceH2_2 != prototypeServiceH2, "prototype service")

  bundleH.Stop();
}

int usServiceFactoryTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceFactoryTest");

  FrameworkFactory factory;
  auto framework = factory.NewFramework();
  framework.Start();

  
  TestServiceFactoryPrototypeScope(framework.GetBundleContext());
  TestServiceFactoryBundleScope(framework.GetBundleContext());

  US_TEST_END()
}
