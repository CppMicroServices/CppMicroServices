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
#if NEVER
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <chrono>

#include "gtest/gtest.h"

#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp"
#include <cppmicroservices/ServiceInterface.h>

using cppmicroservices::service::component::detail::ComponentInstanceImpl;
using cppmicroservices::service::component::detail::Binder;
using cppmicroservices::service::component::detail::StaticBinder;
using cppmicroservices::service::component::detail::DynamicBinder;

namespace {

  // dummy types used for testing
  struct ServiceDependency1 {};
  struct ServiceDependency2 {};

  // Test class to simulate a servcie component
  class TestServiceImpl1 final {
  public:
    TestServiceImpl1() = default;

    TestServiceImpl1(const std::shared_ptr<ServiceDependency1>& f,
                const std::shared_ptr<ServiceDependency2>& b)
    : foo(f)
    , bar(b)
    { }

    virtual ~TestServiceImpl1() = default;

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

    std::shared_ptr<ServiceDependency1> GetFoo() const { return foo; }
    std::shared_ptr<ServiceDependency2> GetBar() const { return bar; }
  private:
    std::shared_ptr<ServiceDependency1> foo;        // dynamic dependency - can change during the lifetime of this object
    const std::shared_ptr<ServiceDependency2> bar;  // static dependency - does not change during the lifetime of this object
  };

  /**
   * This test point is used to verify a compile error is generated when the number of dependencies specified in
   * the service component description does not match the dependencies in the service component implementation
   * class constructor.
   *
   * @todo Automate this test point. Currently interactive is the only way to verify compilation failures.
   */
  TEST(ComponentInstance, VerifyDependencyCount)
  {
    std::vector<std::shared_ptr<Binder<TestServiceImpl1>>> binders;
    binders.push_back(std::make_shared<DynamicBinder<TestServiceImpl1, ServiceDependency1>>("foo", &TestServiceImpl1::BindFoo, &TestServiceImpl1::UnbindFoo));
    binders.push_back(std::make_shared<StaticBinder<TestServiceImpl1, ServiceDependency2>>("bar"));
    binders.push_back(std::make_shared<StaticBinder<TestServiceImpl1, ServiceDependency2>>("bar2"));
    ComponentInstanceImpl<TestServiceImpl1, std::tuple<>, std::true_type, ServiceDependency1, ServiceDependency2, ServiceDependency2> compInstance(binders); // compile error
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif
