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

#include <random>
#include <tuple>
#include <typeindex>
#include <typeinfo>

#include "../src/manager/ReferenceManagerImpl.hpp"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp"

#include "ConcurrencyTestUtil.hpp"
#include "Mocks.hpp"
#include "TestInterfaces/Interfaces.hpp"
#include "TestUtils.hpp"

namespace scr = cppmicroservices::service::component::runtime;

namespace test {
class InterfaceImpl
  : public Interface1
  , public Interface3
  , public Interface4
  , public Interface5
  , public Interface6
{
public:
  InterfaceImpl(std::string str)
    : str_(std::move(str))
  {}
  virtual ~InterfaceImpl() = default;
  virtual std::string Description() { return str_; }

private:
  std::string str_;
};
}

namespace cppmicroservices {
namespace scrimpl {

struct Policy
{
  const char* policy;
  const char* policyOption;
  std::type_index policyType;
};

class BindingPolicyTest : public ::testing::TestWithParam<Policy>
{
protected:
  BindingPolicyTest()
    : framework(cppmicroservices::FrameworkFactory().NewFramework())
  {}

  virtual ~BindingPolicyTest() = default;

  virtual void SetUp() { framework.Start(); }

  virtual void TearDown()
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Framework& GetFramework() { return framework; }

private:
  cppmicroservices::Framework framework;
};

struct DynamicRefPolicy
{
  const char* verificationMessage;
  const char* implClassName;
  InterfaceMapConstPtr interfaceMap;
};

class DynamicRefPolicyTest : public ::testing::TestWithParam<DynamicRefPolicy>
{
protected:
  DynamicRefPolicyTest()
    : framework(cppmicroservices::FrameworkFactory().NewFramework())
  {}

  virtual ~DynamicRefPolicyTest() = default;

  virtual void SetUp() { framework.Start(); }

  virtual void TearDown()
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Framework& GetFramework() { return framework; }

private:
  cppmicroservices::Framework framework;
};

// utility method for creating different types of reference metadata objects used in testing
metadata::ReferenceMetadata CreateFakeReferenceMetadata(
  const std::string& policy,
  const std::string& policyOption)
{
  metadata::ReferenceMetadata fakeMetadata{};
  fakeMetadata.name = "ref";
  fakeMetadata.interfaceName = us_service_interface_iid<dummy::Reference1>();
  fakeMetadata.policy = policy;
  fakeMetadata.policyOption = policyOption;
  fakeMetadata.cardinality = "0..1";
  fakeMetadata.minCardinality = 0;
  fakeMetadata.maxCardinality = 1;
  return fakeMetadata;
}

using namespace cppmicroservices::scrimpl;

INSTANTIATE_TEST_SUITE_P(
  BindingPolicies,
  BindingPolicyTest,
  testing::Values(
    Policy{ "static",
               "greedy",
               typeid(ReferenceManagerBaseImpl::BindingPolicyStaticGreedy) },
    Policy{ "static",
               "reluctant",
               typeid(ReferenceManagerBaseImpl::BindingPolicyStaticReluctant) },
    Policy{ "dynamic",
               "greedy",
               typeid(ReferenceManagerBaseImpl::BindingPolicyDynamicGreedy) },
    Policy{
      "dynamic",
      "reluctant",
      typeid(ReferenceManagerBaseImpl::BindingPolicyDynamicReluctant) }));

TEST_P(BindingPolicyTest, TestPolicyCreation)
{
  auto bc = GetFramework().GetBundleContext();
  auto const& param = GetParam();

  auto fakeMetadata =
    CreateFakeReferenceMetadata(param.policy, param.policyOption);
  auto fakeLogger = std::make_shared<FakeLogger>();
  auto mgr = std::make_shared<MockReferenceManagerBaseImpl>(
    fakeMetadata, bc, fakeLogger, "foo");

  auto bindingPolicy = ReferenceManagerBaseImpl::CreateBindingPolicy(
    *mgr, fakeMetadata.policy, fakeMetadata.policyOption);
  EXPECT_TRUE(bindingPolicy);
  auto* bindingPolicyData = bindingPolicy.get();

  EXPECT_EQ(param.policyType, typeid(*bindingPolicyData));
}

TEST_P(BindingPolicyTest, InvalidServiceReference)
{
  auto bc = GetFramework().GetBundleContext();
  auto const& param = GetParam();
  auto fakeMetadata =
    CreateFakeReferenceMetadata(param.policy, param.policyOption);
  auto fakeLogger = std::make_shared<FakeLogger>();
  auto mgr = std::make_shared<MockReferenceManagerBaseImpl>(
    fakeMetadata, bc, fakeLogger, "foo");

  auto bindingPolicy = ReferenceManagerBaseImpl::CreateBindingPolicy(
    *mgr, fakeMetadata.policy, fakeMetadata.policyOption);

  EXPECT_THROW(bindingPolicy->ServiceAdded(ServiceReferenceU()),
               std::invalid_argument);
}

INSTANTIATE_TEST_SUITE_P(
  DynamicReferencePolicies,
  DynamicRefPolicyTest,
  testing::Values(
    DynamicRefPolicy{
      "ServiceComponentDynamicReluctantMandatoryUnary depends on Interface3",
      "sample::ServiceComponentDynamicReluctantMandatoryUnary",
      MakeInterfaceMap<test::Interface3>(std::make_shared<test::InterfaceImpl>("Interface3")) },
    DynamicRefPolicy{
      "ServiceComponentDynamicGreedyMandatoryUnary depends on Interface4",
      "sample::ServiceComponentDynamicGreedyMandatoryUnary",
      MakeInterfaceMap<test::Interface4>(std::make_shared<test::InterfaceImpl>("Interface4")) },
    DynamicRefPolicy{
      "ServiceComponentDynamicReluctantOptionalUnary depends on Interface5",
      "sample::ServiceComponentDynamicReluctantOptionalUnary",
      MakeInterfaceMap<test::Interface5>(std::make_shared<test::InterfaceImpl>("Interface5")) },
    DynamicRefPolicy{
      "ServiceComponentDynamicGreedyOptionalUnary depends on Interface6",
      "sample::ServiceComponentDynamicGreedyOptionalUnary",
      MakeInterfaceMap<test::Interface6>(std::make_shared<test::InterfaceImpl>("Interface6")) }));

// test binding a service under the following reference policy, reference policy options and cardinality
// Cardinality: 0..1, 1..1
// reference policy: dynamic
// reference policy options: reluctant, greedy
TEST_P(DynamicRefPolicyTest, TestBindingWithDynamicPolicyOptions)
{
  auto bc = GetFramework().GetBundleContext();
  test::InstallAndStartDS(bc);

  auto const& param = GetParam();

  auto testBundle = test::InstallAndStartBundle(bc, "TBDynamicRefPolicy");
  EXPECT_FALSE(bc.GetServiceReference<test::Interface2>())
    << "Service must not be available before it's dependency";
  auto dsRef = bc.GetServiceReference<scr::ServiceComponentRuntime>();
  EXPECT_TRUE(dsRef);
  auto dsRuntimeService = bc.GetService<scr::ServiceComponentRuntime>(dsRef);
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(
    testBundle, param.implClassName);
  auto compConfigDTOs =
    dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigDTOs.size(), 1ul);
  EXPECT_EQ(compConfigDTOs.at(0).state,
            scr::dto::ComponentState::UNSATISFIED_REFERENCE);

