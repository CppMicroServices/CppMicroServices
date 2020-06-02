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
#include <typeinfo>
#include <typeindex>

#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp"
#include "../src/manager/ReferenceManagerImpl.hpp"

#include "Mocks.hpp"
#include "ConcurrencyTestUtil.hpp"
#include "TestUtils.hpp"
#include "TestInterfaces/Interfaces.hpp"


namespace scr = cppmicroservices::service::component::runtime;

namespace cppmicroservices { namespace scrimpl {

struct TestParam
{
  const char* policy;
  const char* policyOption;
  std::type_index policyType;
};
    
class BindingPolicyTest
  : public ::testing::TestWithParam<TestParam> {
protected:
  BindingPolicyTest()
    : framework(cppmicroservices::FrameworkFactory().NewFramework())
  {}
  
  virtual ~BindingPolicyTest() = default;

  virtual void SetUp() {
    framework.Start();
  }

  virtual void TearDown() {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Framework& GetFramework() { return framework; }
private:
  cppmicroservices::Framework framework;
};

// utility method for creating different types of reference metadata objects used in testing
metadata::ReferenceMetadata CreateFakeReferenceMetadata(const std::string& policy,
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

INSTANTIATE_TEST_SUITE_P(BindingPolicyTestParameterized
                         , BindingPolicyTest
                         , testing::Values(TestParam{"static", "greedy", typeid(ReferenceManagerBaseImpl::BindingPolicyStaticGreedy) }
                                           , TestParam{"static", "reluctant", typeid(ReferenceManagerBaseImpl::BindingPolicyStaticReluctant)}
                                           , TestParam{"dynamic", "greedy", typeid(ReferenceManagerBaseImpl::BindingPolicyDynamicGreedy)}
                                           , TestParam{"dynamic", "reluctant", typeid(ReferenceManagerBaseImpl::BindingPolicyDynamicReluctant)}
                               ));

TEST_P(BindingPolicyTest, TestPolicyCreation)
{
  auto bc = GetFramework().GetBundleContext();
  auto const& param = GetParam();

  auto fakeMetadata = CreateFakeReferenceMetadata(param.policy, param.policyOption);
  auto fakeLogger = std::make_shared<FakeLogger>();
  auto mgr = std::make_shared<MockReferenceManagerBaseImpl>(fakeMetadata
                                                            , bc
                                                            , fakeLogger
                                                            , "foo");
  
  auto bindingPolicy = ReferenceManagerBaseImpl::CreateBindingPolicy(*mgr
                                                                     , fakeMetadata.policy
                                                                     , fakeMetadata.policyOption);
  EXPECT_TRUE(bindingPolicy);
  auto* bindingPolicyData = bindingPolicy.get();
  
  EXPECT_EQ(param.policyType, typeid(*bindingPolicyData));
}

TEST_P(BindingPolicyTest, InvalidServiceReference)
{
  auto bc = GetFramework().GetBundleContext();
  auto const& param = GetParam();
  auto fakeMetadata = CreateFakeReferenceMetadata(param.policy, param.policyOption);
  auto fakeLogger = std::make_shared<FakeLogger>();
  auto mgr = std::make_shared<MockReferenceManagerBaseImpl>(fakeMetadata
                                                            , bc
                                                            , fakeLogger
                                                            , "foo");
  
  auto bindingPolicy = ReferenceManagerBaseImpl::CreateBindingPolicy(*mgr
                                                                     , fakeMetadata.policy
                                                                     , fakeMetadata.policyOption);

  // set up mock logger to expect call
  EXPECT_THROW(bindingPolicy->ServiceAdded(ServiceReferenceU()), std::invalid_argument);

}

// test binding a service under the following reference policy, referencep policy options and cardinality
// Cardinality: 0..1, 1..1
// reference policy: dynamic
// reference policy options: reluctant, greedy
TEST_F(BindingPolicyTest, TestBindingWithDynamicPolicyOptions)
{
  auto bc = GetFramework().GetBundleContext();
  test::InstallAndStartDS(bc);

  auto testBundle = test::InstallAndStartBundle(bc, "TestBundleDSTOI20");
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

  auto depBundle = test::InstallAndStartBundle(bc, "TestBundleDSTOI21");
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
  ASSERT_FALSE(svcRef);

  depBundle.Stop();
  EXPECT_FALSE(bc.GetServiceReference<test::Interface2>())
    << "Service should now NOT be available";
}

// test that dynamic greedy re-binding does not happen if a lower ranked service is registered.
TEST_F(BindingPolicyTest, TestDynamicGreedyReBind) {}

// test that binding happens only once for dynamic reluctant reference policy
TEST_F(BindingPolicyTest, TestDynamicReluctantReBind) {}



/*
TEST_P(BindingPolicyTest, ServiceAddedNotSatisfied)
{
  auto bc = GetFramework().GetBundleContext();
  auto const& param = GetParam();
  auto fakeMetadata =
    CreateFakeReferenceMetadata(param.policy, param.policyOption);
  auto fakeLogger = std::make_shared<FakeLogger>();
  auto mgr = std::make_shared<MockReferenceManagerBaseImpl>(fakeMetadata
                                                            , bc
                                                            , fakeLogger
                                                            , "foo");

  
  auto bindingPolicy = ReferenceManagerBaseImpl::CreateBindingPolicy(*mgr
                                                                     , fakeMetadata.policy
                                                                     , fakeMetadata.policyOption);


  using ::testing::Return;
  
  ON_CALL(*mgr, IsSatisfied()).WillByDefault(Return(true));
  
  // set up mock logger to expect call
  bindingPolicy->ServiceAdded(ServiceReferenceU());

}
*/
}}
