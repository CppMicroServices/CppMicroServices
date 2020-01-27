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

#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/ServiceInterface.h"
#include "../src/manager/ComponentConfigurationImpl.hpp"
#include "Mocks.hpp"
#include "ConcurrencyTestUtil.hpp"
#include "../src/manager/states/CCUnsatisfiedReferenceState.hpp"
#include "../src/manager/states/CCRegisteredState.hpp"
#include "../src/manager/states/CCActiveState.hpp"

#include "TestUtils.hpp"
#include <TestInterfaces/Interfaces.hpp>

using cppmicroservices::service::component::ComponentContext;

namespace cppmicroservices {
namespace scrimpl {

class ComponentConfigurationImplTest
  : public ::testing::Test
{
protected:
  ComponentConfigurationImplTest() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {
  }

  virtual ~ComponentConfigurationImplTest() = default;

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

TEST_F(ComponentConfigurationImplTest, VerifyCtor)
{
  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  EXPECT_THROW({
      auto  fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(nullptr,
                                                                              GetFramework(),
                                                                              mockRegistry,
                                                                              fakeLogger);
    }, std::invalid_argument);
  EXPECT_THROW({
      auto  fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                              GetFramework(),
                                                                              nullptr,
                                                                              fakeLogger);
    }, std::invalid_argument);
  EXPECT_THROW({
      auto  fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                              GetFramework(),
                                                                              mockRegistry,
                                                                              nullptr);
    }, std::invalid_argument);

  EXPECT_NO_THROW({
      auto  fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                              GetFramework(),
                                                                              mockRegistry,
                                                                              fakeLogger);
      EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
      EXPECT_EQ(fakeCompConfig->regManager, nullptr);
      EXPECT_EQ(fakeCompConfig->referenceManagers.size(), static_cast<size_t>(0));
    });
}

TEST_F(ComponentConfigurationImplTest, VerifyUniqueId)
{
  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  std::set<unsigned long> idSet;
  const size_t iterCount = 10;
  for(size_t i =0; i < iterCount; ++i)
  {
    EXPECT_NO_THROW({
        auto  fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                                GetFramework(),
                                                                                mockRegistry,
                                                                                fakeLogger);
        idSet.insert(fakeCompConfig->GetId());
      });
  }
  EXPECT_EQ(idSet.size(), iterCount);
}

TEST_F(ComponentConfigurationImplTest, VerifyRefSatisfied)
{
  // test case: A component has three dependencies. Dependency1 is already
  // available, Dependency 2 becomes available, then Dependency3 becomes available.
  // When Dependency2 becomes available, Dependency3 is still unavailable so
  // the component must not trigger a state change. When Dependency3 becomes
  // available, all the dependencies are now available which results in a state change.
  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  mockMetadata->serviceMetadata.interfaces = { us_service_interface_iid<dummy::ServiceImpl>() };
  auto refMgr1 = std::make_shared<MockReferenceManager>();
  auto refMgr2 = std::make_shared<MockReferenceManager>();
  auto refMgr3 = std::make_shared<MockReferenceManager>();
  EXPECT_CALL(*refMgr1, IsSatisfied())
    .WillRepeatedly(testing::Return(true)); // simulate pre-existing reference
  EXPECT_CALL(*refMgr2, IsSatisfied())
    .Times(1)
    .WillOnce(testing::Return(true));       // simulate reference that becomes available
  EXPECT_CALL(*refMgr3, IsSatisfied())
    .Times(1)
    .WillOnce(testing::Return(false));      // simulate reference that is unavailable when ref2 is satisfied
  auto mockFactory = std::make_shared<MockFactory>();
  auto mockCompInstance = std::make_shared<MockComponentInstance>();
  auto bc = GetFramework().GetBundleContext();
  auto  fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                          GetFramework(),
                                                                          mockRegistry,
                                                                          fakeLogger);
  EXPECT_CALL(*fakeCompConfig, GetFactory())
    .Times(1)
    .WillOnce(testing::Return(mockFactory));
  // add the mock reference managers to the config object
  fakeCompConfig->referenceManagers.insert(std::make_pair("ref1", refMgr1));
  fakeCompConfig->referenceManagers.insert(std::make_pair("ref2", refMgr2));
  fakeCompConfig->referenceManagers.insert(std::make_pair("ref3", refMgr3));
  EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  // callback from refMgr2
  fakeCompConfig->RefSatisfied("ref2");
  EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  // callback from refMgr3
  fakeCompConfig->RefSatisfied("ref3");
  EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::SATISFIED);
  EXPECT_EQ(fakeCompConfig->GetServiceReference().operator bool(), true);
  EXPECT_EQ(fakeCompConfig->GetServiceReference().IsConvertibleTo(mockMetadata->serviceMetadata.interfaces.at(0)), true);
  fakeCompConfig->referenceManagers.clear(); // remove the mock reference managers
}

