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

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>

#include "gtest/gtest.h"

#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp"
#include <cppmicroservices/ServiceInterface.h>

using cppmicroservices::service::component::detail::Binder;
using cppmicroservices::service::component::detail::ComponentInstanceImpl;
using cppmicroservices::service::component::detail::DynamicBinder;
using cppmicroservices::service::component::detail::StaticBinder;

namespace
{

    // dummy types used for testing
    struct ServiceDependency1
    {
    };
    struct ServiceDependency2
    {
    };

    // Test class to simulate a service component
    class TestServiceImpl1 final
    {
      public:
        TestServiceImpl1() = default;

        TestServiceImpl1(std::shared_ptr<ServiceDependency1> const& f, std::shared_ptr<ServiceDependency2> const& b)
            : foo(f)
            , bar(b)
        {
        }

        virtual ~TestServiceImpl1() = default;

        void
        BindFoo(std::shared_ptr<ServiceDependency1> const& f)
        {
            foo = f;
        }

        void
        UnbindFoo(std::shared_ptr<ServiceDependency1> const& f)
        {
            if (foo == f)
            {
                foo = nullptr;
            }
        }

        std::shared_ptr<ServiceDependency1>
        GetFoo() const
        {
            return foo;
        }
        std::shared_ptr<ServiceDependency2>
        GetBar() const
        {
            return bar;
        }

      private:
        std::shared_ptr<ServiceDependency1> foo; // dynamic dependency - can change during the lifetime of this object
        const std::shared_ptr<ServiceDependency2>
            bar; // static dependency - does not change during the lifetime of this object
    };

    /**
     * This test points are used to verify a compile error is generated when the order of dependencies specified in
     * the service component description does not match the dependencies in the service component implementation
     * class constructor.
     *
     * CMake tests will build this file to catch the expected compile time error
     */
    TEST(ComponentInstance, VerifyDependencyOrder)
    {
        std::vector<std::shared_ptr<Binder<TestServiceImpl1>>> binders;
        binders.push_back(std::make_shared<StaticBinder<TestServiceImpl1, ServiceDependency2>>("bar"));
        binders.push_back(
            std::make_shared<DynamicBinder<TestServiceImpl1, ServiceDependency1>>("foo",
                                                                                  &TestServiceImpl1::BindFoo,
                                                                                  &TestServiceImpl1::UnbindFoo));
        ComponentInstanceImpl<TestServiceImpl1,
                              std::tuple<>,
                              std::true_type,
                              std::shared_ptr<ServiceDependency2>,
                              std::shared_ptr<ServiceDependency1>>
            compInstance({ "bar", "foo" }, binders); // compile error
    }
} // namespace

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
