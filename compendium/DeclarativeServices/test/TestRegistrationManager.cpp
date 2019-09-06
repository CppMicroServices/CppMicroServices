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

#include <cstdio>
#include <iostream>
#include <algorithm>

#include "Mocks.hpp"

#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleImport.h>
#include <cppmicroservices/Constants.h>
#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/ServiceRegistration.h>
#include <cppmicroservices/ServiceTracker.h>

#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "../src/manager/RegistrationManager.hpp"

using cppmicroservices::Constants::SERVICE_SCOPE;
using cppmicroservices::Constants::SCOPE_BUNDLE;
using cppmicroservices::Constants::SCOPE_SINGLETON;
using cppmicroservices::Constants::SCOPE_PROTOTYPE;
using cppmicroservices::service::component::ComponentConstants::COMPONENT_ID;
using cppmicroservices::service::component::ComponentConstants::COMPONENT_NAME;

namespace cppmicroservices {
namespace scrimpl {

class MockService
  : public dummy::Reference1, public dummy::Reference2
{
};

class MockServiceFactory
  : public cppmicroservices::ServiceFactory
{
public:
  MockServiceFactory() : callcount(0) {};
  virtual ~MockServiceFactory() {};
  virtual cppmicroservices::InterfaceMapConstPtr GetService(const cppmicroservices::Bundle&,
                                                            const cppmicroservices::ServiceRegistrationBase&)
  {
    callcount++;
    return cppmicroservices::MakeInterfaceMap<MockService>(std::make_shared<MockService>());
  }

  virtual void UngetService(const cppmicroservices::Bundle&,
                            const cppmicroservices::ServiceRegistrationBase&,
                            const cppmicroservices::InterfaceMapConstPtr&)
  {
    callcount--;
  }
  int callcount;
};

enum RegState
{
  REGISTERED,
  UNREGISTERED
};

// The fixture for testing class RegistrationManager.
class RegistrationManagerTest
  : public ::testing::Test
{
protected:
  RegistrationManagerTest() : framework(cppmicroservices::FrameworkFactory().NewFramework())
  { }

  virtual ~RegistrationManagerTest() = default;

  virtual void SetUp()
  {
    framework.Start();
  }

  virtual void TearDown()
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Framework& GetFramework() { return framework; }
private:
  cppmicroservices::Framework framework;
};

TEST_F(RegistrationManagerTest, VerifyConstructor)
{
  auto fakeLogger = std::make_shared<FakeLogger>();
  std::vector<std::string> services {"Foo", "Bar"};
  auto bc = GetFramework().GetBundleContext();
  // invalid bundle context
  EXPECT_THROW({
      RegistrationManager rm(BundleContext(), services, SCOPE_SINGLETON, fakeLogger);
    }, std::invalid_argument) << "Invalid bundle context must result in a throw";
  // invalid service interfaces vector
  EXPECT_THROW({
      RegistrationManager rm(bc, {}, SCOPE_SINGLETON, fakeLogger);
    }, std::invalid_argument) << "Empty services param must result in a throw";
  // invalid service scope
  EXPECT_THROW({
      RegistrationManager rm(bc, services, "fuzzy", fakeLogger);
    }, std::invalid_argument) << "Invalid service scope must result in a throw";
  // invalid logger
  EXPECT_THROW({
      RegistrationManager rm(bc, services, SCOPE_SINGLETON, nullptr);
    }, std::invalid_argument) << "Invalid logger must result in a throw";

  EXPECT_NO_THROW({
      RegistrationManager rm(bc, services, SCOPE_SINGLETON, fakeLogger);
    }) << "No throw expected when all params are valid";
}

TEST_F(RegistrationManagerTest, VerifyUnregister)
{
  auto fakeLogger = std::make_shared<FakeLogger>();
  std::vector<std::string> services {"Foo", "Bar"};
  auto bc = GetFramework().GetBundleContext();
  {
    // Unregister without register
    RegistrationManager rm(bc, services, SCOPE_SINGLETON, fakeLogger);
    EXPECT_THROW({
        rm.UnregisterService();
      }, std::logic_error) << "Cannot unregister if not registered by this registration manager";
  }

  {
    // register then unregister
    RegistrationManager rm(bc, services, SCOPE_SINGLETON, fakeLogger);
    std::shared_ptr<ServiceFactory> mockServiceFactory = std::make_shared<MockFactory>();
    rm.serviceReg = bc.RegisterService<dummy::Reference1>(mockServiceFactory);
    EXPECT_TRUE(rm.IsServiceRegistered()) << "state must be REGISTERED after call to RegisterService";
    EXPECT_NO_THROW({
        rm.UnregisterService();
      });
    EXPECT_FALSE(rm.IsServiceRegistered()) << "state must be UNREGISTERED after call to UnregisterService";

    // unregister after an unregister
    EXPECT_THROW({
        rm.UnregisterService();
      }, std::logic_error) << "Unregistering an already unregistered service must throw";
  }
}

TEST_F(RegistrationManagerTest, VerifyRegisterService)
{
  auto fakeLogger = std::make_shared<FakeLogger>();
  std::vector<std::string> services {us_service_interface_iid<dummy::Reference1>(), us_service_interface_iid<dummy::Reference2>()};
  auto bc = GetFramework().GetBundleContext();
  RegistrationManager rm(bc, services, SCOPE_SINGLETON, fakeLogger);
  auto mockServiceFactory = std::make_shared<MockFactory>();
  EXPECT_FALSE(rm.IsServiceRegistered()) << "Initial state must be UNREGISTERED";
  EXPECT_FALSE(static_cast<bool>(rm.GetServiceReference())) << "Initial call to GetServiceReference must return invalid ServiceReference object";
  ServiceProperties props;
  props.insert(std::make_pair(COMPONENT_NAME, Any(std::string("name"))));
  props.insert(std::make_pair(COMPONENT_ID, Any(978723ul)));
  EXPECT_TRUE(rm.RegisterService(std::dynamic_pointer_cast<ServiceFactory>(mockServiceFactory), props)) << "RegisterService must return true";
  EXPECT_TRUE(rm.IsServiceRegistered()) << "state must be REGISTERED after call to RegisterService";
  cppmicroservices::ServiceReference<dummy::Reference1> sRef1 = bc.GetServiceReference<dummy::Reference1>();
  // Verify service is registered
  EXPECT_EQ(rm.GetServiceReference(), sRef1) << "service reference stored in the registration manager must be the same as the one retrieved from the framework for 'dummy::Reference1' interface";
  EXPECT_TRUE(sRef1.IsConvertibleTo(services.at(0))) << "Retrieved service reference must be convertible to 'dummy::Reference1' interface";
  EXPECT_TRUE(rm.RegisterService(std::dynamic_pointer_cast<ServiceFactory>(mockServiceFactory), props)) << "A second call to RegisterService must return true without actually performing a service registration";
  cppmicroservices::ServiceReference<dummy::Reference2> sRef2 = bc.GetServiceReference<dummy::Reference2>();
  EXPECT_EQ(rm.GetServiceReference(), sRef2) << "service reference stored in the registration manager must be the same as the one retrieved from the framework for 'dummy::Reference2' interface";
  EXPECT_TRUE(sRef2.IsConvertibleTo(services.at(0))) << "Retrieved service reference must be convertible to 'dummy::Reference2' interface";
  InterfaceMapPtr iMap = MakeInterfaceMap<dummy::Reference2, dummy::Reference1>(std::make_shared<MockService>());
  EXPECT_CALL(*mockServiceFactory, GetService(testing::_, testing::_))
    .Times(1)
    .WillOnce(testing::Return(iMap));
  auto service2 = bc.GetService<dummy::Reference2>(sRef2);
  EXPECT_FALSE(service2 == nullptr);
  auto service1 = bc.GetService<dummy::Reference1>(sRef1);
  EXPECT_FALSE(service1 == nullptr);
  EXPECT_CALL(*mockServiceFactory, UngetService(testing::_, testing::_, testing::_))
    .Times(1);
  // Verify service properties
  // All services are registered with a factory
  EXPECT_EQ(sRef1.GetProperty(SERVICE_SCOPE).ToString(), SCOPE_SINGLETON) << "Service scope must be SINGLETON";
  EXPECT_EQ(sRef1.GetProperty(COMPONENT_NAME).ToString(), "name") << "Service must have a 'COMPONENT_NAME' property with value 'name'";
  EXPECT_EQ(any_cast<unsigned long>(sRef1.GetProperty(COMPONENT_ID)), 978723ul) << "Service must have a 'COMPONENT_ID' property with value '978723'";
}
}
}