TEST_F(ComponentConfigurationImplTest, VerifyRefUnsatisfied)
{
  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();

  auto  fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                          GetFramework(),
                                                                          mockRegistry,
                                                                          fakeLogger);
  auto mockStatisfiedState = std::make_shared<MockComponentConfigurationState>();
  auto mockUnsatisfiedState = std::make_shared<MockComponentConfigurationState>();
  EXPECT_CALL(*mockStatisfiedState, GetValue())
    .Times(2)
    .WillRepeatedly(testing::Return(service::component::runtime::dto::SATISFIED));
  EXPECT_CALL(*mockStatisfiedState, Deactivate(testing::_))
    .Times(1)
    .WillRepeatedly(testing::Invoke([&](ComponentConfigurationImpl& config){
                                      config.state = mockUnsatisfiedState;
                                    }));
  EXPECT_CALL(*mockUnsatisfiedState, GetValue())
    .Times(1)
    .WillRepeatedly(testing::Return(service::component::runtime::dto::UNSATISFIED_REFERENCE));
  auto refMgr1 = std::make_shared<MockReferenceManager>();
  fakeCompConfig->referenceManagers.insert(std::make_pair("ref1", refMgr1));
  fakeCompConfig->state = mockStatisfiedState;
  EXPECT_EQ(fakeCompConfig->GetConfigState(), service::component::runtime::dto::SATISFIED);
  fakeCompConfig->RefUnsatisfied("invalid_refname");
  EXPECT_EQ(fakeCompConfig->GetConfigState(), service::component::runtime::dto::SATISFIED);
  fakeCompConfig->RefUnsatisfied("ref1");
  EXPECT_EQ(fakeCompConfig->GetConfigState(), service::component::runtime::dto::UNSATISFIED_REFERENCE);
  fakeCompConfig->referenceManagers.clear(); // remove the mock reference managers
}

TEST_F(ComponentConfigurationImplTest, VerifyRefChangedState)
{
  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();

  // Test that a call to Register with a component containing both a service
  // and a reference to the same service interface will not cause a state change.
  scrimpl::metadata::ReferenceMetadata refMetadata{};
  refMetadata.interfaceName = "dummy::ServiceImpl";
  mockMetadata->serviceMetadata.interfaces = { us_service_interface_iid<dummy::ServiceImpl>() };
  mockMetadata->refsMetadata.push_back(refMetadata);
  auto fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(
    mockMetadata, GetFramework(), mockRegistry, fakeLogger);

  auto reg = GetFramework().GetBundleContext().RegisterService<dummy::ServiceImpl>(
    std::make_shared<dummy::ServiceImpl>());

  EXPECT_EQ(fakeCompConfig->GetConfigState(),
    service::component::runtime::dto::UNSATISFIED_REFERENCE);
  reg.Unregister();
}

