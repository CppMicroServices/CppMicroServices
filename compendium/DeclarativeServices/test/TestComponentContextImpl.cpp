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

#include <chrono>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <future>

#include "gmock/gmock.h"
#include "Mocks.hpp"
#include <cppmicroservices/Constants.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/FrameworkEvent.h>
#include "cppmicroservices/servicecomponent/ComponentException.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "../src/ComponentContextImpl.hpp"

using cppmicroservices::service::component::ComponentContext;
using cppmicroservices::service::component::ComponentException;
using cppmicroservices::scrimpl::metadata::ComponentMetadata;
using cppmicroservices::Constants::SCOPE_SINGLETON;
using cppmicroservices::Constants::SCOPE_BUNDLE;
using cppmicroservices::Constants::SCOPE_PROTOTYPE;
using cppmicroservices::service::component::ComponentConstants::REFERENCE_SCOPE_PROTOTYPE_REQUIRED;

namespace cppmicroservices {
namespace scrimpl {

// The fixture for testing class ComponentContextImpl.
class ComponentContextImplTest
  : public ::testing::Test
{
protected:
  ComponentContextImplTest() : framework(cppmicroservices::FrameworkFactory().NewFramework())
  { }
  virtual ~ComponentContextImplTest() = default;

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

namespace test
{
struct Foo{};
struct Bar{};
}

TEST_F(ComponentContextImplTest, VerifyInvalidCtor)
{
  EXPECT_THROW({
      std::shared_ptr<ComponentContext> ctxt = std::make_shared<ComponentContextImpl>(std::weak_ptr<ComponentConfiguration>());
    }, ComponentException);
  EXPECT_THROW({
      std::shared_ptr<ComponentContext> ctxt = std::make_shared<ComponentContextImpl>(std::weak_ptr<ComponentConfiguration>(), GetFramework());
    }, ComponentException);
}

auto anyMapComparator = [](const std::pair<std::string, cppmicroservices::Any>& x,
                           const std::pair<std::string, cppmicroservices::Any>& y) -> bool {
                          auto ret = ((x.first == y.first) && (x.second.ToStringNoExcept() == y.second.ToStringNoExcept()));
                          return ret;
                        };

TEST_F(ComponentContextImplTest, VerifyComponentProperties)
{
  auto mockConfig = std::make_shared<testing::NiceMock<MockComponentConfiguration>>();
  std::shared_ptr<ComponentContext> ctxt = std::make_shared<ComponentContextImpl>(mockConfig);
  std::unordered_map<std::string, cppmicroservices::Any> fakeProps;
  fakeProps["foo"] = std::string("bar");
  EXPECT_CALL(*mockConfig, GetProperties())
    .Times(1)
    .WillRepeatedly(testing::Return(fakeProps));
  EXPECT_NO_THROW(
    {
      auto props = ctxt->GetProperties();
      bool compresult = std::equal(props.begin(), props.end(),
                                   fakeProps.begin(), fakeProps.end(),
                                   anyMapComparator);
      EXPECT_EQ(compresult, true);
    });
}

TEST_F(ComponentContextImplTest, VerifyLocateService)
{
  auto mockConfig = std::make_shared<MockComponentConfiguration>();
  auto fooServ = std::make_shared<test::Foo>();
  auto mockRefMgrFoo = std::make_shared<MockReferenceManager>();
  auto reg = GetFramework().GetBundleContext().RegisterService<test::Foo>(fooServ);
  std::set<cppmicroservices::ServiceReferenceBase> refsSet = { reg.GetReference() };
  EXPECT_CALL(*mockRefMgrFoo, GetBoundReferences())
    .Times(1)
    .WillRepeatedly(testing::Return(refsSet));
  EXPECT_CALL(*mockRefMgrFoo, GetReferenceName())
    .Times(1)
    .WillRepeatedly(testing::Return("foo"));
  EXPECT_CALL(*mockRefMgrFoo, GetReferenceScope())
    .Times(1)
    .WillRepeatedly(testing::Return(cppmicroservices::Constants::SCOPE_BUNDLE));
  EXPECT_CALL(*mockConfig, GetBundle())
    .WillRepeatedly(testing::Return(GetFramework()));
  std::vector<std::shared_ptr<ReferenceManager>> depMgrs{mockRefMgrFoo};
  EXPECT_CALL(*mockConfig, GetAllDependencyManagers())
    .Times(1)
    .WillRepeatedly(testing::Return(depMgrs));
  std::shared_ptr<ComponentContext> ctxt = std::make_shared<ComponentContextImpl>(mockConfig);
  EXPECT_NO_THROW(
    {
      auto serv = ctxt->LocateService<test::Foo>("foo");
      EXPECT_EQ(serv, fooServ); // test with correct type and name
      auto serv1 = ctxt->LocateService<test::Foo>("bar");
      EXPECT_EQ(serv1, nullptr); // test with correct type and wrong name
      auto serv2 = ctxt->LocateService<test::Bar>("foo");
      EXPECT_EQ(serv2, nullptr); // test with wrong type and correct name
    });
}


/**
 * This test point creates mutiple component configs per available bundle and
 * calls LocateService multiple times on each ComponentContext
 */
TEST_F(ComponentContextImplTest, DISABLED_VerifyLocateServiceWithReferenceScopes)
{
  size_t iterCount = 4ul;
  size_t componentInstanceCount = 3ul;
  auto bc = GetFramework().GetBundleContext();
  auto bundles = bc.GetBundles();
  auto VerifyLocateServiceWithScopes =
    [&](std::string referenceScope,
        std::string publishedScope,
        size_t expectedServiceInstanceCount)
    {
      auto mockConfig = std::make_shared<MockComponentConfiguration>();
      
      auto fakeLogger = std::make_shared<FakeLogger>();
      metadata::ReferenceMetadata fakeRefMetadata;
      fakeRefMetadata.name = "foo";
      fakeRefMetadata.interfaceName = us_service_interface_iid<dummy::ServiceImpl>();
      fakeRefMetadata.scope = referenceScope;
      
      std::set<std::shared_ptr<dummy::ServiceImpl>> instances;
      
      // publish a service
      auto mockServiceFactory = std::make_shared<MockFactory>();
      InterfaceMapConstPtr singletonInstance = MakeInterfaceMap<dummy::ServiceImpl>(std::make_shared<dummy::ServiceImpl>());
      EXPECT_CALL(*mockServiceFactory, GetService(testing::_, testing::_)).WillRepeatedly(testing::Invoke([&](const cppmicroservices::Bundle&,
                                                                                                              const cppmicroservices::ServiceRegistrationBase&) {
                                                                                                            if(publishedScope == SCOPE_SINGLETON)
                                                                                                            {
                                                                                                              return singletonInstance;
                                                                                                            }
                                                                                                            else
                                                                                                            {
                                                                                                              InterfaceMapConstPtr iMap = MakeInterfaceMap<dummy::ServiceImpl>(std::make_shared<dummy::ServiceImpl>());
                                                                                                              return iMap;
                                                                                                            }
                                                                                                          }));
      
      EXPECT_CALL(*mockServiceFactory, UngetService(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke([&](const cppmicroservices::Bundle&,
                                            const cppmicroservices::ServiceRegistrationBase&,
                                            const cppmicroservices::InterfaceMapConstPtr& service){
                                          //std::cout << "Unget called from bundle id " << bundle.GetBundleId() << std::endl;
                                          if(publishedScope == SCOPE_SINGLETON)
                                          {
                                            ASSERT_EQ(service, singletonInstance);
                                          }
                                        }));
      
      auto sReg = bc.RegisterService<dummy::ServiceImpl>(ToFactory(mockServiceFactory), {{cppmicroservices::Constants::SERVICE_SCOPE, Any(publishedScope)}});
      
      for(auto& bundle : bundles)
      {
        bundle.Start(); // In case the pkgtest bundle is not started
        auto bundleContext = bundle.GetBundleContext();
        auto mockRefMgrFoo = std::make_shared<ReferenceManagerImpl>(fakeRefMetadata, bundleContext, fakeLogger, "foobar");
        std::vector<std::shared_ptr<ReferenceManager>> depMgrs{mockRefMgrFoo};
        EXPECT_CALL(*mockConfig, GetAllDependencyManagers()).WillRepeatedly(testing::Return(depMgrs));
        EXPECT_CALL(*mockConfig, GetBundle()).WillRepeatedly(testing::Return(bundle));
        
        // component context has 1:1 relation with component instance, so we
        // use the context to validate the number of instances of a reference
        std::vector<std::shared_ptr<ComponentContext>> contexts;
        for(size_t i =0; i<componentInstanceCount; i++)
        {
          contexts.push_back(std::make_shared<ComponentContextImpl>(mockConfig));
        }
        
        for(size_t i =0; i<iterCount; i++)
        {
          for(auto ctxt : contexts)
          {
            auto serviceObj = ctxt->LocateService<dummy::ServiceImpl>("foo");
            if(serviceObj)
            {
              instances.insert(serviceObj);
            }
          }
        }
      }
      EXPECT_TRUE(std::none_of(instances.begin(), instances.end(), [](const std::shared_ptr<dummy::ServiceImpl>& sObj){ return (sObj == nullptr); }));
      EXPECT_EQ(instances.size(), expectedServiceInstanceCount);
      sReg.Unregister();
    };
  
  // See https://osgi.org/specification/osgi.cmpn/7.0.0/service.component.html#service.component-reference.scope
  // If the reference scope is BUNDLE, and service is published with SINGLETON
  // scope all component instances must receive the same instance of the reference
  VerifyLocateServiceWithScopes(SCOPE_BUNDLE,
                                SCOPE_SINGLETON,
                                1ul);
  // If the reference scope is BUNDLE, and service is published with BUNDLE
  // scope all component instances within the same bundle must receive the
  // same instance of the reference. Components from different bundles must
  // receive distinct instances
  VerifyLocateServiceWithScopes(SCOPE_BUNDLE,
                                SCOPE_BUNDLE,
                                bundles.size());
  // If the reference scope is BUNDLE, and service is published with PROTOTYPE
  // this is no different from the case where the service is published with
  // BUNDLE scope
  VerifyLocateServiceWithScopes(SCOPE_BUNDLE,
                                SCOPE_PROTOTYPE,
                                bundles.size());

  // If the reference scope is PROTOTYPE, and service is published with SINGLETON
  // scope all component instances must receive the same instance of the reference
  VerifyLocateServiceWithScopes(SCOPE_PROTOTYPE,
                                SCOPE_SINGLETON,
                                1ul);
  // If the reference scope is PROTOTYPE, and service is published with BUNDLE
  // scope all component instances within the same bundle must receive the
  // same instance of the reference. Components from different bundles must
  // receive distinct instances
  VerifyLocateServiceWithScopes(SCOPE_PROTOTYPE,
                                SCOPE_BUNDLE,
                                bundles.size());
  // If the reference scope is PROTOTYPE, and service is published with PROTOTYPE
  // scope each component instance must receive a unique instance of the reference
  // service
  VerifyLocateServiceWithScopes(SCOPE_PROTOTYPE,
                                SCOPE_PROTOTYPE,
                                bundles.size() * componentInstanceCount);

  // If the reference scope is PROTOTYPE_REQUIRED, and service is published
  // with SINGLETON scope, the reference remains unsatisfied
  VerifyLocateServiceWithScopes(REFERENCE_SCOPE_PROTOTYPE_REQUIRED,
                                SCOPE_SINGLETON,
                                0ul);
  // If the reference scope is PROTOTYPE_REQUIRED, and service is published
  // with BUNDLE scope, the reference remains unsatisfied
  VerifyLocateServiceWithScopes(REFERENCE_SCOPE_PROTOTYPE_REQUIRED,
                                SCOPE_BUNDLE,
                                0ul);
  // If the reference scope is PROTOTYPE_REQUIRED, and service is published
  // with PROTOTYPE scope each component instance must receive a unique
  // instance of the reference service
  VerifyLocateServiceWithScopes(REFERENCE_SCOPE_PROTOTYPE_REQUIRED,
                                SCOPE_PROTOTYPE,
                                bundles.size() * componentInstanceCount);
}

TEST_F(ComponentContextImplTest, VerifyLocateServiceWithHighestRank)
{
  auto mockConfig = std::make_shared<MockComponentConfiguration>();
  auto fooServ = std::make_shared<test::Foo>();
  auto mockRefMgrFoo = std::make_shared<MockReferenceManager>();
  auto reg = GetFramework().GetBundleContext().RegisterService<test::Foo>(fooServ);
  auto fooServ1 = std::make_shared<test::Foo>();
  auto reg1 = GetFramework().GetBundleContext().RegisterService<test::Foo>(fooServ1, {{cppmicroservices::Constants::SERVICE_RANKING, 20}});
  auto refs = GetFramework().GetBundleContext().GetServiceReferences(us_service_interface_iid<test::Foo>());
  std::set<cppmicroservices::ServiceReferenceBase> refsSet;
  refsSet.insert(refs.begin(), refs.end());
  EXPECT_CALL(*mockRefMgrFoo, GetBoundReferences())
    .Times(1)
    .WillRepeatedly(testing::Return(refsSet));
  EXPECT_CALL(*mockRefMgrFoo, GetReferenceName())
    .Times(1)
    .WillRepeatedly(testing::Return("foo"));
  EXPECT_CALL(*mockRefMgrFoo, GetReferenceScope())
    .Times(1)
    .WillRepeatedly(testing::Return(cppmicroservices::Constants::SCOPE_BUNDLE));
  EXPECT_CALL(*mockConfig, GetBundle())
    .WillRepeatedly(testing::Return(GetFramework()));
  std::vector<std::shared_ptr<ReferenceManager>> depMgrs{mockRefMgrFoo};
  EXPECT_CALL(*mockConfig, GetAllDependencyManagers())
    .Times(1)
    .WillRepeatedly(testing::Return(depMgrs));
  std::shared_ptr<ComponentContext> ctxt = std::make_shared<ComponentContextImpl>(mockConfig);
  EXPECT_NO_THROW(
    {
      auto service = ctxt->LocateService<test::Foo>("foo");
      EXPECT_NE(service.get(), fooServ.get());
      EXPECT_EQ(service.get(), fooServ1.get()); // test with correct type and name
    });
}

TEST_F(ComponentContextImplTest, VerifyLocateServiceWithLowestId)
{
  auto mockConfig = std::make_shared<MockComponentConfiguration>();
  auto mockRefMgrFoo = std::make_shared<MockReferenceManager>();
  auto fooServ = std::make_shared<test::Foo>();
  auto reg = GetFramework().GetBundleContext().RegisterService<test::Foo>(fooServ);
  auto fooServ1 = std::make_shared<test::Foo>();
  auto reg1 = GetFramework().GetBundleContext().RegisterService<test::Foo>(fooServ1);

  std::vector<cppmicroservices::ServiceReferenceBase> refArr;
  auto refs = GetFramework().GetBundleContext().GetServiceReferences(us_service_interface_iid<test::Foo>());
  std::set<cppmicroservices::ServiceReferenceBase> refsSet;
  refsSet.insert(refs.begin(), refs.end());
  EXPECT_CALL(*mockRefMgrFoo, GetBoundReferences())
    .Times(1)
    .WillRepeatedly(testing::Return(refsSet));
  EXPECT_CALL(*mockRefMgrFoo, GetReferenceName())
    .Times(1)
    .WillRepeatedly(testing::Return("foo"));
  EXPECT_CALL(*mockRefMgrFoo, GetReferenceScope())
    .Times(1)
    .WillRepeatedly(testing::Return(cppmicroservices::Constants::SCOPE_BUNDLE));
  EXPECT_CALL(*mockConfig, GetBundle())
    .WillRepeatedly(testing::Return(GetFramework()));
  std::vector<std::shared_ptr<ReferenceManager>> depMgrs{mockRefMgrFoo};
  EXPECT_CALL(*mockConfig, GetAllDependencyManagers())
    .Times(1)
    .WillRepeatedly(testing::Return(depMgrs));

  std::shared_ptr<ComponentContext> ctxt = std::make_shared<ComponentContextImpl>(mockConfig);
  EXPECT_NO_THROW(
    {
      auto service = ctxt->LocateService<test::Foo>("foo");
      EXPECT_EQ(service, fooServ); // test with correct type and name
    });
}

TEST_F(ComponentContextImplTest, VerifyLocateServices)
{
  auto mockConfig = std::make_shared<MockComponentConfiguration>();
  auto fooServ = std::make_shared<test::Foo>();
  auto mockRefMgrFoo = std::make_shared<MockReferenceManager>();
  auto reg = GetFramework().GetBundleContext().RegisterService<test::Foo>(fooServ);
  auto fooServ1 = std::make_shared<test::Foo>();
  auto reg1 = GetFramework().GetBundleContext().RegisterService<test::Foo>(fooServ1, {{cppmicroservices::Constants::SERVICE_RANKING, 20}});
  auto refs = GetFramework().GetBundleContext().GetServiceReferences(us_service_interface_iid<test::Foo>());
  std::set<cppmicroservices::ServiceReferenceBase> refsSet;
  refsSet.insert(refs.begin(), refs.end());
  EXPECT_CALL(*mockRefMgrFoo, GetBoundReferences())
    .Times(1)
    .WillRepeatedly(testing::Return(refsSet));
  EXPECT_CALL(*mockRefMgrFoo, GetReferenceName())
    .Times(1)
    .WillRepeatedly(testing::Return("foo"));
  EXPECT_CALL(*mockRefMgrFoo, GetReferenceScope())
    .Times(1)
    .WillRepeatedly(testing::Return(cppmicroservices::Constants::SCOPE_BUNDLE));
  EXPECT_CALL(*mockConfig, GetBundle())
    .WillRepeatedly(testing::Return(GetFramework()));
  std::vector<std::shared_ptr<ReferenceManager>> depMgrs{mockRefMgrFoo};
  EXPECT_CALL(*mockConfig, GetAllDependencyManagers())
    .Times(1)
    .WillRepeatedly(testing::Return(depMgrs));
  std::shared_ptr<ComponentContext> ctxt = std::make_shared<ComponentContextImpl>(mockConfig);
  EXPECT_NO_THROW({
      auto services = ctxt->LocateServices<test::Foo>("foo");
      EXPECT_EQ(services.size(), refs.size()); // test with correct type and name
    });
}

TEST_F(ComponentContextImplTest, VerifyUsingBundle)
{
  auto mockConfig = std::make_shared<testing::NiceMock<MockComponentConfiguration>>();
  // The context created without a usingBundle will always return an invalid bundle.
  std::shared_ptr<ComponentContext> ctxt = std::make_shared<ComponentContextImpl>(mockConfig);
  EXPECT_EQ(ctxt->GetUsingBundle().operator bool(), false);

  // The context created with a usingBundle must return the passed argument.
  std::shared_ptr<ComponentContext> ctxt1 = std::make_shared<ComponentContextImpl>(mockConfig,
                                                                                   GetFramework());
  EXPECT_EQ(ctxt1->GetUsingBundle().operator bool(), true);
  EXPECT_EQ(ctxt1->GetUsingBundle(), GetFramework());
}

TEST_F(ComponentContextImplTest, VerifyEnableComponent)
{
  auto mockConfig = std::make_shared<testing::NiceMock<MockComponentConfiguration>>();
  std::shared_ptr<ComponentContext> ctxt = std::make_shared<ComponentContextImpl>(mockConfig);
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto mockCompMgr = std::make_shared<MockComponentManager>();
  EXPECT_CALL(*mockConfig, GetBundle()).Times(1).WillRepeatedly(testing::Return(GetFramework()));
  EXPECT_CALL(*mockConfig, GetRegistry()).Times(1).WillRepeatedly(testing::Return(mockRegistry));
  EXPECT_CALL(*mockRegistry, GetComponentManager(GetFramework().GetBundleId(),"comp::name")).Times(1).WillRepeatedly(testing::Return(mockCompMgr));
  EXPECT_CALL(*mockCompMgr, Enable()).Times(1);
  ctxt->EnableComponent("comp::name");
}

TEST_F(ComponentContextImplTest, VerifyEnableComponentEmptyArg)
{
  auto mockConfig = std::make_shared<testing::NiceMock<MockComponentConfiguration>>();
  std::shared_ptr<ComponentContext> ctxt = std::make_shared<ComponentContextImpl>(mockConfig);
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto mockCompMgr = std::make_shared<MockComponentManager>();
  auto mockCompMgr1 = std::make_shared<MockComponentManager>();
  auto mockCompMgr2 = std::make_shared<MockComponentManager>();
  std::vector<std::shared_ptr<ComponentManager>> compmgrs = { mockCompMgr, mockCompMgr1, mockCompMgr2 };
  EXPECT_CALL(*mockConfig, GetBundle()).Times(1).WillRepeatedly(testing::Return(GetFramework()));
  EXPECT_CALL(*mockConfig, GetRegistry()).Times(1).WillRepeatedly(testing::Return(mockRegistry));
  EXPECT_CALL(*mockRegistry, GetComponentManagers(GetFramework().GetBundleId())).Times(1).WillRepeatedly(testing::Return(compmgrs));
  EXPECT_CALL(*mockCompMgr, Enable()).Times(1);
  EXPECT_CALL(*mockCompMgr1, Enable()).Times(1);
  EXPECT_CALL(*mockCompMgr2, Enable()).Times(1);
  ctxt->EnableComponent("");
}

TEST_F(ComponentContextImplTest, VerifyDisableComponent)
{
  auto mockConfig = std::make_shared<testing::NiceMock<MockComponentConfiguration>>();
  std::shared_ptr<ComponentContext> ctxt = std::make_shared<ComponentContextImpl>(mockConfig);
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto mockCompMgr = std::make_shared<MockComponentManager>();
  EXPECT_CALL(*mockConfig, GetBundle()).Times(1).WillRepeatedly(testing::Return(GetFramework()));
  EXPECT_CALL(*mockConfig, GetRegistry()).Times(1).WillRepeatedly(testing::Return(mockRegistry));
  EXPECT_CALL(*mockRegistry, GetComponentManager(GetFramework().GetBundleId(),"comp::name")).Times(1).WillRepeatedly(testing::Return(mockCompMgr));
  EXPECT_CALL(*mockCompMgr, Disable()).Times(1);
  ctxt->DisableComponent("comp::name");
}

TEST_F(ComponentContextImplTest, VerifyInvalidate)
{
  auto mockConfig = std::make_shared<testing::NiceMock<MockComponentConfiguration>>();
  // The context created without a usingBundle will always return an invalid bundle.
  std::shared_ptr<ComponentContextImpl> ctxt = std::make_shared<ComponentContextImpl>(mockConfig);
  EXPECT_CALL(*mockConfig, GetBundle())
    .WillRepeatedly(testing::Return(GetFramework()));
  EXPECT_EQ(ctxt->GetBundleContext().operator bool(), true);
  ctxt->Invalidate();
  EXPECT_THROW(ctxt->GetProperties(), ComponentException);
  EXPECT_THROW(ctxt->EnableComponent("foo"), ComponentException);
  EXPECT_THROW(ctxt->DisableComponent("foo"), ComponentException);
  EXPECT_EQ(ctxt->GetBundleContext().operator bool(), false);
  EXPECT_EQ(ctxt->GetServiceReference().operator bool(), false);
}
}
}
