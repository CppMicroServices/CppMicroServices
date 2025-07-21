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
#include "cppmicroservices/ServiceEvent.h"
#include "gtest/gtest.h"

namespace sc = cppmicroservices::service::component;
namespace scr = cppmicroservices::service::component::runtime;

namespace test
{
    /**
     * Verify a component that implements Activate & Deactivate methods receives
     * the callbacks
     */
    TEST_F(tServiceComponent, testLifeCycleHooks) // DS_TOI_10
    {
        auto ctxt = framework.GetBundleContext();
        auto testBundle = StartTestBundle("TestBundleDSTOI10");

        // use Service Registry to validate the component is available
        auto sRef = ctxt.GetServiceReference<test::LifeCycleValidation>();
        ASSERT_TRUE(static_cast<bool>(sRef));
        auto service = ctxt.GetService<test::LifeCycleValidation>(sRef);
        ASSERT_NE(service, nullptr);
        EXPECT_TRUE(service->IsActivated()) << "service component instance must have received the Activate callback";

        // Use DS runtime service to validate the component state
        auto compDesc = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent10");
        auto compConfigs = dsRuntimeService->GetComponentConfigurationDTOs(compDesc);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE);

        // Disable the component which should remove the corresponding configuration
        auto fut = dsRuntimeService->DisableComponent(compDesc);
        EXPECT_NO_THROW(fut.get());
        EXPECT_TRUE(service->IsDeactivated())
            << "service component instance must have received the Deactivate callback";
        compConfigs = dsRuntimeService->GetComponentConfigurationDTOs(compDesc);
        EXPECT_EQ(compConfigs.size(), 0ul) << "No configurations must exist after the component is disbled";