TEST_F(ComponentConfigurationImplTest, VerifyRegister)
{
  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  // Test if a call to Register will change the state when the component
  // does not provide a service.
  {
    auto fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                            GetFramework(),
                                                                            mockRegistry,
                                                                            fakeLogger);
    EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
    EXPECT_EQ(fakeCompConfig->regManager, nullptr);
    EXPECT_EQ(fakeCompConfig->referenceManagers.size(), static_cast<size_t>(0));
    EXPECT_NO_THROW(fakeCompConfig->Register());
    EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::SATISFIED);
    EXPECT_EQ(fakeCompConfig->GetServiceReference().operator bool(), false);
  }
  // Test if a call to Register will change the state when the component
  // provides a service.
  {
    mockMetadata->serviceMetadata.interfaces = { us_service_interface_iid<dummy::ServiceImpl>() };
    auto mockFactory = std::make_shared<MockFactory>();
    auto  fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                            GetFramework(),
                                                                            mockRegistry,
                                                                            fakeLogger);
    EXPECT_CALL(*fakeCompConfig, GetFactory())
      .Times(1)
      .WillRepeatedly(testing::Return(mockFactory));
    EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
    EXPECT_NE(fakeCompConfig->regManager, nullptr);
    EXPECT_EQ(fakeCompConfig->referenceManagers.size(), static_cast<size_t>(0));
    EXPECT_NO_THROW(fakeCompConfig->Register());
    EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::SATISFIED);
    EXPECT_EQ(fakeCompConfig->GetServiceReference().operator bool(), true);
    EXPECT_EQ(fakeCompConfig->GetServiceReference().IsConvertibleTo(us_service_interface_iid<dummy::ServiceImpl>()), true);
  }
  // Test if a call to Register will change the state when the component
  // provides a service and component is immediate type.
  // For immediate component, a call to Register will result in immediate activation
  {
    mockMetadata->serviceMetadata.interfaces = { us_service_interface_iid<dummy::ServiceImpl>() };
    mockMetadata->immediate = true;
    auto mockFactory = std::make_shared<MockFactory>();
    auto mockCompInstance = std::make_shared<MockComponentInstance>();
    auto  fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                            GetFramework(),
                                                                            mockRegistry,
                                                                            fakeLogger);
    EXPECT_CALL(*fakeCompConfig, GetFactory())
      .Times(1)
      .WillRepeatedly(testing::Return(mockFactory));
    EXPECT_CALL(*fakeCompConfig, CreateAndActivateComponentInstance(testing::_))
      .Times(1)
      .WillRepeatedly(testing::Return(mockCompInstance));
    EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
    EXPECT_NE(fakeCompConfig->regManager, nullptr);
    EXPECT_EQ(fakeCompConfig->referenceManagers.size(), static_cast<size_t>(0));
    EXPECT_NO_THROW(fakeCompConfig->Register());
    EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::ACTIVE);
    EXPECT_EQ(fakeCompConfig->GetServiceReference().operator bool(), true);
    EXPECT_EQ(fakeCompConfig->GetServiceReference().IsConvertibleTo(us_service_interface_iid<dummy::ServiceImpl>()), true);
  }
}

TEST_F(ComponentConfigurationImplTest, VerifyStateChangeDelegation)
{
  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto mockState = std::make_shared<MockComponentConfigurationState>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  auto  fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                          GetFramework(),
                                                                          mockRegistry,
                                                                          fakeLogger);
  EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  fakeCompConfig->state = mockState;
  ComponentConfigurationImpl& fakeCompConfigBase = *(std::dynamic_pointer_cast<ComponentConfigurationImpl>(fakeCompConfig));
  EXPECT_CALL(*mockState, Register(testing::Ref(fakeCompConfigBase)))
    .Times(1);
  EXPECT_CALL(*mockState, Activate(testing::Ref(fakeCompConfigBase), GetFramework()))
    .Times(1);
  EXPECT_CALL(*mockState, Deactivate(testing::Ref(fakeCompConfigBase)))
    .Times(1);
  EXPECT_CALL(*mockState, GetValue())
    .Times(1);
  fakeCompConfig->Register();
  fakeCompConfig->Activate(GetFramework());
  fakeCompConfig->Deactivate();
  (void)fakeCompConfig->GetConfigState();
}

TEST_F(ComponentConfigurationImplTest, VerifyActivate_Success)
{
  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  auto  fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                          GetFramework(),
                                                                          mockRegistry,
                                                                          fakeLogger);
  fakeCompConfig->state = std::make_shared<CCRegisteredState>();
  auto mockCompInstance = std::make_shared<MockComponentInstance>();
  EXPECT_CALL(*fakeCompConfig, CreateAndActivateComponentInstance(testing::_))
    .Times(1)
    .WillRepeatedly(testing::Return(mockCompInstance));
  EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::SATISFIED);
  EXPECT_NO_THROW(fakeCompConfig->Activate(GetFramework()));
  EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::ACTIVE);
}

