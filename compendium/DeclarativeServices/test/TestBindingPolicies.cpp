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
#include <tuple>
#include <typeindex>
#include <typeinfo>

#include "../src/manager/ReferenceManagerImpl.hpp"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp"

#include "../src/SCRLogger.hpp"
#include "ConcurrencyTestUtil.hpp"
#include "Mocks.hpp"
#include "TestInterfaces/Interfaces.hpp"
#include "TestUtils.hpp"

namespace scr = cppmicroservices::service::component::runtime;

namespace
{
    // convenience function to get the SCR service
    std::shared_ptr<scr::ServiceComponentRuntime>
    GetServiceComponentRuntime(cppmicroservices::BundleContext bc)
    {
        auto dsRef = bc.GetServiceReference<scr::ServiceComponentRuntime>();
        EXPECT_TRUE(dsRef);
        auto dsRuntimeService = bc.GetService<scr::ServiceComponentRuntime>(dsRef);
        EXPECT_TRUE(dsRuntimeService);
        return dsRuntimeService;
    }

    // convenience function to test a service component's state
    void
    CheckComponentConfigurationState(std::shared_ptr<scr::ServiceComponentRuntime> dsRuntime,
                                     cppmicroservices::Bundle const& bundle,
                                     const std::string svcComponentName,
                                     const scr::dto::ComponentState compState)
    {
        auto compDescDTO = dsRuntime->GetComponentDescriptionDTO(bundle, svcComponentName);
        auto compConfigDTOs = dsRuntime->GetComponentConfigurationDTOs(compDescDTO);
        EXPECT_EQ(compConfigDTOs.size(), 1ul);
        EXPECT_EQ(compConfigDTOs.at(0).state, compState);
    }
} // namespace

namespace test
{
    class InterfaceImpl : public Interface1
    {
      public:
        InterfaceImpl(std::string str) : str_(std::move(str)) {}
        virtual ~InterfaceImpl() = default;
        virtual std::string
        Description()
        {
            return str_;
        }

      private:
        std::string str_;
    };
} // namespace test

namespace cppmicroservices
{
    namespace scrimpl
    {

        struct Policy
        {
            char const* policy;
            char const* policyOption;
            std::type_index policyType;

            friend std::ostream&
            operator<<(std::ostream& os, Policy const& obj)
            {
                return os << "Policy: " << obj.policy << "\n"
                          << "Policy option: " << obj.policyOption << "\n"
                          << "Policy type: " << obj.policyType.name() << "\n";
            }
        };

        class BindingPolicyTest : public ::testing::TestWithParam<Policy>
        {
          protected:
            BindingPolicyTest() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {}

            virtual ~BindingPolicyTest() = default;

            virtual void
            SetUp()
            {
                framework.Start();
            }

            virtual void
            TearDown()
            {
                framework.Stop();
                framework.WaitForStop(std::chrono::milliseconds::zero());
            }

            cppmicroservices::Framework&
            GetFramework()
            {
                return framework;
            }

          private:
            cppmicroservices::Framework framework;
        };

        struct DynamicRefPolicy
        {
            char const* bundleFileName;
            char const* verificationMessage;
            char const* verificationMessageAfterUnregister;
            char const* implClassName;
            bool optional;
            InterfaceMapConstPtr interfaceMap;

            friend std::ostream&
            operator<<(std::ostream& os, DynamicRefPolicy const& obj)
            {
                return os << "Test Bundle Filename: " << obj.bundleFileName << "\n"
                          << "Expected Verification Message: " << obj.verificationMessage << "\n"
                          << "Expected Verification Message after unregistering a "
                             "service: "
                          << obj.verificationMessageAfterUnregister << "\n"
                          << "Service implementation class name: " << obj.implClassName << "\n"
                          << "Is the reference optional?: " << ((obj.optional) ? "Yes" : "No") << "\n";
            }
        };

