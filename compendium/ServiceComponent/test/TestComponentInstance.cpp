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
#include <chrono>

#include "gmock/gmock.h"

#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleImport.h>
#include <cppmicroservices/Constants.h>

#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp"
#include <cppmicroservices/ServiceInterface.h>

using cppmicroservices::service::component::ComponentContext;
using cppmicroservices::service::component::detail::ComponentInstanceImpl;
using cppmicroservices::service::component::detail::Binder;
using cppmicroservices::service::component::detail::StaticBinder;
using cppmicroservices::service::component::detail::DynamicBinder;

namespace {

// dummy types used for testing
struct ServiceDependency1 {};
struct ServiceDependency2 {};
struct TestServiceInterface1 {};
struct TestServiceInterface2 {};
struct TestServiceInterface3 {};

// Test class to simulate a servcie component
// TODO: Use Mock to add more behavior verification
class TestServiceImpl1 final
  : public TestServiceInterface1, public TestServiceInterface2
{
public:
  TestServiceImpl1()
    : foo(nullptr)
    , bar(nullptr)
    , activated(false)
  {
  }

  TestServiceImpl1(const std::shared_ptr<ServiceDependency1>& f,
                   const std::shared_ptr<ServiceDependency2>& b)
    : foo(f)
    , bar(b)
    , activated(false)
  {
  }

  TestServiceImpl1(const std::shared_ptr<ServiceDependency2>& b)
    : foo(nullptr)
    , bar(b)
    , activated(false)
  {
  }

  virtual ~TestServiceImpl1() {}

  void BindFoo(const std::shared_ptr<ServiceDependency1>& f)
  {
    foo = f;
  }

  void UnbindFoo(const std::shared_ptr<ServiceDependency1>& f)
  {
    if(foo == f)
    {
      foo = nullptr;
    }
  }

  void Activate()
  {
    // this is the wrong signature method.
  }

  void Activate(const std::shared_ptr<ComponentContext>&)
  {
    activated = true;
  }

  void Deactivate(const std::shared_ptr<ComponentContext>&)
  {
    activated = false;
  }

  void Deactivate(const std::shared_ptr<ComponentContext>&, bool)
  {
    // this is the wrong signature method.
  }

  bool IsActivated() { return activated; }

  std::shared_ptr<ServiceDependency1> GetFoo() const { return foo; }
  std::shared_ptr<ServiceDependency2> GetBar() const { return bar; }
private:
  std::shared_ptr<ServiceDependency1> foo;        // dynamic dependency - can change during the lifetime of this object
  const std::shared_ptr<ServiceDependency2> bar;  // static dependency - does not change during the lifetime of this object
  bool activated;
};

/**
 * This class is used to mock the behavior of a ComponentContext object
 * created by the declarative services runtime implementation.
 */
class MockComponentContext : public ComponentContext
{
public:
  MOCK_CONST_METHOD0(GetProperties, std::unordered_map<std::string, cppmicroservices::Any>(void));
  MOCK_CONST_METHOD0(GetBundleContext, cppmicroservices::BundleContext(void));
  MOCK_CONST_METHOD0(GetUsingBundle, cppmicroservices::Bundle(void));
  MOCK_CONST_METHOD0(GetServiceReference, cppmicroservices::ServiceReferenceBase(void));
  MOCK_METHOD1(EnableComponent, void(const std::string&));
  MOCK_METHOD1(DisableComponent, void(const std::string&));
  MOCK_CONST_METHOD2(LocateServices, std::vector<std::shared_ptr<void>>(const std::string&, const std::string&));
  MOCK_CONST_METHOD2(LocateService, std::shared_ptr<void>(const std::string&, const std::string&));
};

/**
 * This test point is used to verify the ComponentInstanceImpl works properly
 * for a Service Component that does not provide any services and does not
 * consume any services
 */
TEST(ComponentInstance, VerifyZeroServicesZeroDependencies)
{
  ComponentInstanceImpl<TestServiceImpl1> compInstance;
  auto iMap = compInstance.GetInterfaceMap();
  ASSERT_FALSE(iMap); // empty map is returned since no service is provided
  auto compObj = compInstance.GetInstance();
  ASSERT_FALSE(compObj);
  auto mockContext = std::make_shared<MockComponentContext>();
  EXPECT_CALL(*(mockContext.get()), LocateService(testing::_, testing::_)).Times(0); // ensure the mock context never gets a call to LocateService
  compInstance.CreateInstanceAndBindReferences(mockContext);
  compObj = compInstance.GetInstance();
  ASSERT_TRUE(compObj);
  // ensure none of the dependencies are bound. Dependencies are only bound if they are declared.
  ASSERT_FALSE(compObj->GetFoo());
  ASSERT_FALSE(compObj->GetBar());
}

TEST(ComponentInstanceImpl, VerifyZeroServicesZeroDependencies)
{
  ComponentInstanceImpl<TestServiceImpl1> compInstance;
  auto iMap = compInstance.GetInterfaceMap();
  ASSERT_FALSE(iMap); // empty map is returned since no service is provided
  auto compObj = compInstance.GetInstance();
  ASSERT_FALSE(compObj);
  auto mockContext = std::make_shared<MockComponentContext>();
  EXPECT_CALL(*(mockContext.get()), LocateService(testing::_, testing::_)).Times(0); // ensure the mock context never gets a call to LocateService
  compInstance.CreateInstanceAndBindReferences(mockContext);
  compObj = compInstance.GetInstance();
  ASSERT_TRUE(compObj);
  // ensure none of the dependencies are bound. Dependencies are only bound if they are declared.
  ASSERT_FALSE(compObj->GetFoo());
  ASSERT_FALSE(compObj->GetBar());
}

/**
 * This test point is used to verify the ComponentInstanceImpl works properly
 * for a Service Component that provides a single service but does not consume
 * any services
 */
TEST(ComponentInstance, VerifyWithSingleServiceZeroDependencies)
{
  ComponentInstanceImpl<TestServiceImpl1, std::tuple<TestServiceInterface1>> compInstance;
  auto iMap = compInstance.GetInterfaceMap();
  ASSERT_TRUE(iMap);
  ASSERT_EQ(iMap->size(), size_t(1));
  ASSERT_EQ(iMap->count(us_service_interface_iid<TestServiceInterface1>()), size_t(1));
  auto mockContext = std::make_shared<MockComponentContext>();
  EXPECT_CALL(*(mockContext.get()), LocateService(testing::_, testing::_)).Times(0); // ensure the mock context never gets a call to LocateService
  compInstance.CreateInstanceAndBindReferences(mockContext);
  auto compObj = compInstance.GetInstance();
  ASSERT_TRUE(compObj);
  // ensure none of the dependencies are bound. Dependencies are only bound if they are declared.
  ASSERT_FALSE(compObj->GetFoo());
  ASSERT_FALSE(compObj->GetBar());
}

TEST(ComponentInstanceImpl, VerifyWithSingleServiceZeroDependencies)
{
  ComponentInstanceImpl<TestServiceImpl1, std::tuple<TestServiceInterface1>> compInstance;
  auto iMap = compInstance.GetInterfaceMap();
  ASSERT_TRUE(iMap);
  ASSERT_EQ(iMap->size(), size_t(1));
  ASSERT_EQ(iMap->count(us_service_interface_iid<TestServiceInterface1>()), size_t(1));
  auto mockContext = std::make_shared<MockComponentContext>();
  EXPECT_CALL(*(mockContext.get()), LocateService(testing::_, testing::_)).Times(0); // ensure the mock context never gets a call to LocateService
  compInstance.CreateInstanceAndBindReferences(mockContext);
  auto compObj = compInstance.GetInstance();
  ASSERT_TRUE(compObj);
  // ensure none of the dependencies are bound. Dependencies are only bound if they are declared.
  ASSERT_FALSE(compObj->GetFoo());
  ASSERT_FALSE(compObj->GetBar());
}

/**
 * This test point is used to verify the ComponentInstanceImpl works properly
 * for a Service Component that provides multiple services but does not consume
 * any services
 */
TEST(ComponentInstance, VerifyWithMultipleServiceZeroDependencies)
{
  ComponentInstanceImpl<TestServiceImpl1, std::tuple<TestServiceInterface1, TestServiceInterface2>> compInstance;
  auto iMap = compInstance.GetInterfaceMap();
  ASSERT_TRUE(iMap);
  ASSERT_EQ(iMap->size(), size_t(2));
  ASSERT_EQ(iMap->count(us_service_interface_iid<TestServiceInterface1>()), size_t(1));
  ASSERT_EQ(iMap->count(us_service_interface_iid<TestServiceInterface2>()), size_t(1));
  auto mockContext = std::make_shared<MockComponentContext>();
  EXPECT_CALL(*(mockContext.get()), LocateService(testing::_, testing::_)).Times(0); // ensure the mock context never gets a call to LocateService
  compInstance.CreateInstanceAndBindReferences(mockContext);
  auto compObj = compInstance.GetInstance();
  ASSERT_TRUE(compObj);
  // ensure none of the dependencies are bound. Dependencies are only bound if they are declared.
  ASSERT_FALSE(compObj->GetFoo());
  ASSERT_FALSE(compObj->GetBar());
}

TEST(ComponentInstanceImpl, VerifyWithMultipleServiceZeroDependencies)
{
  ComponentInstanceImpl<TestServiceImpl1, std::tuple<TestServiceInterface1, TestServiceInterface2>> compInstance;
  auto iMap = compInstance.GetInterfaceMap();
  ASSERT_TRUE(iMap);
  ASSERT_EQ(iMap->size(), size_t(2));
  ASSERT_EQ(iMap->count(us_service_interface_iid<TestServiceInterface1>()), size_t(1));
  ASSERT_EQ(iMap->count(us_service_interface_iid<TestServiceInterface2>()), size_t(1));
  auto mockContext = std::make_shared<MockComponentContext>();
  EXPECT_CALL(*(mockContext.get()), LocateService(testing::_, testing::_)).Times(0); // ensure the mock context never gets a call to LocateService
  compInstance.CreateInstanceAndBindReferences(mockContext);
  auto compObj = compInstance.GetInstance();
  ASSERT_TRUE(compObj);
  // ensure none of the dependencies are bound. Dependencies are only bound if they are declared.
  ASSERT_FALSE(compObj->GetFoo());
  ASSERT_FALSE(compObj->GetBar());
}

TEST(ComponentInstanceImpl, VerifyWithStaticDependencies)
{
  auto f = cppmicroservices::FrameworkFactory().NewFramework();
  f.Start();
  auto fc = f.GetBundleContext();
  auto reg = fc.RegisterService<ServiceDependency1>(std::make_shared<ServiceDependency1>());
  auto reg1 = fc.RegisterService<ServiceDependency2>(std::make_shared<ServiceDependency2>());

  ComponentInstanceImpl<TestServiceImpl1, std::tuple<>, ServiceDependency1, ServiceDependency2> compInstance({("foo"), ("bar")}, {});
  auto iMap = compInstance.GetInterfaceMap();
  ASSERT_FALSE(iMap);

  auto mockContext = std::make_shared<MockComponentContext>();
  auto locateService = [&fc](const std::string& type) -> std::shared_ptr<void> {
    auto sRef = fc.GetServiceReference(type);
    if(sRef)
    {
      auto serv = fc.GetService(sRef);
      return serv->at(type);
    }
    else
    {
      return nullptr;
    }
  };

  EXPECT_CALL(*(mockContext.get()), GetBundleContext()).WillRepeatedly(testing::Invoke([&fc]() { return fc;}));

  EXPECT_CALL(*(mockContext.get()), LocateService("foo", us_service_interface_iid<ServiceDependency1>()))
    .Times(1)
    .WillRepeatedly(testing::WithArg<1>(testing::Invoke(locateService))); // ensure the mock context received a call to LocateService for dependency foo

  EXPECT_CALL(*(mockContext.get()), LocateService("bar", us_service_interface_iid<ServiceDependency2>()))
    .Times(1)
    .WillRepeatedly(testing::WithArg<1>(testing::Invoke(locateService))); // ensure the mock context received a call to LocateService for dependency bar

  compInstance.CreateInstanceAndBindReferences(mockContext);
  auto compObj = compInstance.GetInstance();
  ASSERT_TRUE(compObj);
  // ensure the dependencies are bound when the object is constructed
  ASSERT_TRUE(compObj->GetFoo());
  ASSERT_TRUE(compObj->GetBar());

  EXPECT_THROW(compInstance.InvokeBindMethod("foo", fc.GetServiceReference<ServiceDependency1>()), std::out_of_range);
  EXPECT_THROW(compInstance.InvokeBindMethod("bar", fc.GetServiceReference<ServiceDependency2>()), std::out_of_range);
  EXPECT_THROW(compInstance.InvokeUnbindMethod("foo", fc.GetServiceReference<ServiceDependency1>()), std::out_of_range);
  EXPECT_THROW(compInstance.InvokeUnbindMethod("bar", fc.GetServiceReference<ServiceDependency2>()), std::out_of_range);

  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());
}


