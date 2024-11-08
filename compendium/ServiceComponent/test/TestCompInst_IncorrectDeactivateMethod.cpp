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

// #include "version.h"

#include "gtest/gtest.h"

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp"
#include "cppmicroservices/servicecomponent/UserDefinedMethodAssertion.hpp"
#include <cppmicroservices/ServiceInterface.h>

using cppmicroservices::service::component::detail::ComponentInstanceImpl;
using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace
{
    struct TestServiceInterface1
    {
    };

    class ServiceComponentWrongDeactivateSig final
        : public TestServiceInterface1
        , public cppmicroservices::service::component::UserDefinedMethodAssertion<ServiceComponentWrongDeactivateSig>
    {
      public:
        ServiceComponentWrongDeactivateSig() = default;
        ~ServiceComponentWrongDeactivateSig() = default;

        void
        Modified(std::shared_ptr<ComponentContext> const& /*context*/,
                 std::shared_ptr<cppmicroservices::AnyMap> const& /*configuration*/)
        {
        }
        void
        Activate(std::shared_ptr<ComponentContext> const& /*context*/)
        {
        }
    };

    TEST(ComponentInstance, ValidateActivateMethod)
    {
        ComponentInstanceImpl<ServiceComponentWrongDeactivateSig,
                              std::tuple<TestServiceInterface1,
                                         cppmicroservices::service::component::UserDefinedMethodAssertion<
                                             ServiceComponentWrongDeactivateSig>>>
            compInstance; // compile error
    }
} // namespace

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