TEST_F(ComponentConfigurationImplTest, VerifyActivate_Failure)
{
  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  // Test for exception from user code
  auto  fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                          GetFramework(),
                                                                          mockRegistry,
                                                                          fakeLogger);
  fakeCompConfig->state = std::make_shared<CCRegisteredState>();
  EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::SATISFIED);
  EXPECT_CALL(*fakeCompConfig, CreateAndActivateComponentInstance(testing::_))
    .Times(1)
    .WillRepeatedly(testing::Return(nullptr));
  EXPECT_NO_THROW(fakeCompConfig->Activate(GetFramework()));
  EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::SATISFIED);
}

TEST_F(ComponentConfigurationImplTest, VerifyDeactivate)
{
  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  auto  fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                          GetFramework(),
                                                                          mockRegistry,
                                                                          fakeLogger);
  auto activeState = std::make_shared<CCActiveState>();
  fakeCompConfig->state = activeState;
  EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::ACTIVE);
  EXPECT_CALL(*fakeCompConfig, DestroyComponentInstances())
    .Times(1);
  EXPECT_NO_THROW(fakeCompConfig->Deactivate());
  EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
}

bool ValidateStateSequence(const std::vector<std::pair<ComponentState,ComponentState>>& stateArr)
{
  auto vecSize = stateArr.size();
  bool foundInvalidTransition = false;
  for(size_t i = 0; i < vecSize && !foundInvalidTransition; ++i)
  {
    auto currState = stateArr[i].first;
    auto nextState = stateArr[i].second;
    switch(currState)
    {
      case service::component::runtime::dto::UNSATISFIED_REFERENCE:
        if(nextState == service::component::runtime::dto::ACTIVE)
        {
          foundInvalidTransition = true;
        }
        break;
      case service::component::runtime::dto::SATISFIED:
        break;
      case service::component::runtime::dto::ACTIVE:
        if(nextState == service::component::runtime::dto::SATISFIED)
        {
          foundInvalidTransition = true;
        }
        break;
    }
  }
  return !foundInvalidTransition;
}

TEST_F(ComponentConfigurationImplTest, VerifyConcurrentRegisterDeactivate)
{
  // call register and deactivate from multiple threads simultaneously
  // ensure there is
  // - only one registration if the current state is SATISFIED
  // - zero registrations if the current state is UNSATISFIED_REFERENCE
  // ensure there are zero objects of ComponentInstance
  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  mockMetadata->serviceMetadata.interfaces = { "ServiceInterface", "interface" };
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  auto  fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                          GetFramework(),
                                                                          mockRegistry,
                                                                          fakeLogger);
  EXPECT_CALL(*fakeCompConfig, GetFactory())
    .WillRepeatedly(testing::Return(std::make_shared<MockFactory>()));
  EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
  EXPECT_NE(fakeCompConfig->regManager, nullptr);
  std::function<std::pair<ComponentState, ComponentState>()> func = [&fakeCompConfig]() {
                                                                      std::random_device rd;
                                                                      std::mt19937 gen(rd());
                                                                      std::uniform_int_distribution<unsigned int> dis;
                                                                      int randVal = dis(gen);
                                                                      auto prevState = fakeCompConfig->GetConfigState();
                                                                      if(randVal & 0x1)
                                                                      {
                                                                        fakeCompConfig->Register();
                                                                      }
                                                                      else
                                                                      {
                                                                        fakeCompConfig->Deactivate();
                                                                      }
                                                                      auto currentState = fakeCompConfig->GetConfigState();
                                                                      return std::make_pair(prevState, currentState);
                                                                    };

  auto results = ConcurrentInvoke(func);
  EXPECT_TRUE(ValidateStateSequence(results));
  if(fakeCompConfig->GetConfigState() == service::component::runtime::dto::SATISFIED)
  {
    EXPECT_EQ(GetFramework().GetBundleContext().GetServiceReferences("interface").size(), 1u);
  }
  else if(fakeCompConfig->GetConfigState() == service::component::runtime::dto::UNSATISFIED_REFERENCE)
  {
    EXPECT_EQ(GetFramework().GetBundleContext().GetServiceReferences("interface").size(), 0u);
  }
}

