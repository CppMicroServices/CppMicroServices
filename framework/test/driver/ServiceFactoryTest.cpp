/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/LDAPProp.h"
#include "cppmicroservices/ListenerToken.h"
#include "cppmicroservices/ServiceObjects.h"

#include "TestingConfig.h"
#include "TestingMacros.h"
#include "TestUtils.h"

#include <thread>

using namespace cppmicroservices;

namespace cppmicroservices {

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
  US_TEST_CONDITION_REQUIRED(registeredRefs.size() == 6, "Test that the # of registered services is six.");
  US_TEST_CONDITION(registeredRefs[0].GetProperty(Constants::SERVICE_SCOPE).ToString() == Constants::SCOPE_BUNDLE, "First service is bundle scope");
  US_TEST_CONDITION(registeredRefs[1].GetProperty(Constants::SERVICE_SCOPE).ToString() == Constants::SCOPE_BUNDLE, "Second service is bundle scope");
  US_TEST_CONDITION(registeredRefs[2].GetProperty(Constants::SERVICE_SCOPE).ToString() == Constants::SCOPE_BUNDLE, "Third service is bundle scope");
  US_TEST_CONDITION(registeredRefs[3].GetProperty(Constants::SERVICE_SCOPE).ToString() == Constants::SCOPE_BUNDLE, "Fourth service is bundle scope");
  US_TEST_CONDITION(registeredRefs[4].GetProperty(Constants::SERVICE_SCOPE).ToString() == Constants::SCOPE_BUNDLE, "Fifth service is bundle scope");
  US_TEST_CONDITION(registeredRefs[5].GetProperty(Constants::SERVICE_SCOPE).ToString() == Constants::SCOPE_PROTOTYPE, "Sixth service is prototype scope");

  // Check that a service reference exists
  const ServiceReferenceU sr1 = context.GetServiceReference("cppmicroservices::TestBundleH");
  US_TEST_CONDITION_REQUIRED(sr1, "A valid service reference was returned.");
  US_TEST_CONDITION(sr1.GetProperty(Constants::SERVICE_SCOPE).ToString() == Constants::SCOPE_BUNDLE, "service is bundle scope");

  InterfaceMapConstPtr service = context.GetService(sr1);
  US_TEST_CONDITION_REQUIRED(service->size() >= 1, "Returned at least 1 service object");
  InterfaceMap::const_iterator serviceIter = service->find("cppmicroservices::TestBundleH");
  US_TEST_CONDITION_REQUIRED(serviceIter != service->end(), "Find a service object implementing the 'cppmicroservices::TestBundleH' interface");
  US_TEST_CONDITION_REQUIRED(serviceIter->second != nullptr, "The service object is valid (i.e. not nullptr)");

  InterfaceMapConstPtr service2 = context.GetService(sr1);
  US_TEST_CONDITION(*(service.get()) == *(service2.get()), "Two service interface maps are equal");

  std::vector<ServiceReferenceU> usedRefs = context.GetBundle().GetServicesInUse();
  US_TEST_CONDITION_REQUIRED(usedRefs.size() == 1, "1 service in use");
  US_TEST_CONDITION(usedRefs[0] == sr1, "service references are equal");

  InterfaceMapConstPtr service3 = bundleH.GetBundleContext().GetService(sr1);
  US_TEST_CONDITION(service.get() != service3.get(), "Different service pointer");

  // release any service objects before stopping the bundle
  service.reset();
  service2.reset();
  service3.reset();

  bundleH.Stop();
}