  // register the dependent service to trigger the bind
  auto depSvcReg = bc.RegisterService(param.interfaceMap);
  ASSERT_TRUE(depSvcReg);

  auto result = test::RepeatTaskUntilOrTimeout(
    [&compConfigDTOs, &dsRuntimeService, &compDescDTO]() {
      compConfigDTOs =
        dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
    },
    [&compConfigDTOs]() -> bool {
      return compConfigDTOs.at(0).state == scr::dto::ComponentState::ACTIVE;
    });

  ASSERT_TRUE(result) << "Timed out waiting for state to change to ACTIVE "
                         "after the dependency became available";
  auto svcRef = bc.GetServiceReference<test::Interface2>();
  ASSERT_TRUE(svcRef);
  auto svc = bc.GetService<test::Interface2>(svcRef);
  ASSERT_TRUE(svc);
  EXPECT_NO_THROW(svc->ExtendedDescription());
  EXPECT_STREQ(
    param.verificationMessage,
    svc->ExtendedDescription().c_str())
    << "String value returned was not expected. Was the correct service "
       "dependency bound?";

  depSvcReg.Unregister();
  EXPECT_FALSE(bc.GetServiceReference<test::Interface2>())
    << "Service should now NOT be available";
  EXPECT_THROW(svc->ExtendedDescription(), std::runtime_error);
  testBundle.Stop();
}

// test error handling and logging when the bind and unbind methods throw an exception
TEST_F(BindingPolicyTest, TestDynamicBindUnBindExceptionHandling)
{
  auto bc = GetFramework().GetBundleContext();
  test::InstallAndStartDS(bc);

  auto testBundle = test::InstallAndStartBundle(bc, "TestBundleDSTOI22");
  EXPECT_FALSE(bc.GetServiceReference<test::Interface2>())
    << "Service must not be available before it's dependency";
  auto dsRef = bc.GetServiceReference<scr::ServiceComponentRuntime>();
  EXPECT_TRUE(dsRef);
  auto dsRuntimeService = bc.GetService<scr::ServiceComponentRuntime>(dsRef);
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(
    testBundle, "sample::ServiceComponent22");
  auto compConfigDTOs =
    dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigDTOs.size(), 1ul);
  EXPECT_EQ(compConfigDTOs.at(0).state,
            scr::dto::ComponentState::UNSATISFIED_REFERENCE);