TEST(ComponentInstanceImpl, VerifyWithDynamicDependencies)
{
  auto f = cppmicroservices::FrameworkFactory().NewFramework();
  f.Start();
  auto fc = f.GetBundleContext();
  auto reg = fc.RegisterService<ServiceDependency1>(std::make_shared<ServiceDependency1>());
  auto reg1 = fc.RegisterService<ServiceDependency2>(std::make_shared<ServiceDependency2>());

  std::vector<std::shared_ptr<Binder<TestServiceImpl1>>> binders;
  binders.push_back(std::make_shared<DynamicBinder<TestServiceImpl1, ServiceDependency1>>("foo", &TestServiceImpl1::BindFoo, &TestServiceImpl1::UnbindFoo));

  ComponentInstanceImpl<TestServiceImpl1, std::tuple<>, ServiceDependency2> compInstance({("bar")}, binders);
  auto iMap = compInstance.GetInterfaceMap();
  ASSERT_FALSE(iMap);
  ASSERT_EQ(compInstance.GetInstance(), nullptr);

  // create a mock context and setup expectations on the context
  auto mockContext = std::make_shared<MockComponentContext>();
  auto locateService = [&fc](const std::string& type) -> std::shared_ptr<void>
                       {
                         auto sRef = fc.GetServiceReference(type);
                         if(sRef)
                         {
                           auto serv = fc.GetService(sRef);
                           return serv->at(type);
                         }
                         else
                         {
                           return nullptr;
                         }
                       };

  EXPECT_CALL(*(mockContext.get()), GetBundleContext()).WillRepeatedly(testing::Invoke([&fc]() { return fc;}));

  EXPECT_CALL(*(mockContext.get()), LocateService("foo", us_service_interface_iid<ServiceDependency1>()))
    .Times(1)
    .WillRepeatedly(testing::WithArg<1>(testing::Invoke(locateService))); // ensure the mock context received a call to LocateService for dependency foo

  EXPECT_CALL(*(mockContext.get()), LocateService("bar", us_service_interface_iid<ServiceDependency2>()))
    .Times(1)
    .WillRepeatedly(testing::WithArg<1>(testing::Invoke(locateService))); // ensure the mock context received a call to LocateService for dependency bar

  // use mock context to create an instance of the component implementation class.
  compInstance.CreateInstanceAndBindReferences(mockContext);
  auto compObj = compInstance.GetInstance();
  ASSERT_TRUE(compObj);
  // ensure the dependencies are bound when the object is constructed
  ASSERT_NE(compObj->GetFoo(), nullptr);
  ASSERT_NE(compObj->GetBar(), nullptr);

  // ensure only dynamic dependencies can be re-bound.
  // The runtime calls the wrapper object with the name of the reference and the ServiceReference object to use for binding.
  EXPECT_THROW(compInstance.InvokeUnbindMethod("bar", fc.GetServiceReference<ServiceDependency2>()), std::out_of_range);
  EXPECT_THROW(compInstance.InvokeBindMethod("bar", fc.GetServiceReference<ServiceDependency2>()), std::out_of_range);
  EXPECT_NO_THROW(compInstance.InvokeUnbindMethod("foo", fc.GetServiceReference<ServiceDependency1>()));
  ASSERT_EQ(compObj->GetFoo(), nullptr);
  EXPECT_NO_THROW(compInstance.InvokeBindMethod("foo", fc.GetServiceReference<ServiceDependency1>()));
  ASSERT_NE(compObj->GetFoo(), nullptr);

  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());
}