void TestServiceFactoryBundleScopeErrorConditions()
{
  auto framework = FrameworkFactory().NewFramework();
  framework.Start();

  auto context = framework.GetBundleContext();
  
  auto bundle = testing::InstallLib(context, "TestBundleH");
  US_TEST_CONDITION_REQUIRED(bundle, "Test for existing bundle TestBundleH")

  auto bundleH = testing::GetBundle("TestBundleH", context);
  US_TEST_CONDITION_REQUIRED(bundleH, "Test for existing bundle TestBundleH")

  bundleH.Start();

  int eventCount(0);
  auto eventCountListener = [&eventCount](const FrameworkEvent&) { ++eventCount; };
  auto listenerToken = bundleH.GetBundleContext().AddFrameworkListener(eventCountListener);

 // Test that a service factory which returns a nullptr returns an invalid (nullptr) shared_ptr
  std::string returnsNullPtrFilter(LDAPProp("returns_nullptr") == std::string("true"));
  auto serviceRefs(context.GetServiceReferences("cppmicroservices::TestBundleH", returnsNullPtrFilter));
  US_TEST_CONDITION_REQUIRED(serviceRefs.size() == 1, "Number of service references returned is 1.");
  US_TEST_CONDITION_REQUIRED("1" == serviceRefs[0].GetProperty(std::string("returns_nullptr")).ToString(), "Test that 'returns_nullptr' service property is 'true'");
  US_TEST_CONDITION_REQUIRED(nullptr == context.GetService(serviceRefs[0]), "Test that the service object returned is a nullptr");
  US_TEST_CONDITION_REQUIRED(1 == eventCount, "Test that one FrameworkEvent was sent");

  bundleH.GetBundleContext().RemoveListener(std::move(listenerToken));
  
  eventCount = 0;
  listenerToken = bundleH.GetBundleContext().AddFrameworkListener(eventCountListener);

  // Test getting a service object using an interface which isn't implemented by the service factory
  std::string returnsWrongInterfaceFilter(LDAPProp("returns_wrong_interface") == std::string("true"));
  auto svcNoInterfaceRefs(context.GetServiceReferences("cppmicroservices::TestBundleH", returnsWrongInterfaceFilter));
  US_TEST_CONDITION_REQUIRED(svcNoInterfaceRefs.size() == 1, "Number of service references returned is 1.");
  US_TEST_CONDITION_REQUIRED("1" == svcNoInterfaceRefs[0].GetProperty(std::string("returns_wrong_interface")).ToString(), "Test that 'returns_wrong_interface' service property is 'true'");
  US_TEST_CONDITION_REQUIRED(nullptr == context.GetService(svcNoInterfaceRefs[0]), "Test that the service object returned is a nullptr");
  US_TEST_CONDITION_REQUIRED(1 == eventCount, "Test that one FrameworkEvent was sent");

  bundleH.GetBundleContext().RemoveListener(std::move(listenerToken));

  eventCount = 0;
  listenerToken = bundleH.GetBundleContext().AddFrameworkListener(eventCountListener);

  // Test getting a service object from a service factory which throws an exception
  std::string getServiceThrowsFilter(LDAPProp("getservice_exception") == std::string("true"));
  auto svcGetServiceThrowsRefs(context.GetServiceReferences("cppmicroservices::TestBundleH", getServiceThrowsFilter));
  US_TEST_CONDITION_REQUIRED(svcGetServiceThrowsRefs.size() == 1, "Number of service references returned is 1.");
  US_TEST_CONDITION_REQUIRED("1" == svcGetServiceThrowsRefs[0].GetProperty(std::string("getservice_exception")).ToString(), "Test that 'getservice_exception' service property is 'true'");
  US_TEST_CONDITION_REQUIRED(nullptr == context.GetService(svcGetServiceThrowsRefs[0]), "Test that the service object returned is a nullptr");
  US_TEST_CONDITION_REQUIRED(1 == eventCount, "Test that one FrameworkEvent was sent");

  bundleH.GetBundleContext().RemoveListener(std::move(listenerToken));

  eventCount = 0;
  listenerToken = bundleH.GetBundleContext().AddFrameworkListener(eventCountListener);

  std::string unGetServiceThrowsFilter(LDAPProp("ungetservice_exception") == std::string("true"));
  auto svcUngetServiceThrowsRefs(context.GetServiceReferences("cppmicroservices::TestBundleH", unGetServiceThrowsFilter));
  US_TEST_CONDITION_REQUIRED(svcUngetServiceThrowsRefs.size() == 1, "Number of service references returned is 1.");
  US_TEST_CONDITION_REQUIRED("1" == svcUngetServiceThrowsRefs[0].GetProperty(std::string("ungetservice_exception")).ToString(), "Test that 'ungetservice_exception' service property is 'true'");
  {
    auto sfUngetServiceThrowSvc = context.GetService(svcUngetServiceThrowsRefs[0]);
  } // When sfUngetServiceThrowSvc goes out of scope, ServiceFactory::UngetService should be called
  US_TEST_CONDITION_REQUIRED(1 == eventCount, "Test that one FrameworkEvent was sent");

  framework.Stop();
  framework.WaitForStop(std::chrono::milliseconds::zero());
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

#ifdef US_ENABLE_THREADING_SUPPORT

// test that concurrent calls to ServiceFactory::GetService and ServiceFactory::UngetService
// don't cause race conditions.
void TestConcurrentServiceFactory()
{
  auto framework = FrameworkFactory().NewFramework();
  framework.Start();

  auto bundle = testing::InstallLib(framework.GetBundleContext(), "TestBundleH");
  bundle.Start();

  std::vector<std::thread> worker_threads;
  for (size_t i = 0; i < 100; ++i)
  {
    worker_threads.push_back(std::thread([framework]()
        {
          auto frameworkCtx = framework.GetBundleContext();
          if (!frameworkCtx)
          {
            US_TEST_FAILED_MSG(<< "Failed to get Framework's bundle context. Terminating the thread...");
            return;
          }
          
          for (int i = 0; i < 100; ++i)
          {
            auto ref = frameworkCtx.GetServiceReference<cppmicroservices::TestBundleH2>();
            if (ref)
            {
              std::shared_ptr<cppmicroservices::TestBundleH2> svc = frameworkCtx.GetService(ref);
              if (!svc)
              {
                US_TEST_FAILED_MSG(<< "Failed to retrieve a valid service object");
              }
            }
          }
        }));
  }

  for (auto& t : worker_threads) t.join();
}
#endif

int ServiceFactoryTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceFactoryTest");

  FrameworkFactory factory;
  auto framework = factory.NewFramework();
  framework.Start();

  
  TestServiceFactoryPrototypeScope(framework.GetBundleContext());
  TestServiceFactoryBundleScope(framework.GetBundleContext());
  TestServiceFactoryBundleScopeErrorConditions();

#ifdef US_ENABLE_THREADING_SUPPORT
  TestConcurrentServiceFactory();
#endif

  US_TEST_END()
}
