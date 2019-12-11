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

#include "cppmicroservices/ServiceFactory.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/ServiceObjects.h"
#include "gmock/gmock.h"

using namespace cppmicroservices;

namespace {
// Service interfaces
struct ITestServiceA
{
  virtual ~ITestServiceA() {}
};

struct ITestServiceB
{
  virtual ~ITestServiceB() {}
};

// Service implementations
struct TestServiceAImpl
  : public ITestServiceA
{};

// Mocks
class MockFactory
  : public ServiceFactory
{
public:
  MOCK_METHOD2(GetService,
               InterfaceMapConstPtr(const Bundle&
                                    , const ServiceRegistrationBase&));
  MOCK_METHOD3(UngetService,
               void(const Bundle&,
                    const ServiceRegistrationBase&,
                    const InterfaceMapConstPtr&));
};

}

class ServiceFactoryTest : public ::testing::Test
{
public:
  ServiceFactoryTest()
    : framework(FrameworkFactory().NewFramework()){};
  ~ServiceFactoryTest() override = default;
  void SetUp() override { framework.Start(); }

  void TearDown() override
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

protected:
  Framework framework;
};

// Tests the return type and FrameworkEvent generated when a
// ServiceFactory instance returns a map that does not contain
// all the interfaces specified in the call to RegisterService
TEST_F(ServiceFactoryTest, TestGetServiceReturnsIncompleteMap)
{
  auto context = framework.GetBundleContext();
  auto sf = std::make_shared<MockFactory>();
  EXPECT_CALL(*sf, GetService(testing::_, testing::_))
    .Times(2)
    .WillRepeatedly(testing::Invoke(
      [](const Bundle& /*bundle*/, const ServiceRegistrationBase& /*reg*/) {
        std::shared_ptr<ITestServiceA> implPtr =
          std::make_shared<TestServiceAImpl>();
        return MakeInterfaceMap<ITestServiceA>(implPtr);
      }));
  EXPECT_CALL(*sf, UngetService(testing::_, testing::_, testing::_)).Times(0);

  ServiceRegistration<ITestServiceA, ITestServiceB> reg1 =
    context.RegisterService<ITestServiceA, ITestServiceB>(
      ToFactory(sf),
      { { Constants::SERVICE_SCOPE, Any(Constants::SCOPE_PROTOTYPE) } });

  auto sref1 = context.GetServiceReference<ITestServiceA>();
  ASSERT_TRUE(static_cast<bool>(sref1));
  FrameworkEvent lastEvent;
  auto lToken = context.AddFrameworkListener(
    [](const cppmicroservices::FrameworkEvent& evt) {
      ASSERT_EQ(evt.GetType(), FrameworkEvent::FRAMEWORK_WARNING);
    });
  ASSERT_EQ(context.GetService<ITestServiceA>(sref1), nullptr);
  auto sref2 = context.GetServiceReference<ITestServiceB>();
  ASSERT_TRUE(static_cast<bool>(sref2));
  ASSERT_EQ(context.GetService<ITestServiceB>(sref2), nullptr);
  context.RemoveListener(std::move(lToken));
}

// Tests the return type and FrameworkEvent generated when a
// ServiceFactory instance throws an exception in it's GetService
// callback
TEST_F(ServiceFactoryTest, TestGetServiceThrows)
{
  auto context = framework.GetBundleContext();
  auto sf = std::make_shared<MockFactory>();
  std::string exceptionMsg("ServiceFactory threw an unknown exception.");
  EXPECT_CALL(*sf, GetService(testing::_, testing::_))
    .Times(1)
    .WillRepeatedly(testing::Throw(std::runtime_error(exceptionMsg)));
  EXPECT_CALL(*sf, UngetService(testing::_, testing::_, testing::_)).Times(0);

  ServiceRegistration<ITestServiceA> reg1 =
    context.RegisterService<ITestServiceA>(
      ToFactory(sf),
      { { Constants::SERVICE_SCOPE, Any(Constants::SCOPE_PROTOTYPE) } });

  auto sref = context.GetServiceReference<ITestServiceA>();
  ASSERT_TRUE(static_cast<bool>(sref));
  auto lToken = context.AddFrameworkListener(
    [&exceptionMsg](const cppmicroservices::FrameworkEvent& evt) {
      ASSERT_EQ(evt.GetType(), FrameworkEvent::FRAMEWORK_ERROR);
      ASSERT_NE(evt.GetThrowable(), nullptr);
      EXPECT_NO_THROW(try {
          std::rethrow_exception(evt.GetThrowable());
        } catch (const std::runtime_error& err) {
          ASSERT_STREQ(err.what(), exceptionMsg.c_str());
        });
    });
  ASSERT_EQ(context.GetService<ITestServiceA>(sref), nullptr);
  context.RemoveListener(std::move(lToken));
}

// Tests the return type and FrameworkEvent generated when a
// ServiceFactory instance throws an exception in it's GetService
// callback
TEST_F(ServiceFactoryTest, TestGetServiceObjectThrows)
{
  auto context = framework.GetBundleContext();
  auto sf = std::make_shared<MockFactory>();
  std::string exceptionMsg("ServiceFactory threw an unknown exception.");
  EXPECT_CALL(*sf, GetService(testing::_, testing::_))
    .Times(1)
    .WillRepeatedly(testing::Throw(std::runtime_error(exceptionMsg)));
  EXPECT_CALL(*sf, UngetService(testing::_, testing::_, testing::_)).Times(0);

  (void)context.RegisterService<ITestServiceA>(ToFactory(sf)
                                               , {{
                                                   Constants::SERVICE_SCOPE
                                                   , Any(Constants::SCOPE_PROTOTYPE)
                                                 }});

  auto sref = context.GetServiceReference<ITestServiceA>();
  auto serviceObjects = context.GetServiceObjects<ITestServiceA>(sref);

  // Next line used to crash if a service implementation threw an exception in the
  // constructor. This test case checks to make sure that the fix is in place.
  EXPECT_NO_THROW((void)serviceObjects.GetService());
}

// Tests the return type and FrameworkEvent generated when a
// ServiceFactory instance returns nullptr
TEST_F(ServiceFactoryTest, TestGetServiceReturnsNull)
{
  auto context = framework.GetBundleContext();
  auto sf = std::make_shared<MockFactory>();
  EXPECT_CALL(*sf, GetService(testing::_, testing::_))
    .Times(1)
    .WillRepeatedly(testing::Return(nullptr));
  EXPECT_CALL(*sf, UngetService(testing::_, testing::_, testing::_)).Times(0);

  ServiceRegistration<ITestServiceA> reg1 =
    context.RegisterService<ITestServiceA>(
      ToFactory(sf),
      { { Constants::SERVICE_SCOPE, Any(Constants::SCOPE_PROTOTYPE) } });

  auto sref = context.GetServiceReference<ITestServiceA>();
  ASSERT_TRUE(static_cast<bool>(sref));
  auto lToken = context.AddFrameworkListener(
    [](const cppmicroservices::FrameworkEvent& evt) {
      ASSERT_EQ(evt.GetType(), FrameworkEvent::FRAMEWORK_ERROR);
    });
  ASSERT_EQ(context.GetService<ITestServiceA>(sref), nullptr);
  context.RemoveListener(std::move(lToken));
}
