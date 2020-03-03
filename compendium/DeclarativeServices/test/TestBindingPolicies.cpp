/**
 * @file      TestBindingPolicies.cpp
 * @copyright Copyright 2020 MathWorks, Inc. 
 */ 

#include <random>
#include <tuple>
#include <typeinfo>
#include <typeindex>

#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "../src/manager/ReferenceManagerImpl.hpp"

#include "Mocks.hpp"
#include "ConcurrencyTestUtil.hpp"

namespace cppmicroservices { namespace scrimpl {

class DummyServiceRefT
  : public ServiceReference<void>
{
};

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

class StaticGreedyTest
  : public ::testing::Test
{
protected:
  StaticGreedyTest()
    : framework(cppmicroservices::FrameworkFactory().NewFramework())
  {}
  
  virtual ~StaticGreedyTest() = default;

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
                                           , TestParam{"static", "reluctant", typeid(ReferenceManagerBaseImpl::BindingPolicyStaticReluctant) }
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
#if NEVER
TEST_F(StaticGreedyTest, NullReference)
{
  auto bc = GetFramework().GetBundleContext();
  auto fakeMetadata = CreateFakeReferenceMetadata("static", "greedy");
  auto fakeLogger = std::make_shared<FakeLogger>();
  auto mgr = std::make_shared<MockReferenceManagerBaseImpl>(fakeMetadata
                                                            , bc
                                                            , fakeLogger
                                                            , "foo");
  
  auto bindingPolicy = ReferenceManagerBaseImpl::CreateBindingPolicy(*mgr
                                                                     , fakeMetadata.policy
                                                                     , fakeMetadata.policyOption);

  // set up mock logger to expect call
  bindingPolicy->ServiceAdded(DummyServiceRefT());
  EXPECT_TRUE(true);
}

TEST_F(StaticGreedyTest, ServiceAddedNotSatisfied)
{
  auto bc = GetFramework().GetBundleContext();
  auto fakeMetadata = CreateFakeReferenceMetadata("static", "greedy");
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
  bindingPolicy->ServiceAdded(DummyServiceRefT());
  EXPECT_TRUE(true);
}
#endif

}}