        class DynamicRefPolicyTest : public ::testing::TestWithParam<DynamicRefPolicy>
        {
          protected:
            DynamicRefPolicyTest() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {}

            virtual ~DynamicRefPolicyTest() = default;

            virtual void
            SetUp()
            {
                framework.Start();
            }

            virtual void
            TearDown()
            {
                framework.Stop();
                framework.WaitForStop(std::chrono::milliseconds::zero());
            }

            cppmicroservices::Framework&
            GetFramework()
            {
                return framework;
            }

          private:
            cppmicroservices::Framework framework;
        };

        // utility method for creating different types of reference metadata objects used in testing
        metadata::ReferenceMetadata
        CreateFakeReferenceMetadata(std::string const& policy, std::string const& policyOption)
        {
            metadata::ReferenceMetadata fakeMetadata {};
            fakeMetadata.name = "ref";
            fakeMetadata.interfaceName = us_service_interface_iid<dummy::Reference1>();
            fakeMetadata.policy = policy;
            fakeMetadata.policyOption = policyOption;
            fakeMetadata.cardinality = "0..1";
            fakeMetadata.minCardinality = 0;
            fakeMetadata.maxCardinality = 1;
            return fakeMetadata;
        }

        using namespace cppmicroservices::scrimpl;

        INSTANTIATE_TEST_SUITE_P(
            BindingPolicies,
            BindingPolicyTest,
            testing::Values(
                Policy { "static", "greedy", typeid(ReferenceManagerBaseImpl::BindingPolicyStaticGreedy) },
                Policy { "static", "reluctant", typeid(ReferenceManagerBaseImpl::BindingPolicyStaticReluctant) },
                Policy { "dynamic", "greedy", typeid(ReferenceManagerBaseImpl::BindingPolicyDynamicGreedy) },
                Policy { "dynamic", "reluctant", typeid(ReferenceManagerBaseImpl::BindingPolicyDynamicReluctant) }));

        TEST_P(BindingPolicyTest, TestPolicyCreation)
        {
            auto bc = GetFramework().GetBundleContext();
            auto const& param = GetParam();

            auto fakeMetadata = CreateFakeReferenceMetadata(param.policy, param.policyOption);
            auto fakeLogger = std::make_shared<FakeLogger>();
            auto mgr = std::make_shared<MockReferenceManagerBaseImpl>(fakeMetadata, bc, fakeLogger, "foo");

            auto bindingPolicy
                = ReferenceManagerBaseImpl::CreateBindingPolicy(*mgr, fakeMetadata.policy, fakeMetadata.policyOption);
            EXPECT_TRUE(bindingPolicy);
            auto* bindingPolicyData = bindingPolicy.get();

            EXPECT_EQ(param.policyType, typeid(*bindingPolicyData));
        }

        TEST_P(BindingPolicyTest, InvalidServiceReference)
        {
            auto bc = GetFramework().GetBundleContext();
            auto const& param = GetParam();
            auto fakeMetadata = CreateFakeReferenceMetadata(param.policy, param.policyOption);
            auto mockLogger = std::make_shared<MockLogger>();
            auto mgr = std::make_shared<MockReferenceManagerBaseImpl>(fakeMetadata, bc, mockLogger, "foo");

            auto bindingPolicy
                = ReferenceManagerBaseImpl::CreateBindingPolicy(*mgr, fakeMetadata.policy, fakeMetadata.policyOption);

            EXPECT_CALL(*mockLogger.get(), Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG, testing::_))
                .Times(1);

            EXPECT_NO_THROW(bindingPolicy->ServiceAdded(ServiceReferenceU()));
        }