  auto depSvcReg = bc.RegisterService<test::Interface1>(
    std::make_shared<test::InterfaceImpl>("Interface1"));
  ASSERT_TRUE(depSvcReg);
  auto result = test::RepeatTaskUntilOrTimeout(
    [&compConfigDTOs, &dsRuntimeService, &compDescDTO]() {
      compConfigDTOs =
        dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
    },
    [&compConfigDTOs]() -> bool {
      return compConfigDTOs.at(0).state == scr::dto::ComponentState::SATISFIED;
    });

  ASSERT_TRUE(result)
    << "Timed out waiting for state to change to SATISFIED "
       "after the dependency became available";

  // TODO: what is the correct behavior as it relates to service references
  // and service objects returned to the service consumer if the bind/unbind method throws?
  // currently the service reference is valid and the service object is nullptr
  auto svcRef = bc.GetServiceReference<test::Interface2>();
  ASSERT_TRUE(svcRef);
  auto svc = bc.GetService<test::Interface2>(svcRef);
  ASSERT_TRUE(svc);
  ASSERT_THROW(svc->ExtendedDescription(), std::runtime_error);

  depSvcReg.Unregister();
  EXPECT_FALSE(bc.GetServiceReference<test::Interface2>())
    << "Service should now NOT be available";

  // TODO: test when unbind throws


  testBundle.Stop();
}

// test that:
//  a new higher ranked service causes a re-bind
//  a new lower ranked service does not cause a re-bind 
TEST_F(BindingPolicyTest, TestDynamicGreedyReBind)
{
  auto bc = GetFramework().GetBundleContext();
  test::InstallAndStartDS(bc);

  auto testBundle = test::InstallAndStartBundle(bc, "TBDynamicRefPolicy");
  EXPECT_FALSE(bc.GetServiceReference<test::Interface2>())
    << "Service must not be available before it's dependency";
  auto dsRef = bc.GetServiceReference<scr::ServiceComponentRuntime>();
  EXPECT_TRUE(dsRef);
  auto dsRuntimeService = bc.GetService<scr::ServiceComponentRuntime>(dsRef);
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(
    testBundle, "sample::ServiceComponentDynamicGreedyMandatoryUnary");
  auto compConfigDTOs =
    dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigDTOs.size(), 1ul);
  EXPECT_EQ(compConfigDTOs.at(0).state,
            scr::dto::ComponentState::UNSATISFIED_REFERENCE);

  // register the dependent service to trigger the bind
  auto depSvcReg = bc.RegisterService<test::Interface4>(std::make_shared<test::InterfaceImpl>("Interface4"));
  ASSERT_TRUE(depSvcReg);

  auto result = test::RepeatTaskUntilOrTimeout(
    [&compConfigDTOs, &dsRuntimeService, &compDescDTO]() {
      compConfigDTOs =
        dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
    },
    [&compConfigDTOs]() -> bool {
      return compConfigDTOs.at(0).state == scr::dto::ComponentState::ACTIVE;
    });

  ASSERT_TRUE(result) << "Timed out waiting for state to change to ACTIVE "
                         "after the dependency became available";
  auto svcRef = bc.GetServiceReference<test::Interface2>();
  ASSERT_TRUE(svcRef);
  auto svc = bc.GetService<test::Interface2>(svcRef);
  ASSERT_TRUE(svc);
  EXPECT_NO_THROW(svc->ExtendedDescription()); 
  EXPECT_STREQ("ServiceComponentDynamicGreedyMandatoryUnary depends on Interface4", svc->ExtendedDescription().c_str())
    << "String value returned was not expected. Was the correct service "
       "dependency bound?";

  // registering a new service with a higher rank which should cause a re-binding and use of the new service
  ASSERT_TRUE(bc.RegisterService<test::Interface4>(
    std::make_shared<test::InterfaceImpl>("higher ranked Interface4"),
    { { Constants::SERVICE_RANKING, Any(10000) } }));
  EXPECT_NO_THROW(svc->ExtendedDescription());
  EXPECT_STREQ(
    "ServiceComponentDynamicGreedyMandatoryUnary depends on higher ranked Interface4",
    svc->ExtendedDescription().c_str())
    << "String value returned was not expected. Was the correct service "
       "dependency bound?";

  // registering a new service with a lower rank should NOT cause re-binding and use of the new service
  ASSERT_TRUE(bc.RegisterService<test::Interface4>(
    std::make_shared<test::InterfaceImpl>("lower ranked Interface4"),
    { { Constants::SERVICE_RANKING, Any(1) } }));
  EXPECT_NO_THROW(svc->ExtendedDescription());
  EXPECT_STREQ(
    "ServiceComponentDynamicGreedyMandatoryUnary depends on Interface4",
    svc->ExtendedDescription().c_str())
    << "String value returned was not expected. Was the correct service "
       "dependency bound?";

  depSvcReg.Unregister();
  EXPECT_FALSE(bc.GetServiceReference<test::Interface2>())
    << "Service should now NOT be available";
  EXPECT_THROW(svc->ExtendedDescription(), std::runtime_error);
  testBundle.Stop();
}

