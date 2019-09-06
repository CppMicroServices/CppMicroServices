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

#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/BundleContext.h"
#include "Mocks.hpp"
#include "../src/ServiceComponentRuntimeImpl.hpp"

namespace cppmicroservices {
namespace scrimpl {

// The fixture for testing class ServiceComponentRuntimeImpl.
class ServiceComponentRuntimeImplTest
  : public ::testing::Test
{
protected:
  ServiceComponentRuntimeImplTest() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {
  }

  virtual ~ServiceComponentRuntimeImplTest() = default;

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

TEST_F(ServiceComponentRuntimeImplTest, Validate_Ctor)
{
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  EXPECT_THROW({
      ServiceComponentRuntimeImpl service(BundleContext(),
                                          mockRegistry,
                                          fakeLogger);
    }, std::invalid_argument);
  EXPECT_THROW({
      ServiceComponentRuntimeImpl service(GetFramework().GetBundleContext(),
                                          nullptr,
                                          fakeLogger);
    }, std::invalid_argument);
  EXPECT_THROW({
      ServiceComponentRuntimeImpl service(GetFramework().GetBundleContext(),
                                          mockRegistry,
                                          nullptr);
    }, std::invalid_argument);
  EXPECT_NO_THROW({
      ServiceComponentRuntimeImpl service(GetFramework().GetBundleContext(),
                                          mockRegistry,
                                          fakeLogger);
      EXPECT_EQ(service.scrContext, GetFramework().GetBundleContext());
      EXPECT_EQ(service.registry, mockRegistry);
      EXPECT_EQ(service.logger, fakeLogger);
    });
}

TEST_F(ServiceComponentRuntimeImplTest, Validate_GetComponentDescriptionDTO)
{
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  auto mockCompMgr = std::make_shared<MockComponentManager>();
  auto fakeCompDesc = std::make_shared<metadata::ComponentMetadata>();
  fakeCompDesc->name = "componentname";
  metadata::ReferenceMetadata refData;
  fakeCompDesc->refsMetadata.push_back(refData);
  ServiceComponentRuntimeImpl service(GetFramework().GetBundleContext(),
                                      mockRegistry,
                                      fakeLogger);
  EXPECT_CALL(*mockRegistry, GetComponentManager(GetFramework().GetBundleId(), "Foo::Bar"))
    .Times(1)
    .WillRepeatedly(testing::Return(mockCompMgr));
  EXPECT_CALL(*mockCompMgr, GetMetadata())
    .Times(1)
    .WillRepeatedly(testing::Return(fakeCompDesc));
  EXPECT_CALL(*mockCompMgr, GetBundleId())
    .Times(1)
    .WillRepeatedly(testing::Return(GetFramework().GetBundleId()));
  EXPECT_NO_THROW({
      auto compDesc = service.GetComponentDescriptionDTO(GetFramework(), "Foo::Bar");
      EXPECT_EQ(compDesc.name, "componentname");
      EXPECT_EQ(compDesc.references.size(), 1u);
    });

  EXPECT_CALL(*mockRegistry, GetComponentManager(GetFramework().GetBundleId(), "FooBar"))
    .Times(1)
    .WillRepeatedly(testing::Throw(std::out_of_range("Invalid Component")));
  EXPECT_NO_THROW({
      auto compDesc = service.GetComponentDescriptionDTO(GetFramework(), "FooBar");
      EXPECT_EQ(compDesc.name, "");
    });
}

TEST_F(ServiceComponentRuntimeImplTest, Validate_GetComponentDescriptionDTOs_EmptyArg)
{
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  ServiceComponentRuntimeImpl service(GetFramework().GetBundleContext(),
                                      mockRegistry,
                                      fakeLogger);
  auto mgr1 = std::make_shared<MockComponentManager>();
  auto mgr2 = std::make_shared<MockComponentManager>();
  std::vector<std::shared_ptr<ComponentManager>> mgrs {mgr1, mgr2};
  EXPECT_CALL(*mockRegistry, GetComponentManagers())
    .Times(2)
    .WillOnce(testing::Return(std::vector<std::shared_ptr<ComponentManager>>{}))
    .WillOnce(testing::Return(mgrs));
  EXPECT_CALL(*mgr1, GetMetadata())
    .Times(1)
    .WillRepeatedly(testing::Return(nullptr));
  EXPECT_CALL(*mgr2, GetMetadata())
    .Times(1)
    .WillRepeatedly(testing::Return(nullptr));

  // check against empty registry
  EXPECT_NO_THROW({
      auto compDTOs = service.GetComponentDescriptionDTOs({});
      EXPECT_EQ(compDTOs.size(), 0u);
    });

  // check against registry with valid elements
  EXPECT_NO_THROW({
      auto compDTOs = service.GetComponentDescriptionDTOs({});
      EXPECT_EQ(compDTOs.size(), mgrs.size());
    });
}

TEST_F(ServiceComponentRuntimeImplTest, Validate_GetComponentDescriptionDTOs)
{
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  ServiceComponentRuntimeImpl service(GetFramework().GetBundleContext(),
                                      mockRegistry,
                                      fakeLogger);
  auto mgr1 = std::make_shared<MockComponentManager>();
  auto mgr2 = std::make_shared<MockComponentManager>();
  std::vector<std::shared_ptr<ComponentManager>> mgrs {mgr1, mgr2};
  EXPECT_CALL(*mockRegistry, GetComponentManagers(GetFramework().GetBundleId()))
    .Times(1)
    .WillRepeatedly(testing::Return(mgrs));
  EXPECT_CALL(*mgr1, GetMetadata())
    .Times(1)
    .WillRepeatedly(testing::Return(nullptr));
  EXPECT_CALL(*mgr2, GetMetadata())
    .Times(1)
    .WillRepeatedly(testing::Return(nullptr));
  EXPECT_NO_THROW({
      auto compDTOs = service.GetComponentDescriptionDTOs({GetFramework()});
      EXPECT_EQ(compDTOs.size(), mgrs.size());
    });
//      This test point fails due to a crash in the core framework
//      EXPECT_NO_THROW({
//        auto compDTOs = service.GetComponentDescriptionDTOs({Bundle()});
//        EXPECT_EQ(compDTOs.size(), 0UL);
//      });
}

TEST_F(ServiceComponentRuntimeImplTest, GetComponentConfigurationDTOs)
{
  ComponentDescriptionDTO compDescDTO;
  compDescDTO.name = "FooBar";
  compDescDTO.bundle.id = 21;
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  ServiceComponentRuntimeImpl service(GetFramework().GetBundleContext(),
                                      mockRegistry,
                                      fakeLogger);
  auto mockCompMgr = std::make_shared<MockComponentManager>();
  auto config1 = std::make_shared<MockComponentConfiguration>();
  auto config2 = std::make_shared<MockComponentConfiguration>();
  std::vector<std::shared_ptr<ComponentConfiguration>> configs {config1, config2};
  EXPECT_CALL(*mockRegistry, GetComponentManager(21, "FooBar"))
    .WillRepeatedly(testing::Return(mockCompMgr));
  EXPECT_CALL(*mockCompMgr, GetComponentConfigurations())
    .WillRepeatedly(testing::Return(configs));
  EXPECT_CALL(*mockCompMgr, GetMetadata())
    .WillRepeatedly(testing::Return(nullptr));
  EXPECT_CALL(*config1, GetId())
    .WillRepeatedly(testing::Return(100));
  EXPECT_CALL(*config2, GetId())
    .WillRepeatedly(testing::Return(200));
  std::unordered_map<std::string, cppmicroservices::Any> emptyProperties;
  EXPECT_CALL(*config1, GetProperties())
    .WillRepeatedly(testing::Return(emptyProperties));
  EXPECT_CALL(*config2, GetProperties())
    .WillRepeatedly(testing::Return(emptyProperties));
  EXPECT_CALL(*config1, GetConfigState())
    .WillRepeatedly(testing::Return(service::component::runtime::dto::UNSATISFIED_REFERENCE));
  EXPECT_CALL(*config2, GetConfigState())
    .WillRepeatedly(testing::Return(service::component::runtime::dto::ACTIVE));
  auto refMgr1 = std::make_shared<MockReferenceManager>();
  auto refMgr2 = std::make_shared<MockReferenceManager>();
  std::vector<std::shared_ptr<ReferenceManager>> refMgrs {refMgr1, refMgr2};
  EXPECT_CALL(*config1, GetAllDependencyManagers())
    .WillRepeatedly(testing::Return(std::vector<std::shared_ptr<ReferenceManager>>{}));
  EXPECT_CALL(*config2, GetAllDependencyManagers())
    .WillRepeatedly(testing::Return(refMgrs));
  EXPECT_CALL(*refMgr1, IsSatisfied()).WillRepeatedly(testing::Return(false));
  EXPECT_CALL(*refMgr1, GetReferenceName()).WillRepeatedly(testing::Return("ref1"));
  EXPECT_CALL(*refMgr1, GetLDAPString()).WillRepeatedly(testing::Return("(OBJECTCLASS=ref1Impl)"));
  EXPECT_CALL(*refMgr1, GetTargetReferences())
    .WillRepeatedly(testing::Return(std::set<cppmicroservices::ServiceReferenceBase>{}));
  EXPECT_CALL(*refMgr2, IsSatisfied()).WillRepeatedly(testing::Return(true));
  EXPECT_CALL(*refMgr2, GetReferenceName()).WillRepeatedly(testing::Return("ref2"));
  EXPECT_CALL(*refMgr2, GetLDAPString()).WillRepeatedly(testing::Return("(OBJECTCLASS=ref2Impl)"));
  EXPECT_CALL(*refMgr2, GetBoundReferences())
    .WillRepeatedly(testing::Return(std::set<cppmicroservices::ServiceReferenceBase>{}));
  auto configDTOs = service.GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(configDTOs.size(), configs.size());
  EXPECT_EQ(configDTOs.at(0).id, 100ul);
  EXPECT_EQ(configDTOs.at(1).id, 200ul);
  EXPECT_EQ(configDTOs.at(0).state, service::component::runtime::dto::UNSATISFIED_REFERENCE);
  EXPECT_EQ(configDTOs.at(1).state, service::component::runtime::dto::ACTIVE);

  // no matching component in the ComponentRegistry
  compDescDTO.name = "FooBar";
  compDescDTO.bundle.id = 23;
  EXPECT_CALL(*mockRegistry, GetComponentManager(23, "FooBar"))
    .Times(1)
    .WillRepeatedly(testing::Throw(std::out_of_range("Unknown Component")));
  EXPECT_THROW({
      configDTOs = service.GetComponentConfigurationDTOs(compDescDTO);
    }, std::out_of_range);
}

TEST_F(ServiceComponentRuntimeImplTest, IsComponentEnabled)
{
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  ServiceComponentRuntimeImpl service(GetFramework().GetBundleContext(),
                                      mockRegistry,
                                      fakeLogger);
  ComponentDescriptionDTO compDescDTO;
  compDescDTO.name = "FooBar";
  compDescDTO.bundle.id = 21;
  auto mockCompMgr = std::make_shared<MockComponentManager>();
  EXPECT_CALL(*mockRegistry, GetComponentManager(21, "FooBar"))
    .Times(2)
    .WillRepeatedly(testing::Return(mockCompMgr));
  EXPECT_CALL(*mockCompMgr, IsEnabled())
    .Times(2)
    .WillOnce(testing::Return(false))
    .WillOnce(testing::Return(true));
  EXPECT_EQ(service.IsComponentEnabled(compDescDTO), false);
  EXPECT_EQ(service.IsComponentEnabled(compDescDTO), true);

  compDescDTO.bundle.id = 23;
  EXPECT_CALL(*mockRegistry, GetComponentManager(23, "FooBar"))
    .Times(1)
    .WillRepeatedly(testing::Throw(std::out_of_range("unknown component")));
  EXPECT_THROW(service.IsComponentEnabled(compDescDTO), std::out_of_range);
}

TEST_F(ServiceComponentRuntimeImplTest, EnableComponent)
{
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  ServiceComponentRuntimeImpl service(GetFramework().GetBundleContext(),
                                      mockRegistry,
                                      fakeLogger);
  ComponentDescriptionDTO compDescDTO;
  compDescDTO.name = "FooBar";
  compDescDTO.bundle.id = 21;
  auto mockCompMgr = std::make_shared<MockComponentManager>();
  std::promise<void> promise;
  promise.set_value();
  EXPECT_CALL(*mockRegistry, GetComponentManager(21, "FooBar"))
    .Times(1)
    .WillRepeatedly(testing::Return(mockCompMgr));
  EXPECT_CALL(*mockCompMgr, Enable())
    .Times(1)
    .WillOnce(testing::Return(promise.get_future().share()));
  EXPECT_NO_THROW({
      auto fut = service.EnableComponent(compDescDTO);
    });

  compDescDTO.bundle.id = 23;
  EXPECT_CALL(*mockRegistry, GetComponentManager(23, "FooBar"))
    .Times(1)
    .WillRepeatedly(testing::Throw(std::out_of_range("unknown component")));
  EXPECT_THROW(service.EnableComponent(compDescDTO), std::out_of_range);
}

TEST_F(ServiceComponentRuntimeImplTest, DisableComponent)
{
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  ServiceComponentRuntimeImpl service(GetFramework().GetBundleContext(),
                                      mockRegistry,
                                      fakeLogger);
  ComponentDescriptionDTO compDescDTO;
  compDescDTO.name = "FooBar";
  compDescDTO.bundle.id = 21;
  auto mockCompMgr = std::make_shared<MockComponentManager>();
  std::promise<void> promise;
  promise.set_value();
  EXPECT_CALL(*mockRegistry, GetComponentManager(21, "FooBar"))
    .Times(1)
    .WillRepeatedly(testing::Return(mockCompMgr));
  EXPECT_CALL(*mockCompMgr, Disable())
    .Times(1)
    .WillOnce(testing::Return(promise.get_future().share()));
  EXPECT_NO_THROW({
      auto fut = service.DisableComponent(compDescDTO);
    });

  compDescDTO.bundle.id = 23;
  EXPECT_CALL(*mockRegistry, GetComponentManager(23, "FooBar"))
    .Times(1)
    .WillRepeatedly(testing::Throw(std::out_of_range("unknown component")));
  EXPECT_THROW(service.DisableComponent(compDescDTO), std::out_of_range);
}

// declaration of the standalone helper functions defined in ServiceComponentRuntimeImpl.cpp
framework::dto::BundleDTO ToDTO(const cppmicroservices::Bundle& bundle);
framework::dto::ServiceReferenceDTO ToDTO(const cppmicroservices::ServiceReferenceBase& sRef);

TEST_F(ServiceComponentRuntimeImplTest, TestBundleDTO)
{
  auto bundleDTO = ToDTO(GetFramework());
  EXPECT_EQ(bundleDTO.id, static_cast<unsigned long>(GetFramework().GetBundleId()));
  EXPECT_EQ(bundleDTO.symbolicName, GetFramework().GetSymbolicName());
  EXPECT_EQ(bundleDTO.state, GetFramework().GetState());
  EXPECT_EQ(bundleDTO.version, GetFramework().GetVersion().ToString());
}

TEST_F(ServiceComponentRuntimeImplTest, TestServiceReferenceDTO)
{
  auto fc = GetFramework().GetBundleContext();
  auto iMap = std::make_shared<InterfaceMap>();
  auto obj = std::make_shared<double>();
  iMap->insert(std::make_pair("double", std::static_pointer_cast<void>(obj)));
  fc.RegisterService(iMap);

  auto sRef = fc.GetServiceReference("double");
  EXPECT_TRUE(static_cast<bool>(sRef));
  auto sRefDTO = ToDTO(sRef);

  EXPECT_EQ(sRefDTO.bundle, static_cast<unsigned long>(sRef.GetBundle().GetBundleId()));
  EXPECT_EQ(sRefDTO.properties.size(), sRef.GetPropertyKeys().size());
  EXPECT_EQ(sRefDTO.usingBundles.size(), sRef.GetUsingBundles().size());
}
}
}
