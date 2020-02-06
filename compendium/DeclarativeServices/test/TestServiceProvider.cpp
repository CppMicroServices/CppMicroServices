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

#include "gtest/gtest.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/ServiceObjects.h"
#include "TestFixture.hpp"
#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"

#ifdef US_PLATFORM_POSIX
#include <dlfcn.h>
#endif

namespace test {

namespace sc = cppmicroservices::service::component;

/**
 * Verify that a service is published by the bundle with component description
 * in its metadata
 */
TEST_F(tServiceComponent, testService) //DS_TOI_4
{
  auto testBundle = StartTestBundle("TestBundleDSTOI1");
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent");
  auto sRef = framework.GetBundleContext().GetServiceReference<test::Interface1>();
  EXPECT_TRUE(static_cast<bool>(sRef)) << "valid service reference must be available after the bundle is started";
  auto service = framework.GetBundleContext().GetService<test::Interface1>(sRef);
  EXPECT_NE(service, nullptr) << "a valid service instance must be available";
  EXPECT_EQ(service->Description(), testBundle.GetSymbolicName()) << "service instance must return the symbolic name of the bundle that published the service";
}
  
/**
 * Verify that a service published through DS
 * always has the properties COMP_ID & COMP_NAME
 */
TEST_F(tServiceComponent, testDefaultServiceProperties) //DS_TOI_11
{
  std::string testBundleName("TestBundleDSTOI1");
  auto testBundle = StartTestBundle(testBundleName);
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent");
  EXPECT_EQ(compDescDTO.properties.size(), 0ul) << "componet description must not have any properties specified";
  EXPECT_EQ(dsRuntimeService->IsComponentEnabled(compDescDTO), true) << "current state reported by the runtime service must match the initial state in component description";
  auto sRef = framework.GetBundleContext().GetServiceReference<test::Interface1>();
  EXPECT_TRUE(static_cast<bool>(sRef)) << "valid service reference must be available after the bundle is started";
  auto compIdAny = sRef.GetProperty(sc::ComponentConstants::COMPONENT_ID);
  EXPECT_FALSE(compIdAny.Empty()) << "COMP_ID property must exist for a service published by DS";
  auto compNameAny = sRef.GetProperty(sc::ComponentConstants::COMPONENT_NAME);
  EXPECT_FALSE(compNameAny.Empty()) << "COMP_NAME property must exist for a service published by DS";
  EXPECT_EQ(compNameAny.ToStringNoExcept(), compDescDTO.name) << "value of COMP_NAME service property must match the value from the component description";
}
  
/**
 * verify that any properties specified in the component description file are
 * made available as service properties when it is published to the service registry.
 */
TEST_F(tServiceComponent, testCustomServiceProperties) //DS_TOI_12
{
  auto testBundle = StartTestBundle("TestBundleDSTOI12");
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent12");
  EXPECT_EQ(compDescDTO.properties.size(), 3ul) << "component description must have properties specified";
  EXPECT_EQ(dsRuntimeService->IsComponentEnabled(compDescDTO), true) << "current state reported by the runtime service must match the initial state in component description";
  auto sRef = framework.GetBundleContext().GetServiceReference<test::Interface1>();
  EXPECT_TRUE(static_cast<bool>(sRef)) << "valid service reference must be available after the bundle is started";
  auto compIdAny = sRef.GetProperty(sc::ComponentConstants::COMPONENT_ID);
  EXPECT_FALSE(compIdAny.Empty()) << "COMP_ID property must exist for a service published by DS";
  auto compNameAny = sRef.GetProperty(sc::ComponentConstants::COMPONENT_NAME);
  EXPECT_FALSE(compNameAny.Empty()) << "COMP_NAME property must exist for a service published by DS";
  EXPECT_EQ(compNameAny.ToStringNoExcept(), compDescDTO.name) << "value of COMP_NAME service property must match the value from the component description";
    
  // verify that all properties specified in the component description are in the service properties
  for(auto prop : compDescDTO.properties)
  {
    auto propVal = sRef.GetProperty(prop.first);
    EXPECT_FALSE(propVal.Empty()) << "Property from component description must be present in the service properties";
    EXPECT_EQ(propVal.Type(), prop.second.Type());
    EXPECT_EQ(propVal.ToStringNoExcept(), prop.second.ToStringNoExcept()) << "Value of the service property must match the value specified in the component description";
  }

  // verify custom objects in service properties
  EXPECT_NO_THROW({
      auto propObject = cppmicroservices::any_cast<cppmicroservices::AnyMap>(sRef.GetProperty("DummyAnyMap"));
      EXPECT_EQ(propObject.AtCompoundKey("NestedMap.a").ToString(), "b");
      EXPECT_EQ(cppmicroservices::any_cast<int>(propObject.AtCompoundKey("NestedVector.1")), 2);
    });
}
  
/**
 * Verify a service specified with scope as SINGLETON in component description
 * is published with the correct scope and all calls to GetService return the
 * singleton instance
 */
TEST_F(tServiceComponent, testSingletonScope) //DS_TOI_13
{
  std::string testBundleName("TestBundleDSTOI1");
  auto testBundle = StartTestBundle(testBundleName);
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent");
  EXPECT_EQ(compDescDTO.scope, cppmicroservices::Constants::SCOPE_SINGLETON);
  auto ctxt = framework.GetBundleContext();
  auto sRef = ctxt.GetServiceReference<test::Interface1>();
  EXPECT_TRUE(static_cast<bool>(sRef));
  auto serviceScope = sRef.GetProperty(cppmicroservices::Constants::SERVICE_SCOPE);
  EXPECT_EQ(compDescDTO.scope, serviceScope.ToStringNoExcept());
  auto serv = ctxt.GetService<test::Interface1>(sRef);
  EXPECT_NE(serv, nullptr);
  auto sRef1 = ctxt.GetServiceReference<test::Interface1>();
  auto serv1 = ctxt.GetService<test::Interface1>(sRef1);
  EXPECT_NE(serv1, nullptr);
  EXPECT_EQ(serv.get(), serv1.get()) << "same service instance must be returned from all calls to GetService";
  auto sRef2 = testBundle.GetBundleContext().GetServiceReference<test::Interface1>();
  auto serv2 = testBundle.GetBundleContext().GetService<test::Interface1>(sRef2);
  EXPECT_EQ(serv.get(), serv2.get()) << "same service instance must be returned from all calls to GetService";
}
  
/**
 * Verify a service specified with scope as BUNDLE in component description
 * is published with the correct scope and calls to GetService from the same
 * bundle context return the same instance but calls from different contexts
 * result in different instances of the service
 */
TEST_F(tServiceComponent, testBundleScope) //DS_TOI_14
{
  auto testBundle = StartTestBundle("TestBundleDSTOI14");
  auto helperBundle = StartTestBundle("TestBundleDSTOI1");
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent14");
  EXPECT_EQ(compDescDTO.scope, cppmicroservices::Constants::SCOPE_BUNDLE);
  auto ctxt = framework.GetBundleContext();
  cppmicroservices::ServiceReference<test::Interface1> sRef = ctxt.GetServiceReference<test::Interface1>();
  EXPECT_TRUE(static_cast<bool>(sRef));
  auto serviceScope = sRef.GetProperty(cppmicroservices::Constants::SERVICE_SCOPE);
  EXPECT_EQ(compDescDTO.scope, serviceScope.ToStringNoExcept());
    
  cppmicroservices::ServiceObjects<test::Interface1> serviceObjects = ctxt.GetServiceObjects(sRef);
  size_t callCount = 10;
  size_t i = 0;
  std::set<std::shared_ptr<test::Interface1>> instanceSet;
  for(;i < callCount; i++)
  {
    instanceSet.emplace(serviceObjects.GetService());
  }
    
  auto helperBundleCtxt = helperBundle.GetBundleContext();
  auto sRef1 = helperBundleCtxt.GetServiceReference<test::Interface1>();
  cppmicroservices::ServiceObjects<test::Interface1> serviceObjects1 = helperBundleCtxt.GetServiceObjects(sRef1);
  callCount += 5;
  for(;i < callCount; i++)
  {
    instanceSet.emplace(serviceObjects1.GetService());
  }
  EXPECT_TRUE(std::none_of(instanceSet.begin(), instanceSet.end(), [](const std::shared_ptr<test::Interface1>& service) { return service == nullptr; }));
  EXPECT_EQ(instanceSet.size(), 2) << "number of service instances returned must be equal to the number of distinct contexts used to call GetService";
  instanceSet.clear();
}
  
/**
 * Verify a service specified with scope as PROTOTYPE in component description
 * is published with the correct scope and calls to GetService always return a
 * unique instance
 */
TEST_F(tServiceComponent, testPrototypeScope) //DS_TOI_15
{
  auto testBundle = StartTestBundle("TestBundleDSTOI15");
  auto helperBundle = StartTestBundle("TestBundleDSTOI1");
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent15");
  EXPECT_EQ(compDescDTO.scope, cppmicroservices::Constants::SCOPE_PROTOTYPE);
  auto ctxt = framework.GetBundleContext();
  cppmicroservices::ServiceReference<test::Interface1> sRef = ctxt.GetServiceReference<test::Interface1>();
  EXPECT_TRUE(static_cast<bool>(sRef));
  auto serviceScope = sRef.GetProperty(cppmicroservices::Constants::SERVICE_SCOPE);
  EXPECT_EQ(compDescDTO.scope, serviceScope.ToStringNoExcept());
    
  cppmicroservices::ServiceObjects<test::Interface1> serviceObjects = ctxt.GetServiceObjects(sRef);
  size_t expectedInstanceCount = 10;
  size_t i = 0;
  std::set<std::shared_ptr<test::Interface1>> instanceSet;
  for(;i < expectedInstanceCount; i++)
  {
    instanceSet.emplace(serviceObjects.GetService());
  }
    
  auto helperBundleCtxt = helperBundle.GetBundleContext();
  auto sRef1 = helperBundleCtxt.GetServiceReference<test::Interface1>();
  cppmicroservices::ServiceObjects<test::Interface1> serviceObjects1 = helperBundleCtxt.GetServiceObjects(sRef1);
  expectedInstanceCount += 5;
  for(;i < expectedInstanceCount; i++)
  {
    instanceSet.emplace(serviceObjects1.GetService());
  }
  EXPECT_TRUE(std::none_of(instanceSet.begin(), instanceSet.end(), [](const std::shared_ptr<test::Interface1>& service) { return service == nullptr; }));
  EXPECT_EQ(instanceSet.size(), expectedInstanceCount) << "service instances returned from calls to GetService must be unique";
  instanceSet.clear();
}
  
/**
 * Verify that a service with multiple interfaces in component description is
 * registered with all the interfaces specified
 */
TEST_F(tServiceComponent, testMultipleInterfaces) //DS_TOI_16
{
  auto testBundle = StartTestBundle("TestBundleDSTOI16");
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent16");
  EXPECT_EQ(compDescDTO.serviceInterfaces.size(), 2ul);
  auto ctxt = framework.GetBundleContext();
  auto sRef = ctxt.GetServiceReference<test::Interface1>();
  EXPECT_TRUE(static_cast<bool>(sRef));
  auto sRef2 = ctxt.GetServiceReference<test::Interface2>();
  EXPECT_TRUE(static_cast<bool>(sRef2));
  auto serv1 = ctxt.GetService<test::Interface1>(sRef);
  EXPECT_NE(serv1, nullptr);
  auto serv2 = ctxt.GetService<test::Interface1>(sRef);
  EXPECT_NE(serv2, nullptr);
}

}