TEST_F(ComponentConfigurationImplTest, VerifyConcurrentActivateDeactivate)
{
  // call activate and deactivate from multiple threads simultaneously
  // ensure there the current state is UNSATISFIED_REFERENCE
  // ensure there are zero objects of ComponentInstance
  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  mockMetadata->serviceMetadata.interfaces = { "ServiceInterface", "interface" };
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  auto mockCompInstance = std::make_shared<MockComponentInstance>();
  auto fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                          GetFramework(),
                                                                          mockRegistry,
                                                                          fakeLogger);
  EXPECT_CALL(*fakeCompConfig, CreateAndActivateComponentInstance(testing::_))
    .WillRepeatedly(testing::Return(mockCompInstance));
  EXPECT_CALL(*fakeCompConfig, GetFactory())
    .WillRepeatedly(testing::Return(std::make_shared<MockFactory>()));
  fakeCompConfig->Register();
  EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::SATISFIED);
  auto clientBundle = GetFramework();
  std::function<std::pair<ComponentState,ComponentState>()> func = [&fakeCompConfig, &clientBundle]() {
                                                                     std::random_device rd;
                                                                     std::mt19937 gen(rd());
                                                                     std::uniform_int_distribution<unsigned int> dis;
                                                                     int randVal = dis(gen);
                                                                     auto prevState = fakeCompConfig->GetConfigState();
                                                                     if(randVal & 0x1)
                                                                     {
                                                                       fakeCompConfig->Activate(clientBundle);
                                                                     }
                                                                     else
                                                                     {
                                                                       fakeCompConfig->Deactivate();
                                                                     }
                                                                     auto currentState = fakeCompConfig->GetConfigState();
                                                                     return std::make_pair(prevState, currentState);
                                                                   };
  auto results = ConcurrentInvoke(func);
  EXPECT_TRUE(ValidateStateSequence(results));
}

TEST_F(ComponentConfigurationImplTest, VerifyImmediateComponent)
{
  EXPECT_NO_THROW({
      auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
      mockMetadata->immediate = true;
      auto mockRegistry = std::make_shared<MockComponentRegistry>();
      auto fakeLogger = std::make_shared<FakeLogger>();
      auto mockCompInstance = std::make_shared<MockComponentInstance>();
      auto fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                             GetFramework(),
                                                                             mockRegistry,
                                                                             fakeLogger);
      EXPECT_CALL(*fakeCompConfig, CreateAndActivateComponentInstance(testing::_))
        .Times(2)
        .WillRepeatedly(testing::Return(mockCompInstance));
      fakeCompConfig->Initialize();
      EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::ACTIVE);
      EXPECT_CALL(*fakeCompConfig, DestroyComponentInstances())
        .Times(1);
      fakeCompConfig->Deactivate();
      EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
      fakeCompConfig->Register();
      // since its an immediate component, it gets activated on call to Register.
      EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::ACTIVE);
    });
}