        // stop the bundle
        testBundle.Stop();
        compDesc = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent10");
        EXPECT_EQ(compDesc.name, "") << "Component must not be found after bundle is stopped";
    }

    /**
     * verify state progressions for a component which throws from the lifecycle callbacks
     */
    TEST_F(tServiceComponent, testThrowingLifeCycleHooks) // DS_TOI_9
    {
        auto testBundle = StartTestBundle("TestBundleDSTOI9");
        auto ctxt = framework.GetBundleContext();
        auto sRef = ctxt.GetServiceReference<test::LifeCycleValidation>();
        ASSERT_TRUE(static_cast<bool>(sRef));
        auto service = ctxt.GetService<test::LifeCycleValidation>(sRef);
        EXPECT_EQ(service, nullptr) << "Service object must be nullptr since the service component should have "
                                       "thrown an exception when activated";
        auto compDesc = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent9");
        auto compConfigs = dsRuntimeService->GetComponentConfigurationDTOs(compDesc);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
            << "state must be SATISFIED, and never progresses to ACTIVE";
    }

    /**
     * verify state progressions for a immediate component
     * UNSATISFIED_REFERENCE -> ACTIVE -> UNSATISFIED_REFERENCE
     */
    TEST_F(tServiceComponent, testImmediateComponent_LifeCycle) // DS_TOI_51
    {
        auto testBundle = StartTestBundle("TestBundleDSTOI5");
        auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent5");
        auto compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        EXPECT_EQ(compConfigDTOs.size(), 1ul);
        EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE);
        auto ctxt = framework.GetBundleContext();
        auto sRef = ctxt.GetServiceReference<test::Interface2>();
        EXPECT_FALSE(static_cast<bool>(sRef)) << "Service must not be available before it's dependency";
        auto depBundle = StartTestBundle("TestBundleDSTOI1");

        // wait for the asynchronous task to take effect
        auto result = RepeatTaskUntilOrTimeout(
            [&compConfigDTOs, service = this->dsRuntimeService, &compDescDTO]()
            { compConfigDTOs = service->GetComponentConfigurationDTOs(compDescDTO); },
            [&compConfigDTOs]() -> bool { return compConfigDTOs.at(0).state == scr::dto::ComponentState::ACTIVE; });

        ASSERT_TRUE(result) << "Timed out waiting for state to change to ACTIVE "
                               "after the dependency became available";
        auto sRef1 = ctxt.GetServiceReference<test::Interface2>();
        EXPECT_TRUE(static_cast<bool>(sRef1)) << "Service must be available after it's dependency is available";
        auto service = ctxt.GetService<test::Interface2>(sRef1);

        ASSERT_NE(service, nullptr);
        EXPECT_NO_THROW(service->ExtendedDescription()) << "Throws if the dependency could not be found";
        depBundle.Stop();

        result = RepeatTaskUntilOrTimeout(
            [&compConfigDTOs, service = this->dsRuntimeService, &compDescDTO]()
            { compConfigDTOs = service->GetComponentConfigurationDTOs(compDescDTO); },
            [&compConfigDTOs]() -> bool
            { return compConfigDTOs.at(0).state == scr::dto::ComponentState::UNSATISFIED_REFERENCE; });

        ASSERT_TRUE(result) << "Timed out waiting for state to change to UNSATISFIED_REFERENCE after "
                               "the dependency was removed";
        auto sRef2 = ctxt.GetServiceReference<test::Interface2>();
        EXPECT_FALSE(static_cast<bool>(sRef2)) << "Service must not be available after it's dependency is removed";
    }

    /**
     * verify state progressions for a immediate component
     * UNSATISFIED_REFERENCE -> ACTIVE -> UNSATISFIED_REFERENCE
     */
    TEST_F(tServiceComponent, testImmediateComponent_LifeCycle_Dynamic) // DS_TOI_51
    {
        auto testBundle = StartTestBundle("TestBundleDSTOI7");
        auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent7");
        auto compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        EXPECT_EQ(compConfigDTOs.size(), 1ul);
        EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE);
        auto ctxt = framework.GetBundleContext();
        auto sRef = ctxt.GetServiceReference<test::Interface2>();
        EXPECT_FALSE(static_cast<bool>(sRef)) << "Service must not be available before it's dependency";
        auto depBundle = StartTestBundle("TestBundleDSTOI1");

        // wait for the asynchronous task to take effect
        auto result = RepeatTaskUntilOrTimeout(
            [&compConfigDTOs, service = this->dsRuntimeService, &compDescDTO]()
            { compConfigDTOs = service->GetComponentConfigurationDTOs(compDescDTO); },
            [&compConfigDTOs]() -> bool { return compConfigDTOs.at(0).state == scr::dto::ComponentState::ACTIVE; });

        ASSERT_TRUE(result) << "Timed out waiting for state to change to ACTIVE "
                               "after the dependency became available";
        auto sRef1 = ctxt.GetServiceReference<test::Interface2>();
        EXPECT_TRUE(static_cast<bool>(sRef1)) << "Service must be available after it's dependency is available";
        auto service = ctxt.GetService<test::Interface2>(sRef1);

        ASSERT_NE(service, nullptr);
        EXPECT_NO_THROW(service->ExtendedDescription()) << "Throws if the dependency could not be found";
        depBundle.Stop();
        result = RepeatTaskUntilOrTimeout(
            [&compConfigDTOs, service = this->dsRuntimeService, &compDescDTO]()
            { compConfigDTOs = service->GetComponentConfigurationDTOs(compDescDTO); },
            [&compConfigDTOs]() -> bool
            { return compConfigDTOs.at(0).state == scr::dto::ComponentState::UNSATISFIED_REFERENCE; });
        ASSERT_TRUE(result) << "Timed out waiting for state to change to UNSATISFIED_REFERENCE after "
                               "the dependency was removed";
        auto sRef2 = ctxt.GetServiceReference<test::Interface2>();
        EXPECT_FALSE(static_cast<bool>(sRef2)) << "Service must not be available after it's dependency is removed";
    }

    /**
     * Verify state progressions for a delayed component
     * UNSATISFIED_REFERENCE -> SATISFIED -> ACTIVE -> UNSATISFIED_REFERENCE
     * Verify that the bundle is not loaded into the process before GetService is called.
     * Verify that the bundle is loaded into the process once GetService is called.
     */
    TEST_F(tServiceComponent, testDelayedComponent_LifeCycle) // DS_TOI_52 //DS_TOI_6
    {
        auto testBundle = StartTestBundle("TestBundleDSTOI6");
        auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent6");
        auto compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        EXPECT_EQ(compConfigDTOs.size(), 1ul);
        EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE);
        auto ctxt = framework.GetBundleContext();
        auto sRef = ctxt.GetServiceReference<test::Interface2>();
        EXPECT_FALSE(static_cast<bool>(sRef)) << "Service must not be available before it's dependency";

        auto result = isBundleLoadedInThisProcess("TestBundleDSTOI6");
        EXPECT_FALSE(result) << "library must not be available";

        std::mutex mtx, mtx1;
        std::condition_variable cv, cv1;
        bool serviceRegistered = false;
        bool serviceUnregistered = false;
        // add a listener
        auto token = ctxt.AddServiceListener(
            [&](cppmicroservices::ServiceEvent const& evt)
            {
                //      std::cout << evt << std::endl;
                //      std::cout << GetServiceInterface(evt.GetServiceReference()) << ", " <<
                //      us_service_interface_iid<test::Interface2>() << std::endl;
                if (evt.GetType() == cppmicroservices::ServiceEvent::SERVICE_REGISTERED
                    && evt.GetServiceReference().IsConvertibleTo(us_service_interface_iid<test::Interface2>()))
                {
                    std::lock_guard<std::mutex> lk(mtx);
                    serviceRegistered = true;
                    cv.notify_all();
                }
            });
        auto depBundle = StartTestBundle("TestBundleDSTOI1");
        {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&serviceRegistered]() { return serviceRegistered == true; });
            ctxt.RemoveListener(std::move(token));
        }

        compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::SATISFIED);

        result = isBundleLoadedInThisProcess("TestBundleDSTOI6");
        EXPECT_FALSE(result) << "library must not be available";

        auto sRef1 = ctxt.GetServiceReference<test::Interface2>();
        ASSERT_TRUE(static_cast<bool>(sRef1)) << "Service must be available after it's dependency is available";
        auto service = ctxt.GetService<test::Interface2>(sRef1);
        ASSERT_NE(service, nullptr);
        compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::ACTIVE)
            << "State must be ACTIVE after call to GetService";
        EXPECT_NO_THROW(service->ExtendedDescription()) << "Throws if the dependency could not be found";

        result = isBundleLoadedInThisProcess("TestBundleDSTOI6");
        EXPECT_TRUE(result) << "library must be available";

        auto token1 = ctxt.AddServiceListener(
            [&](cppmicroservices::ServiceEvent const& evt)
            {
                // std::cout << evt << std::endl;
                auto sRef = evt.GetServiceReference();
                if (evt.GetType() == cppmicroservices::ServiceEvent::SERVICE_UNREGISTERING
                    && test::GetServiceId(sRef) == test::GetServiceId(sRef1))
                {
                    std::lock_guard<std::mutex> lk(mtx1);
                    serviceUnregistered = true;
                    cv1.notify_all();
                }
            });

        depBundle.Stop();
        {
            std::unique_lock<std::mutex> lk(mtx1);
            cv1.wait(lk, [&serviceUnregistered]() { return serviceUnregistered == true; });
            ctxt.RemoveListener(std::move(token1));
        }

        compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE);
        auto sRef2 = ctxt.GetServiceReference<test::Interface2>();
        EXPECT_FALSE(static_cast<bool>(sRef2)) << "Service must not be available after it's dependency is removed";
    }

    class tInitialization
        : public tServiceComponent
        , public testing::WithParamInterface<std::string>
    {
    };

    /**
     * Verify that Declarative Services data stored in the bundle is initialized when a component is loaded
     * Currently the only applicable data is the BundleContextPrivate used by GetBundleContext
     */
    TEST_P(tInitialization, testInitialization)
    {
        const std::string bundle = "TestBundleDSTOI" + GetParam();
        const std::string component = "sample::ServiceComponent" + GetParam();
        auto testBundle = this->StartTestBundle(bundle);
        auto compDescDTO = this->dsRuntimeService->GetComponentDescriptionDTO(testBundle, component);
        auto compConfigDTOs = this->dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        EXPECT_EQ(compConfigDTOs.size(), 1ul);

        auto ctxt = testBundle.GetBundleContext();
        ASSERT_TRUE(ctxt);
        auto sRef = ctxt.GetServiceReference<test::TestInitialization>();
        auto service = ctxt.GetService(sRef);
        std::vector<cppmicroservices::BundleContext> ctxt2 = service->GetContexts();
        int items = ctxt2.size();
        ASSERT_TRUE(items == 2 || items == 3);
        EXPECT_TRUE(ctxt == ctxt2[0]) << "Service's activator must be provided with its bundle context";
        EXPECT_TRUE(ctxt == ctxt2[1]) << "Service's US_GET_CTX_FUNC must return the bundle context";
        if (items == 3)
        {
            EXPECT_TRUE(ctxt == ctxt2[2]) << "Service's bundle activator must be provided with its bundle context";
        }
    }

    /**
     * Bundle 20 tests DS data initialization for an immediate component without a bundle activator
     * Bundle 22 tests DS data initialization for an immediate component with a bundle activator
     * Bundle 21 tests DS data initialization for a delayed component
     */
    INSTANTIATE_TEST_SUITE_P(testInitImmediateActivatorDelayed, tInitialization, testing::Values("20", "22", "21"));

    TEST_F(tServiceComponent, testDependencyInjection) // DS_TOI_18
    {
        auto testBundle = StartTestBundle("TestBundleDSTOI18");
        auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent18");
        auto compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        EXPECT_EQ(compConfigDTOs.size(), 1ul);
        EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE);

        auto ctxt = framework.GetBundleContext();
        auto sRef = ctxt.GetServiceReference<test::Interface3>();
        EXPECT_FALSE(static_cast<bool>(sRef)) << "Service must not be available before it's dependency";

        auto depBundle = StartTestBundle("TestBundleDSTOI1");

        // wait for the asynchronous task to take effect
        auto result = RepeatTaskUntilOrTimeout(
            [&compConfigDTOs, service = this->dsRuntimeService, &compDescDTO]()
            { compConfigDTOs = service->GetComponentConfigurationDTOs(compDescDTO); },
            [&compConfigDTOs]() -> bool { return compConfigDTOs.at(0).state == scr::dto::ComponentState::ACTIVE; });

        ASSERT_TRUE(result) << "Timed out waiting for state to change to ACTIVE "
                               "after the dependency became available";
        auto sRef1 = ctxt.GetServiceReference<test::Interface3>();
        EXPECT_TRUE(static_cast<bool>(sRef1)) << "Service must be available after it's dependency is available";
        auto service = ctxt.GetService<test::Interface3>(sRef1);
        ASSERT_NE(service, nullptr);

        // Verify Constructor Injection
        ASSERT_TRUE(service->isDependencyInjected()) << "Constructor based dependency injection failed";

        depBundle.Stop();
        result = RepeatTaskUntilOrTimeout(
            [&compConfigDTOs, service = this->dsRuntimeService, &compDescDTO]()
            { compConfigDTOs = service->GetComponentConfigurationDTOs(compDescDTO); },
            [&compConfigDTOs]() -> bool
            { return compConfigDTOs.at(0).state == scr::dto::ComponentState::UNSATISFIED_REFERENCE; });

        ASSERT_TRUE(result) << "Timed out waiting for state to change to UNSATISFIED_REFERENCE after "
                               "the dependency was removed";
        auto sRef2 = ctxt.GetServiceReference<test::Interface3>();
        EXPECT_FALSE(static_cast<bool>(sRef2)) << "Service must not be available after it's dependency is removed";
    }

    TEST_F(tServiceComponent, testServiceDependency_LDAPFilter) // DS_TOI_19
    {
        auto testBundle = StartTestBundle("TestBundleDSTOI19");
        auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent19");
        auto compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        EXPECT_EQ(compConfigDTOs.size(), 1ul);
        EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE);
        auto ctxt = framework.GetBundleContext();
        auto sRef = ctxt.GetServiceReference<test::Interface2>();
        EXPECT_FALSE(static_cast<bool>(sRef)) << "Service must not be available before it's dependency";

        // start non-matching bundle
        auto depBundle1 = StartTestBundle("TestBundleDSTOI1");
        auto sRef1 = ctxt.GetServiceReference<test::Interface2>();
        EXPECT_FALSE(static_cast<bool>(sRef1)) << "Service must not be available as this dependency does not match the "
                                                  "filter";

        // start matching bundle
        auto depBundle2 = StartTestBundle("TestBundleDSTOI12");
        auto result = RepeatTaskUntilOrTimeout(
            [&compConfigDTOs, service = this->dsRuntimeService, &compDescDTO]()
            { compConfigDTOs = service->GetComponentConfigurationDTOs(compDescDTO); },
            [&compConfigDTOs]() -> bool { return compConfigDTOs.at(0).state == scr::dto::ComponentState::ACTIVE; });

        ASSERT_TRUE(result) << "Timed out waiting for state to change to ACTIVE "
                               "after the dependency became available";
        auto sRef2 = ctxt.GetServiceReference<test::Interface2>();
        EXPECT_TRUE(static_cast<bool>(sRef2)) << "Service must be available after it's dependency is available";
        auto service = ctxt.GetService<test::Interface2>(sRef2);
        ASSERT_NE(service, nullptr);
        EXPECT_NO_THROW(service->ExtendedDescription()) << "Throws if the dependency could not be found";

        // stop non matching bundle
        depBundle1.Stop();
        sRef1 = ctxt.GetServiceReference<test::Interface2>();
        EXPECT_TRUE(static_cast<bool>(sRef1))
            << "Service must be available as the removed dependency does not match the "
               "filter";
        service = ctxt.GetService<test::Interface2>(sRef1);
        ASSERT_NE(service, nullptr);

        // stop matching bundle
        depBundle2.Stop();
        result = RepeatTaskUntilOrTimeout(
            [&compConfigDTOs, service = this->dsRuntimeService, &compDescDTO]()
            { compConfigDTOs = service->GetComponentConfigurationDTOs(compDescDTO); },
            [&compConfigDTOs]() -> bool
            { return compConfigDTOs.at(0).state == scr::dto::ComponentState::UNSATISFIED_REFERENCE; });

        ASSERT_TRUE(result) << "Timed out waiting for state to change to UNSATISFIED_REFERENCE after "
                               "the dependency was removed";
        sRef2 = ctxt.GetServiceReference<test::Interface2>();
        EXPECT_FALSE(static_cast<bool>(sRef2)) << "Service must not be available after it's dependency is removed";
    }

} // namespace test