/**
 * This test point is used to verify the service component implementation class
 * receives lifecycle callbacks if it implements the lifecycle hook methods.
 * i.e, the Activate and Deactivate methods with correct signature.
 */
TEST(ComponentInstance, VerifyLifeCycleHooks)
{
  ComponentInstanceImpl<TestServiceImpl1> compInstance;
  auto iMap = compInstance.GetInterfaceMap();
  ASSERT_FALSE(iMap);
  auto compObj = compInstance.GetInstance();
  ASSERT_FALSE(compObj);
  compInstance.CreateInstanceAndBindReferences(std::make_shared<MockComponentContext>());
  compObj = compInstance.GetInstance();
  ASSERT_FALSE(compObj->IsActivated());
  compInstance.Activate();
  ASSERT_TRUE(compObj->IsActivated());
  compInstance.Deactivate();
  ASSERT_FALSE(compObj->IsActivated());
}

TEST(ComponentInstanceImpl, VerifyLifeCycleHooks)
{
  ComponentInstanceImpl<TestServiceImpl1> compInstance;
  auto iMap = compInstance.GetInterfaceMap();
  ASSERT_FALSE(iMap);
  auto compObj = compInstance.GetInstance();
  ASSERT_FALSE(compObj);
  compInstance.CreateInstanceAndBindReferences(std::make_shared<MockComponentContext>());
  compObj = compInstance.GetInstance();
  ASSERT_FALSE(compObj->IsActivated());
  compInstance.Activate();
  ASSERT_TRUE(compObj->IsActivated());
  compInstance.Deactivate();
  ASSERT_FALSE(compObj->IsActivated());
}
}