TEST_F(ComponentConfigurationImplTest, VerifyDelayedComponent)
{
  EXPECT_NO_THROW({
      auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
      auto mockRegistry = std::make_shared<MockComponentRegistry>();
      auto fakeLogger = std::make_shared<FakeLogger>();
      auto mockCompInstance = std::make_shared<MockComponentInstance>();
      auto mockFactory = std::make_shared<MockFactory>();

      mockMetadata->serviceMetadata.interfaces = { us_service_interface_iid<dummy::ServiceImpl>() };
      mockMetadata->immediate = false;
      auto fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                             GetFramework(),
                                                                             mockRegistry,
                                                                             fakeLogger);
      EXPECT_CALL(*fakeCompConfig, GetFactory())
        .Times(testing::AtLeast(1)) // 2
        .WillRepeatedly(testing::Return(mockFactory));
      fakeCompConfig->Initialize();
      EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::SATISFIED);
      EXPECT_CALL(*fakeCompConfig, CreateAndActivateComponentInstance(testing::_))
        .Times(testing::AtLeast(1)) // 2
        .WillRepeatedly(testing::Return(mockCompInstance));
      fakeCompConfig->Activate(GetFramework());
      EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::ACTIVE);
      EXPECT_CALL(*fakeCompConfig, DestroyComponentInstances())
        .Times(1);
      fakeCompConfig->Deactivate();
      EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::UNSATISFIED_REFERENCE);
      fakeCompConfig->Register();
      EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::SATISFIED);
      auto bc = GetFramework().GetBundleContext();
      auto sRef = bc.GetServiceReference<dummy::ServiceImpl>();
      ASSERT_EQ(sRef.operator bool(), true);
      ASSERT_EQ(sRef, fakeCompConfig->GetServiceReference());
      auto mockServiceImpl = std::make_shared<dummy::ServiceImpl>();
      InterfaceMapPtr instanceMap = MakeInterfaceMap<dummy::ServiceImpl>(mockServiceImpl);
      EXPECT_CALL(*mockFactory, GetService(testing::_, testing::_))
        .Times(1)
        .WillRepeatedly(testing::Invoke([&](const cppmicroservices::Bundle &b,
                                            const cppmicroservices::ServiceRegistrationBase&){
                                          fakeCompConfig->Activate(b);
                                          return instanceMap;
                                        }));
      EXPECT_CALL(*mockFactory, UngetService(testing::_, testing::_, testing::_));
      auto service = bc.GetService<dummy::ServiceImpl>(sRef);
      EXPECT_NE(service, nullptr);
      EXPECT_EQ(fakeCompConfig->GetConfigState(), ComponentState::ACTIVE);
    });
}

TEST_F(ComponentConfigurationImplTest, TestGetDependencyManagers)
{
  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  auto mockCompInstance = std::make_shared<MockComponentInstance>();
  auto mockFactory = std::make_shared<MockFactory>();

  mockMetadata->serviceMetadata.interfaces = { us_service_interface_iid<dummy::ServiceImpl>() };
  mockMetadata->immediate = false;
  metadata::ReferenceMetadata rm1;
  rm1.name = "Foo";
  rm1.interfaceName = "sample::Foo";
  metadata::ReferenceMetadata rm2;
  rm2.name = "Bar";
  rm2.interfaceName = "sample::Bar";
  mockMetadata->refsMetadata.push_back(rm1);
  mockMetadata->refsMetadata.push_back(rm2);
  auto fakeCompConfig = std::make_shared<MockComponentConfigurationImpl>(mockMetadata,
                                                                         GetFramework(),
                                                                         mockRegistry,
                                                                         fakeLogger);
  EXPECT_EQ(fakeCompConfig->GetAllDependencyManagers().size(), mockMetadata->refsMetadata.size());
  EXPECT_NE(fakeCompConfig->GetDependencyManager("Foo"), nullptr);
  EXPECT_NE(fakeCompConfig->GetDependencyManager("Bar"), nullptr);
}

TEST_F(ComponentConfigurationImplTest, TestComponentWithUniqueName)
{
#if defined(US_BUILD_SHARED_LIBS)
  auto dsPluginPath = test::GetDSRuntimePluginFilePath();
  auto dsbundles = GetFramework().GetBundleContext().InstallBundles(dsPluginPath);
  ASSERT_EQ(dsbundles.size(), 1);
  for (auto& bundle : dsbundles) {
    ASSERT_TRUE(bundle);
    bundle.Start();
  }
#endif

  auto testBundle = test::InstallAndStartBundle(GetFramework().GetBundleContext(), "TestBundleDSTOI8");
  ASSERT_TRUE(testBundle);
  ASSERT_EQ(testBundle.GetSymbolicName(), "TestBundleDSTOI8");

  auto svcRef = testBundle.GetBundleContext().GetServiceReference<test::Interface1>();
  ASSERT_TRUE(svcRef);
  auto svc = testBundle.GetBundleContext().GetService<test::Interface1>(svcRef);
  EXPECT_NE(svc, nullptr);
}
}
}
