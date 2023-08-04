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

#include "TestFixture.hpp"
#include "TestInterfaces/Interfaces.hpp"
#include "../TestUtils.hpp"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/ServiceEvent.h"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "gtest/gtest.h"

#include <iostream>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace test
{

    class FailedBoundServiceActivationTest : public testing::TestWithParam<std::pair<std::string, std::string>>
    {
      public:
        FailedBoundServiceActivationTest() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {}

        void
        SetUp() override
        {
            framework.Start();
            auto context = framework.GetBundleContext();

            ::test::InstallAndStartDS(context);

            auto sRef = context.GetServiceReference<scr::ServiceComponentRuntime>();
            ASSERT_TRUE(sRef);
            dsRuntimeService = context.GetService<scr::ServiceComponentRuntime>(sRef);
            ASSERT_TRUE(dsRuntimeService);
        }

        void
        TearDown() override
        {
            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        }

        std::shared_ptr<scr::ServiceComponentRuntime> dsRuntimeService;
        cppmicroservices::Framework framework;
    };

    INSTANTIATE_TEST_SUITE_P(BoundServices,
                             FailedBoundServiceActivationTest,
                             testing::Values(std::make_pair(std::string("TestBundleDSDependent"),
                                                            std::string("TestBundleDSUpstreamDependencyA")),
                                             std::make_pair(std::string("TestBundleDSDependentNoInject"),
                                                            std::string("TestBundleDSUpstreamDependencyA")),
                                             std::make_pair(std::string("TestBundleDSDependent"),
                                                            std::string("TestBundleDSUpstreamDependencyB")),
                                             std::make_pair(std::string("TestBundleDSDependentNoInject"),
                                                            std::string("TestBundleDSUpstreamDependencyB")),
                                             std::make_pair(std::string("TestBundleDSDependentOptional"),
                                                            std::string("TestBundleDSUpstreamDependencyA")),
                                             std::make_pair(std::string("TestBundleDSDependentOptional"),
                                                            std::string("TestBundleDSUpstreamDependencyB"))));

    TEST_P(FailedBoundServiceActivationTest, TestExceptionOnConstructionOrActivationFailure)
    {
        auto const& params = GetParam();

        auto ctx = framework.GetBundleContext();

        auto bundle = ::test::InstallAndStartBundle(ctx, params.first);
        auto dependency = ::test::InstallAndStartBundle(ctx, params.second);

        auto sDepRef = ctx.GetServiceReference<test::TestBundleDSDependent>();
        auto depService = ctx.GetService<test::TestBundleDSDependent>(sDepRef);

        if (params.first == "TestBundleDSDependentOptional")
        {
            // The dependent service should not be null since the cardinality of
            // TestBundleDSDependentOptional's cardinality for its reference is 0..1
            EXPECT_NE(depService, nullptr);
        }
        else
        {
            // The dependent service should be null since the upstream dependency failed
            // to construct
            EXPECT_EQ(depService, nullptr);
        }

        auto sUpstreamRef = ctx.GetServiceReference<test::TestBundleDSUpstreamDependency>();
        auto upstreamService = ctx.GetService<test::TestBundleDSUpstreamDependency>(sUpstreamRef);

        // The upstream service should be null since it throws in the constructor
        EXPECT_EQ(upstreamService, nullptr);

        auto bundleDescriptionDTO
            = dsRuntimeService->GetComponentDescriptionDTO(bundle, "dependent::" + params.first + "Impl");

        auto configDTO = dsRuntimeService->GetComponentConfigurationDTOs(bundleDescriptionDTO);

        EXPECT_EQ(configDTO.size(), 1);

        // The number of unsatisfied references should be 1 since the upstream reference
        // failed to construct
        auto const& unsatisfiedRefs = configDTO[0].unsatisfiedReferences;
        EXPECT_EQ(unsatisfiedRefs.size(), 0);

        // The number of satisfied references should be 0 since the upstream reference failed
        // to construct
        auto const& satisfiedRefs = configDTO[0].satisfiedReferences;
        EXPECT_EQ(satisfiedRefs.size(), 1);

        if (params.first == "TestBundleDSDependentOptional")
        {
            // The state of the bundle should be ACTIVE since the sole dependency
            // of the service failed to construct but the cardinality is 0..1
            EXPECT_EQ(configDTO[0].state, cppmicroservices::service::component::runtime::dto::ComponentState::ACTIVE);
        }
        else
        {
            // The state of the bundle should be SATISFIED since the sole dependency
            // of the service failed to construct and the cardinality is 1..1
            EXPECT_EQ(configDTO[0].state,
                      cppmicroservices::service::component::runtime::dto::ComponentState::SATISFIED);
        }
    }
} // namespace test