        INSTANTIATE_TEST_SUITE_P(
            DynamicReferencePolicies,
            DynamicRefPolicyTest,
            testing::Values(DynamicRefPolicy { "TestBundleDSDRMU",
                                               "ServiceComponentDynamicReluctantMandatoryUnary depends on "
                                               "ServiceComponentDynamicReluctantMandatoryUnary Interface1",
                                               "",
                                               "sample::ServiceComponentDynamicReluctantMandatoryUnary",
                                               false,
                                               MakeInterfaceMap<test::Interface1>(std::make_shared<test::InterfaceImpl>(
                                                   "ServiceComponentDynamicReluctantMandatoryUnary Interface1")) },
                            DynamicRefPolicy { "TestBundleDSDGMU",
                                               "ServiceComponentDynamicGreedyMandatoryUnary depends on "
                                               "ServiceComponentDynamicGreedyMandatoryUnary Interface1",
                                               "",
                                               "sample::ServiceComponentDynamicGreedyMandatoryUnary",
                                               false,
                                               MakeInterfaceMap<test::Interface1>(std::make_shared<test::InterfaceImpl>(
                                                   "ServiceComponentDynamicGreedyMandatoryUnary Interface1")) },
                            DynamicRefPolicy { "TestBundleDSDROU",
                                               "ServiceComponentDynamicReluctantOptionalUnary depends on "
                                               "ServiceComponentDynamicReluctantOptionalUnary Interface1",
                                               "ServiceComponentDynamicReluctantOptionalUnary depends on ",
                                               "sample::ServiceComponentDynamicReluctantOptionalUnary",
                                               true,
                                               MakeInterfaceMap<test::Interface1>(std::make_shared<test::InterfaceImpl>(
                                                   "ServiceComponentDynamicReluctantOptionalUnary Interface1")) },
                            DynamicRefPolicy { "TestBundleDSDGOU",
                                               "ServiceComponentDynamicGreedyOptionalUnary depends on "
                                               "ServiceComponentDynamicGreedyOptionalUnary Interface1",
                                               "ServiceComponentDynamicGreedyOptionalUnary depends on ",
                                               "sample::ServiceComponentDynamicGreedyOptionalUnary",
                                               true,
                                               MakeInterfaceMap<test::Interface1>(std::make_shared<test::InterfaceImpl>(
                                                   "ServiceComponentDynamicGreedyOptionalUnary Interface1")) }));