// test that binding happens only once for dynamic reluctant reference policy
TEST_F(BindingPolicyTest, TestDynamicReluctantReBind)
{
  auto bc = GetFramework().GetBundleContext();
  test::InstallAndStartDS(bc);

  auto testBundle = test::InstallAndStartBundle(bc, "TBDynamicRefPolicy");
  EXPECT_FALSE(bc.GetServiceReference<test::Interface2>())
    << "Service must not be available before it's dependency";
  auto dsRef = bc.GetServiceReference<scr::ServiceComponentRuntime>();
  EXPECT_TRUE(dsRef);
  auto dsRuntimeService = bc.GetService<scr::ServiceComponentRuntime>(dsRef);
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(
    testBundle, "sample::ServiceComponentDynamicReluctantMandatoryUnary");
  auto compConfigDTOs =
    dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigDTOs.size(), 1ul);
  EXPECT_EQ(compConfigDTOs.at(0).state,
            scr::dto::ComponentState::UNSATISFIED_REFERENCE);

  // register the dependent service to trigger the bind
  auto depSvcReg = bc.RegisterService<test::Interface3>(
    std::make_shared<test::InterfaceImpl>("Interface3"));
  ASSERT_TRUE(depSvcReg);

  auto result = test::RepeatTaskUntilOrTimeout(
    [&compConfigDTOs, &dsRuntimeService, &compDescDTO]() {
      compConfigDTOs =
        dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
    },
    [&compConfigDTOs]() -> bool {
      return compConfigDTOs.at(0).state == scr::dto::ComponentState::ACTIVE;
    });

  ASSERT_TRUE(result) << "Timed out waiting for state to change to ACTIVE "
                         "after the dependency became available";
  auto svcRef = bc.GetServiceReference<test::Interface2>();
  ASSERT_TRUE(svcRef);
  auto svc = bc.GetService<test::Interface2>(svcRef);
  ASSERT_TRUE(svc);
  EXPECT_NO_THROW(svc->ExtendedDescription());
  EXPECT_STREQ(
    "ServiceComponentDynamicReluctantMandatoryUnary depends on Interface3",
    svc->ExtendedDescription().c_str())
    << "String value returned was not expected. Was the correct service "
       "dependency bound?";

  // registering a new service with a higher rank should not cause re-binding
  ASSERT_TRUE(bc.RegisterService<test::Interface3>(
    std::make_shared<test::InterfaceImpl>("higher ranked Interface4"),
    { { Constants::SERVICE_RANKING, Any(10000) } }));
  EXPECT_NO_THROW(svc->ExtendedDescription());
  EXPECT_STREQ("ServiceComponentDynamicReluctantMandatoryUnary depends on Interface3",
               svc->ExtendedDescription().c_str())
    << "String value returned was not expected. Was the correct service "
       "dependency bound?";

  // registering a new service with a lower rank should NOT cause re-binding
  ASSERT_TRUE(bc.RegisterService<test::Interface3>(
    std::make_shared<test::InterfaceImpl>("lower ranked Interface4"),
    { { Constants::SERVICE_RANKING, Any(1) } }));
  EXPECT_NO_THROW(svc->ExtendedDescription());
  EXPECT_STREQ(
    "ServiceComponentDynamicReluctantMandatoryUnary depends on Interface3",
    svc->ExtendedDescription().c_str())
    << "String value returned was not expected. Was the correct service "
       "dependency bound?";

  depSvcReg.Unregister();
  EXPECT_FALSE(bc.GetServiceReference<test::Interface2>())
    << "Service should now NOT be available";
  EXPECT_THROW(svc->ExtendedDescription(), std::runtime_error);
  testBundle.Stop();
}

}
}