        // test binding a service under the following reference policy, reference policy options and cardinality
        // Cardinality: 0..1, 1..1
        // reference policy: dynamic
        // reference policy options: reluctant, greedy
        TEST_P(DynamicRefPolicyTest, TestBindingWithDynamicPolicyOptions)
        {
            auto bc = GetFramework().GetBundleContext();
            test::InstallAndStartDS(bc);

            auto const& param = GetParam();

            auto testBundle = test::InstallAndStartBundle(bc, param.bundleFileName);

            auto dsRuntimeService = GetServiceComponentRuntime(bc);

            if (param.optional)
            {
                EXPECT_TRUE(bc.GetServiceReference<test::Interface2>())
                    << "Service must be available before it's dependency because the "
                       "dependency is optional";
            }
            else
            {
                EXPECT_FALSE(bc.GetServiceReference<test::Interface2>())
                    << "Service must not be available before it's dependency";
            }

            CheckComponentConfigurationState(dsRuntimeService,
                                             testBundle,
                                             param.implClassName,
                                             ((param.optional) ? scr::dto::ComponentState::ACTIVE
                                                               : scr::dto::ComponentState::UNSATISFIED_REFERENCE));

            // register the dependent service to trigger the bind
            auto depSvcReg = bc.RegisterService(param.interfaceMap);
            ASSERT_TRUE(depSvcReg);

            CheckComponentConfigurationState(dsRuntimeService,
                                             testBundle,
                                             param.implClassName,
                                             scr::dto::ComponentState::ACTIVE);

            auto svcRef = bc.GetServiceReference<test::Interface2>();
            ASSERT_TRUE(svcRef);
            auto svc = bc.GetService<test::Interface2>(svcRef);
            ASSERT_TRUE(svc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ(param.verificationMessage, svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // unregister the service dependency and test the depedent service
            depSvcReg.Unregister();
            if (param.optional)
            {
                //  optional service dependencies mean that the service is still available and (typically) usable.
                EXPECT_TRUE(bc.GetServiceReference<test::Interface2>()) << "Service should NOT be available";
                EXPECT_NO_THROW(svc->ExtendedDescription());
                EXPECT_STREQ(param.verificationMessageAfterUnregister, svc->ExtendedDescription().c_str())
                    << "String value returned was not expected. Was the correct service "
                       "dependency bound?";
            }
            else
            {
                EXPECT_FALSE(bc.GetServiceReference<test::Interface2>()) << "Service should NOT be available";
                EXPECT_THROW(svc->ExtendedDescription(), std::runtime_error);
            }
            testBundle.Stop();
        }

        // test error handling and logging when the bind and unbind methods throw an exception
        // this test ensures that exception handling when binding and unbinding for both
        // 0..1 and 1..1 cardinalities works.
        //
        // According to OSGi Compendium Release 7 Sections 112.5.10 and 112.5.18, if a
        // bind/unbind method throws the activate/deactivation of the component configuration
        // does not fail. This indicates that the service reference and service objects are
        // both valid and usable by service consumers.
        //
        TEST_F(BindingPolicyTest, TestDynamicBindUnBindExceptionHandling)
        {
            auto bc = GetFramework().GetBundleContext();
            auto mockLogger = std::make_shared<MockLogger>();
            auto dsLoggerSvcReg = bc.RegisterService<cppmicroservices::logservice::LogService>(mockLogger);

            test::InstallAndStartDS(bc);

            // The expectation is that Log(...) with a log severity of LOG_ERROR will be called
            // exactly 10 times - 1 bind and 1 unbind per service component of which there are three,
            // plus an additional bind and unbind for each service component with optional cardinality
            // of which there are two.
            EXPECT_CALL(*mockLogger.get(),
                        Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, testing::_, testing::_))
                .Times(10);

            auto testBundle = test::InstallAndStartBundle(bc, "TestBindUnbindThrows");
            EXPECT_TRUE(bc.GetServiceReference<test::Interface2>())
                << "Service must be available before it's dependency";

            auto dsRuntimeService = GetServiceComponentRuntime(bc);

            CheckComponentConfigurationState(dsRuntimeService,
                                             testBundle,
                                             "sample::ServiceComponentDGMU",
                                             scr::dto::ComponentState::UNSATISFIED_REFERENCE);

            CheckComponentConfigurationState(dsRuntimeService,
                                             testBundle,
                                             "sample::ServiceComponentDGOU",
                                             scr::dto::ComponentState::ACTIVE);

            // trigger the bind to be called.
            auto depSvcReg = bc.RegisterService<test::Interface1>(std::make_shared<test::InterfaceImpl>("Interface1"));
            ASSERT_TRUE(depSvcReg);

            CheckComponentConfigurationState(dsRuntimeService,
                                             testBundle,
                                             "sample::ServiceComponentDGMU",
                                             scr::dto::ComponentState::ACTIVE);

            CheckComponentConfigurationState(dsRuntimeService,
                                             testBundle,
                                             "sample::ServiceComponentDGOU",
                                             scr::dto::ComponentState::ACTIVE);

            auto svcRef = bc.GetServiceReference<test::Interface2>();
            ASSERT_TRUE(svcRef);
            auto svc = bc.GetService<test::Interface2>(svcRef);
            ASSERT_TRUE(svc);
            ASSERT_THROW(svc->ExtendedDescription(), std::runtime_error);

            // trigger the unbind to be called.
            depSvcReg.Unregister();
            EXPECT_TRUE(bc.GetServiceReference<test::Interface2>()) << "Service should be available";

            testBundle.Stop();
        }

        // test that:
        //  a new higher ranked service causes a re-bind
        //  a new lower ranked service does not cause a re-bind
        //  unregistering services causes correct re-binds
        TEST_F(BindingPolicyTest, TestDynamicGreedyMandatoryUnaryReBind)
        {
            auto bc = GetFramework().GetBundleContext();
            test::InstallAndStartDS(bc);

            auto testBundle = test::InstallAndStartBundle(bc, "TestBundleDSDGMU");
            EXPECT_FALSE(bc.GetServiceReference<test::Interface2>())
                << "Service must not be available before it's dependency";

            auto dsRuntimeService = GetServiceComponentRuntime(bc);

            CheckComponentConfigurationState(dsRuntimeService,
                                             testBundle,
                                             "sample::ServiceComponentDynamicGreedyMandatoryUnary",
                                             scr::dto::ComponentState::UNSATISFIED_REFERENCE);

            // register the dependent service to trigger a bind
            auto depSvcReg = bc.RegisterService<test::Interface1>(
                std::make_shared<test::InterfaceImpl>("ServiceComponentDynamicGreedyMandatoryUnary Interface1"));
            ASSERT_TRUE(depSvcReg);

            CheckComponentConfigurationState(dsRuntimeService,
                                             testBundle,
                                             "sample::ServiceComponentDynamicGreedyMandatoryUnary",
                                             scr::dto::ComponentState::ACTIVE);

            auto svcRef = bc.GetServiceReference<test::Interface2>();
            ASSERT_TRUE(svcRef);
            auto svc = bc.GetService<test::Interface2>(svcRef);
            ASSERT_TRUE(svc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicGreedyMandatoryUnary depends on "
                         "ServiceComponentDynamicGreedyMandatoryUnary Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // registering a new service with a higher rank should cause a re-binding and use of the new service
            auto higherRankedSvc = bc.RegisterService<test::Interface1>(
                std::make_shared<test::InterfaceImpl>("higher ranked Interface1"),
                {
                    {Constants::SERVICE_RANKING, Any(10000)}
            });
            ASSERT_TRUE(higherRankedSvc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicGreedyMandatoryUnary depends on higher "
                         "ranked Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // registering a new service with a lower rank should NOT cause re-binding and use of the new service
            auto lowerRankedSvc
                = bc.RegisterService<test::Interface1>(std::make_shared<test::InterfaceImpl>("lower ranked Interface1"),
                                                       {
                                                           {Constants::SERVICE_RANKING, Any(1)}
            });
            ASSERT_TRUE(lowerRankedSvc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicGreedyMandatoryUnary depends on higher "
                         "ranked Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // unregistering the higher ranked service should cause a re-bind to the lower ranked service.
            higherRankedSvc.Unregister();
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicGreedyMandatoryUnary depends on lower "
                         "ranked Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // unregistering the lower ranked service should cause a re-bind to the last registered service.
            lowerRankedSvc.Unregister();
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicGreedyMandatoryUnary depends on "
                         "ServiceComponentDynamicGreedyMandatoryUnary Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // unregistering the last service now causes the dependent service to be unregistered.
            depSvcReg.Unregister();
            EXPECT_FALSE(bc.GetServiceReference<test::Interface2>()) << "Service should NOT be available";
            EXPECT_THROW(svc->ExtendedDescription(), std::runtime_error);
            testBundle.Stop();
        }

        // test that:
        //  a new higher ranked service causes a re-bind
        //  a new lower ranked service does not cause a re-bind
        //  unregistering services causes correct re-binds
        TEST_F(BindingPolicyTest, TestDynamicGreedyOptionalUnaryReBind)
        {
            auto bc = GetFramework().GetBundleContext();
            test::InstallAndStartDS(bc);

            auto testBundle = test::InstallAndStartBundle(bc, "TestBundleDSDGOU");
            EXPECT_TRUE(bc.GetServiceReference<test::Interface2>())
                << "Service should be available before it's dependency";

            auto dsRuntimeService = GetServiceComponentRuntime(bc);

            CheckComponentConfigurationState(dsRuntimeService,
                                             testBundle,
                                             "sample::ServiceComponentDynamicGreedyOptionalUnary",
                                             scr::dto::ComponentState::ACTIVE);

            auto svcRef = bc.GetServiceReference<test::Interface2>();
            ASSERT_TRUE(svcRef);
            auto svc = bc.GetService<test::Interface2>(svcRef);
            ASSERT_TRUE(svc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicGreedyOptionalUnary depends on ", svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // register the dependent service to trigger the re-bind
            auto depSvcReg = bc.RegisterService<test::Interface1>(
                std::make_shared<test::InterfaceImpl>("ServiceComponentDynamicGreedyOptionalUnary Interface1"));
            ASSERT_TRUE(depSvcReg);

            CheckComponentConfigurationState(dsRuntimeService,
                                             testBundle,
                                             "sample::ServiceComponentDynamicGreedyOptionalUnary",
                                             scr::dto::ComponentState::ACTIVE);

            svcRef = bc.GetServiceReference<test::Interface2>();
            ASSERT_TRUE(svcRef);
            svc = bc.GetService<test::Interface2>(svcRef);
            ASSERT_TRUE(svc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicGreedyOptionalUnary depends on "
                         "ServiceComponentDynamicGreedyOptionalUnary Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // registering a new service with a higher rank should cause a re-binding and use of the new service
            auto higherRankedSvc = bc.RegisterService<test::Interface1>(
                std::make_shared<test::InterfaceImpl>("higher ranked Interface1"),
                {
                    {Constants::SERVICE_RANKING, Any(10000)}
            });
            ASSERT_TRUE(higherRankedSvc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicGreedyOptionalUnary depends on higher "
                         "ranked Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // registering a new service with a lower rank should NOT cause re-binding and use of the new service
            auto lowerRankedSvc
                = bc.RegisterService<test::Interface1>(std::make_shared<test::InterfaceImpl>("lower ranked Interface1"),
                                                       {
                                                           {Constants::SERVICE_RANKING, Any(1)}
            });
            ASSERT_TRUE(lowerRankedSvc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicGreedyOptionalUnary depends on higher "
                         "ranked Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // unregistering the higher ranked service should cause a re-bind to the lower ranked service.
            higherRankedSvc.Unregister();
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicGreedyOptionalUnary depends on lower "
                         "ranked Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // unregistering the lower ranked service should cause a re-bind to the last registered service.
            lowerRankedSvc.Unregister();
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicGreedyOptionalUnary depends on "
                         "ServiceComponentDynamicGreedyOptionalUnary Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // unregistering the last service now causes the dependent service to be unregistered.
            depSvcReg.Unregister();
            EXPECT_TRUE(bc.GetServiceReference<test::Interface2>()) << "Service should be available";
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicGreedyOptionalUnary depends on ", svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";
            testBundle.Stop();
        }

        // test that binding happens only once for dynamic reluctant reference policy
        TEST_F(BindingPolicyTest, TestDynamicReluctantMandatoryUnaryReBind)
        {
            auto bc = GetFramework().GetBundleContext();
            test::InstallAndStartDS(bc);

            auto testBundle = test::InstallAndStartBundle(bc, "TestBundleDSDRMU");
            EXPECT_FALSE(bc.GetServiceReference<test::Interface2>())
                << "Service must not be available before it's dependency";

            auto dsRuntimeService = GetServiceComponentRuntime(bc);

            CheckComponentConfigurationState(dsRuntimeService,
                                             testBundle,
                                             "sample::ServiceComponentDynamicReluctantMandatoryUnary",
                                             scr::dto::ComponentState::UNSATISFIED_REFERENCE);

            // register the dependent service to trigger the bind
            auto depSvcReg = bc.RegisterService<test::Interface1>(
                std::make_shared<test::InterfaceImpl>("ServiceComponentDynamicReluctantMandatoryUnary Interface1"));
            ASSERT_TRUE(depSvcReg);

            CheckComponentConfigurationState(dsRuntimeService,
                                             testBundle,
                                             "sample::ServiceComponentDynamicReluctantMandatoryUnary",
                                             scr::dto::ComponentState::ACTIVE);

            auto svcRef = bc.GetServiceReference<test::Interface2>();
            ASSERT_TRUE(svcRef);
            auto svc = bc.GetService<test::Interface2>(svcRef);
            ASSERT_TRUE(svc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicReluctantMandatoryUnary depends on "
                         "ServiceComponentDynamicReluctantMandatoryUnary Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // registering a new service with a higher rank should not cause re-binding
            auto higherRankedSvc = bc.RegisterService<test::Interface1>(
                std::make_shared<test::InterfaceImpl>("higher ranked Interface1"),
                {
                    {Constants::SERVICE_RANKING, Any(10000)}
            });
            ASSERT_TRUE(higherRankedSvc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicReluctantMandatoryUnary depends on "
                         "ServiceComponentDynamicReluctantMandatoryUnary Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // registering a new service with a lower rank should NOT cause re-binding
            auto lowerRankedSvc
                = bc.RegisterService<test::Interface1>(std::make_shared<test::InterfaceImpl>("lower ranked Interface1"),
                                                       {
                                                           {Constants::SERVICE_RANKING, Any(1)}
            });
            ASSERT_TRUE(lowerRankedSvc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicReluctantMandatoryUnary depends on "
                         "ServiceComponentDynamicReluctantMandatoryUnary Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // unregistering the bound service should cause a re-bind to the higher ranked service.
            depSvcReg.Unregister();
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicReluctantMandatoryUnary depends on higher "
                         "ranked Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // unregistering the higher ranked service should cause a re-bind to the lower ranked service.
            higherRankedSvc.Unregister();
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicReluctantMandatoryUnary depends on "
                         "lower ranked Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // unregistering the lower ranked service now causes the dependent service to be unregistered.
            lowerRankedSvc.Unregister();
            EXPECT_FALSE(bc.GetServiceReference<test::Interface2>()) << "Service should NOT be available";
            EXPECT_THROW(svc->ExtendedDescription(), std::runtime_error);
            testBundle.Stop();
        }

        // test that binding happens only once for dynamic reluctant reference policy
        TEST_F(BindingPolicyTest, TestDynamicReluctantOptionalUnaryReBind)
        {
            auto bc = GetFramework().GetBundleContext();
            test::InstallAndStartDS(bc);

            auto testBundle = test::InstallAndStartBundle(bc, "TestBundleDSDROU");
            EXPECT_TRUE(bc.GetServiceReference<test::Interface2>())
                << "Service should be available before it's dependency";

            auto dsRuntimeService = GetServiceComponentRuntime(bc);

            CheckComponentConfigurationState(dsRuntimeService,
                                             testBundle,
                                             "sample::ServiceComponentDynamicReluctantOptionalUnary",
                                             scr::dto::ComponentState::ACTIVE);

            auto svcRef = bc.GetServiceReference<test::Interface2>();
            ASSERT_TRUE(svcRef);
            auto svc = bc.GetService<test::Interface2>(svcRef);
            ASSERT_TRUE(svc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicReluctantOptionalUnary depends on ",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // register the dependent service to trigger a bind
            auto depSvcReg = bc.RegisterService<test::Interface1>(
                std::make_shared<test::InterfaceImpl>("ServiceComponentDynamicReluctantOptionalUnary Interface1"));
            ASSERT_TRUE(depSvcReg);

            CheckComponentConfigurationState(dsRuntimeService,
                                             testBundle,
                                             "sample::ServiceComponentDynamicReluctantOptionalUnary",
                                             scr::dto::ComponentState::ACTIVE);

            svcRef = bc.GetServiceReference<test::Interface2>();
            ASSERT_TRUE(svcRef);
            svc = bc.GetService<test::Interface2>(svcRef);
            ASSERT_TRUE(svc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicReluctantOptionalUnary depends on "
                         "ServiceComponentDynamicReluctantOptionalUnary Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // registering a new service with a higher rank should not cause re-binding
            auto higherRankedSvc = bc.RegisterService<test::Interface1>(
                std::make_shared<test::InterfaceImpl>("higher ranked Interface1"),
                {
                    {Constants::SERVICE_RANKING, Any(10000)}
            });
            ASSERT_TRUE(higherRankedSvc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicReluctantOptionalUnary depends on "
                         "ServiceComponentDynamicReluctantOptionalUnary Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // registering a new service with a lower rank should NOT cause re-binding
            auto lowerRankedSvc
                = bc.RegisterService<test::Interface1>(std::make_shared<test::InterfaceImpl>("lower ranked Interface1"),
                                                       {
                                                           {Constants::SERVICE_RANKING, Any(1)}
            });
            ASSERT_TRUE(lowerRankedSvc);
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicReluctantOptionalUnary depends on "
                         "ServiceComponentDynamicReluctantOptionalUnary Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // unregistering the higher ranked service should not cause a re-bind to the lower ranked service
            // since the lower ranked service wasn't bound.
            higherRankedSvc.Unregister();
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicReluctantOptionalUnary depends on "
                         "ServiceComponentDynamicReluctantOptionalUnary Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // unregistering the lower ranked service should not cause a re-bind since the lower ranked service
            // was not the bound service.
            lowerRankedSvc.Unregister();
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicReluctantOptionalUnary depends on "
                         "ServiceComponentDynamicReluctantOptionalUnary Interface1",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";

            // unregistering the last service should still cause the service component to be satisfied since
            // the reference cardinality is optional.
            depSvcReg.Unregister();
            EXPECT_TRUE(bc.GetServiceReference<test::Interface2>()) << "Service should NOT be available";
            EXPECT_NO_THROW(svc->ExtendedDescription());
            EXPECT_STREQ("ServiceComponentDynamicReluctantOptionalUnary depends on ",
                         svc->ExtendedDescription().c_str())
                << "String value returned was not expected. Was the correct service "
                   "dependency bound?";
            testBundle.Stop();
        }

        // test that concurrent service registrations and unregistrations
        // do not cause crashes.
        TEST_F(BindingPolicyTest, TestConcurrentBindUnbind)
        {
            auto bc = GetFramework().GetBundleContext();
            test::InstallAndStartDS(bc);

            auto testBundle = test::InstallAndStartBundle(bc, "TestBundleDSDRMU");
            EXPECT_FALSE(bc.GetServiceReference<test::Interface2>())
                << "Service must not be available before it's dependency";

            auto dsRuntimeService = GetServiceComponentRuntime(bc);

            std::function<bool()> func = [&bc]() -> bool
            {
                // register the dependent service to trigger the bind
                auto depSvcReg = bc.RegisterService<test::Interface1>(
                    std::make_shared<test::InterfaceImpl>("ServiceComponentDynamicReluctantMandatoryUnary Interface1"));

                // registering a new service with a higher rank should not cause re-binding
                auto higherRankedSvc = bc.RegisterService<test::Interface1>(
                    std::make_shared<test::InterfaceImpl>("higher ranked Interface1"),
                    {
                        {Constants::SERVICE_RANKING, Any(10000)}
                });

                // registering a new service with a lower rank should NOT cause re-binding
                auto lowerRankedSvc = bc.RegisterService<test::Interface1>(
                    std::make_shared<test::InterfaceImpl>("lower ranked Interface1"),
                    {
                        {Constants::SERVICE_RANKING, Any(1)}
                });

                depSvcReg.Unregister();
                higherRankedSvc.Unregister();
                lowerRankedSvc.Unregister();
                return true;
            };

            auto results = ConcurrentInvoke(func);
            EXPECT_TRUE(!results.empty());
            EXPECT_TRUE(std::all_of(results.cbegin(), results.cend(), [](bool result) { return result; }));
            testBundle.Stop();
        }

    } // namespace scrimpl
} // namespace cppmicroservices
