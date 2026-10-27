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

#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "../../src/manager/ReferenceManagerImpl.hpp"
#include "../TestUtils.hpp"
#include "ConcurrencyTestUtil.hpp"
#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/servicecomponent/detail/Binders.hpp"
#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp"

#include "Mocks.hpp"

#include "gtest/gtest.h"

namespace cppmicroservices
{
    namespace scrimpl
    {

        namespace
        {
            struct TestService
            {
                virtual ~TestService() = default;
            };

            struct TestComponent
            {
                std::vector<std::shared_ptr<TestService>> boundServices;

                void
                BindService(std::shared_ptr<TestService> const& svc)
                {
                    boundServices.push_back(svc);
                }

                void
                UnbindService(std::shared_ptr<TestService> const& svc)
                {
                    boundServices.erase(
                        std::remove(boundServices.begin(), boundServices.end(), svc),
                        boundServices.end());
                }
            };

            class MockComponentContext : public service::component::ComponentContext
            {
              public:
                MOCK_CONST_METHOD0(GetProperties, std::unordered_map<std::string, Any>(void));
                MOCK_CONST_METHOD0(GetBundleContext, BundleContext(void));
                MOCK_CONST_METHOD0(GetUsingBundle, Bundle(void));
                MOCK_METHOD1(EnableComponent, void(std::string const&));
                MOCK_METHOD1(DisableComponent, void(std::string const&));
                MOCK_CONST_METHOD0(GetServiceReference, ServiceReferenceBase(void));
                MOCK_CONST_METHOD2(LocateService, std::shared_ptr<void>(std::string const&, std::string const&));
                MOCK_CONST_METHOD2(LocateService,
                                   std::shared_ptr<void>(std::string const&, ServiceReferenceBase const&));
                MOCK_CONST_METHOD2(LocateServices,
                                   std::vector<std::shared_ptr<void>>(std::string const&, std::string const&));
            };
        } // namespace

        // Verify that BindReferences does not invoke the user's bind method when
        // LocateService returns nullptr (no matching services for a 0..n reference
        // at activation time).
        TEST(DynamicBinderBehavior, BindReferencesSkipsBindWhenNoServiceAvailable)
        {
            using namespace service::component::detail;
            using ::testing::An;
            using ::testing::Return;

            std::vector<std::shared_ptr<Binder<TestComponent>>> binders;
            binders.push_back(
                std::make_shared<DynamicBinder<TestComponent, TestService>>(
                    "TestRef",
                    &TestComponent::BindService,
                    &TestComponent::UnbindService));

            ComponentInstanceImpl<TestComponent> compInstance({}, binders);

            auto mockContext = std::make_shared<MockComponentContext>();
            EXPECT_CALL(*mockContext, LocateService(An<std::string const&>(), An<std::string const&>()))
                .WillRepeatedly(Return(nullptr));

            compInstance.CreateInstance(mockContext);
            compInstance.BindReferences(mockContext);

            auto comp = compInstance.GetInstance();
            ASSERT_TRUE(comp);

            EXPECT_TRUE(comp->boundServices.empty())
                << "BindReferences must not invoke the user's bind method with nullptr.";
        }

        // Verify that UnbindReferences does not invoke the user's unbind method
        // when LocateService returns nullptr (service already removed before
        // component deactivation).
        TEST(DynamicBinderBehavior, UnbindReferencesSkipsUnbindWhenServiceAlreadyGone)
        {
            using namespace service::component::detail;
            using ::testing::An;
            using ::testing::Return;

            std::vector<std::shared_ptr<TestService>> unboundServices;
            struct TrackingComponent
            {
                std::vector<std::shared_ptr<TestService>>* unboundSvcs;

                void
                BindService(std::shared_ptr<TestService> const&)
                {
                }

                void
                UnbindService(std::shared_ptr<TestService> const& svc)
                {
                    unboundSvcs->push_back(svc);
                }
            };

            std::vector<std::shared_ptr<Binder<TrackingComponent>>> binders;
            binders.push_back(
                std::make_shared<DynamicBinder<TrackingComponent, TestService>>(
                    "TestRef",
                    &TrackingComponent::BindService,
                    &TrackingComponent::UnbindService));

            ComponentInstanceImpl<TrackingComponent> compInstance({}, binders);

            auto mockContext = std::make_shared<MockComponentContext>();
            EXPECT_CALL(*mockContext, LocateService(An<std::string const&>(), An<std::string const&>()))
                .WillRepeatedly(Return(nullptr));

            compInstance.CreateInstance(mockContext);

            auto comp = compInstance.GetInstance();
            ASSERT_TRUE(comp);
            comp->unboundSvcs = &unboundServices;

            compInstance.UnbindReferences();

            EXPECT_TRUE(unboundServices.empty())
                << "UnbindReferences must not invoke the user's unbind method with nullptr.";
        }

        // Verify that a 0..n dynamic reluctant reference produces exactly one
        // REBIND notification per service added.
        TEST(DynamicBinderBehavior, DynamicReluctantMultipleCardinalityOneNotificationPerService)
        {
            auto framework = cppmicroservices::FrameworkFactory().NewFramework();
            framework.Start();

            auto bc = framework.GetBundleContext();
            auto mockLogger = std::make_shared<MockLogger>();

            metadata::ReferenceMetadata fakeMetadata {};
            fakeMetadata.name = "ref";
            fakeMetadata.interfaceName = us_service_interface_iid<dummy::Reference1>();
            fakeMetadata.policy = "dynamic";
            fakeMetadata.policyOption = "reluctant";
            fakeMetadata.cardinality = "0..n";
            auto cardLimits = metadata::GetReferenceCardinalityExtents(fakeMetadata.cardinality);
            fakeMetadata.minCardinality = std::get<0>(cardLimits);
            fakeMetadata.maxCardinality = std::get<1>(cardLimits);

            ReferenceManagerImpl refManager(fakeMetadata, bc, mockLogger, "TestComponent");

            int bindNotificationCount = 0;
            refManager.RegisterListener(
                [&](RefChangeNotification const& notification)
                {
                    if (notification.event == RefEvent::REBIND && notification.serviceRefToBind)
                    {
                        bindNotificationCount++;
                    }
                });

            EXPECT_CALL(*(mockLogger).get(),
                        Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG, testing::_))
                .Times(testing::AnyNumber());

            auto depSvcReg = bc.RegisterService<dummy::Reference1>(
                std::make_shared<dummy::Reference1>());
            ASSERT_TRUE(depSvcReg);

            EXPECT_EQ(refManager.GetBoundReferences().size(), 1ul);
            EXPECT_EQ(bindNotificationCount, 1)
                << "First service added to a 0..n dynamic reluctant reference must produce "
                   "exactly 1 REBIND notification.";

            int bindCountBeforeSecond = bindNotificationCount;
            auto depSvcReg2 = bc.RegisterService<dummy::Reference1>(
                std::make_shared<dummy::Reference1>());
            ASSERT_TRUE(depSvcReg2);

            EXPECT_EQ(refManager.GetBoundReferences().size(), 2ul);
            EXPECT_EQ(bindNotificationCount - bindCountBeforeSecond, 1)
                << "Each additional service must produce exactly 1 REBIND notification.";

            depSvcReg.Unregister();
            depSvcReg2.Unregister();

            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        }

    } // namespace scrimpl
} // namespace cppmicroservices

// ============================================================
// Integration tests using the TestBundleDSDROM bundle.
//
// This bundle has:
//   - A static required reference to test::Interface1 (constructor-injected)
//   - A dynamic reluctant 0..n reference to test::Interface3
//
// The component provides test::Interface2 with ExtendedDescription() that
// reports bind/unbind counts.
// ============================================================

namespace
{
    class Interface1Impl : public test::Interface1
    {
      public:
        std::string
        Description() override
        {
            return "Interface1Impl";
        }
    };

    class Interface3Impl : public test::Interface3
    {
      public:
        bool
        isDependencyInjected() override
        {
            return true;
        }
    };
} // namespace

// Verify that activating a component with a dynamic 0..n reference does not
// invoke bind when no matching services exist.
TEST(DynamicReferenceBinding, BindNotCalledDuringActivationWhenNoServicesExist)
{
    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();
    auto bc = framework.GetBundleContext();

    test::InstallAndStartDS(bc);

    auto testBundle = test::InstallAndStartBundle(bc, "TestBundleDSDROM");
    ASSERT_TRUE(testBundle);

    EXPECT_FALSE(bc.GetServiceReference<test::Interface2>());

    // Register the static required dependency — component activates.
    // No Interface3 services exist, so LocateService returns nullptr for the
    // dynamic 0..n reference during BindReferences.
    auto staticDepReg = bc.RegisterService<test::Interface1>(
        std::make_shared<Interface1Impl>());
    ASSERT_TRUE(staticDepReg);

    auto svcRef = bc.GetServiceReference<test::Interface2>();
    ASSERT_TRUE(svcRef);
    auto svc = bc.GetService<test::Interface2>(svcRef);
    ASSERT_TRUE(svc);

    auto desc = svc->ExtendedDescription();

    EXPECT_NE(desc.find("nullBindCount=0"), std::string::npos)
        << "Bind must not be called with nullptr during activation. Got: " << desc;
    EXPECT_NE(desc.find("bindCount=0"), std::string::npos)
        << "No real binds should occur when no services exist. Got: " << desc;

    staticDepReg.Unregister();
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
}

// Verify that registering one service for a dynamic reluctant 0..n reference
// results in exactly one bind call.
TEST(DynamicReferenceBinding, OneBindPerServiceForDynamicReluctantMultiple)
{
    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();
    auto bc = framework.GetBundleContext();

    test::InstallAndStartDS(bc);

    auto testBundle = test::InstallAndStartBundle(bc, "TestBundleDSDROM");
    ASSERT_TRUE(testBundle);

    auto staticDepReg = bc.RegisterService<test::Interface1>(
        std::make_shared<Interface1Impl>());
    ASSERT_TRUE(staticDepReg);

    auto svcRef = bc.GetServiceReference<test::Interface2>();
    ASSERT_TRUE(svcRef);
    auto svc = bc.GetService<test::Interface2>(svcRef);
    ASSERT_TRUE(svc);

    // Register the first Interface3 service.
    auto optionalDepReg = bc.RegisterService<test::Interface3>(
        std::make_shared<Interface3Impl>());
    ASSERT_TRUE(optionalDepReg);

    auto desc = svc->ExtendedDescription();

    EXPECT_NE(desc.find("bindCount=1"), std::string::npos)
        << "Expected exactly 1 bind after registering one service. Got: " << desc;

    // Register a second Interface3 service.
    auto optionalDepReg2 = bc.RegisterService<test::Interface3>(
        std::make_shared<Interface3Impl>());
    ASSERT_TRUE(optionalDepReg2);

    desc = svc->ExtendedDescription();
    EXPECT_NE(desc.find("bindCount=2"), std::string::npos)
        << "Expected exactly 2 binds after registering two services. Got: " << desc;

    optionalDepReg.Unregister();
    optionalDepReg2.Unregister();
    staticDepReg.Unregister();

    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
}

namespace
{
    // Extract the integer value following "key=" in a component ExtendedDescription
    // string such as "bindCount=1 nullBindCount=0 ... optionalDeps=1".
    int
    ExtractCount(std::string const& desc, std::string const& key)
    {
        auto pos = desc.find(key + "=");
        if (pos == std::string::npos)
        {
            return -1;
        }
        return std::stoi(desc.substr(pos + key.size() + 1));
    }
} // namespace

// The TestBundleDSDROM component has a static required reference (test::Interface1)
// and a dynamic reluctant 0..n reference (test::Interface3). Registering Interface1
// activates the (immediate) component; if a single Interface3 service is registered
// concurrently, there is a window in which that service is both (a) captured by the
// activation path's bound-services snapshot and bound during BindReferences, and
// (b) delivered again by the queued Rebind -- ensure we don't double bind the same svc
TEST(DynamicReferenceBinding, ConcurrentActivationAndDependencyChangeDoesNotDoubleBind)
{
    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();
    auto bc = framework.GetBundleContext();

    test::InstallAndStartDS(bc);
    auto testBundle = test::InstallAndStartBundle(bc, "TestBundleDSDROM");
    ASSERT_TRUE(testBundle);

    constexpr int kIterations = 500;
    for (int i = 0; i < kIterations; ++i)
    {
        Barrier startLine(2);

        // Thread A: trigger activation by registering the static required dependency.
        // Thread B: register the single dynamic (Interface3) dependency.
        // Both are released simultaneously so activation and the dependency change
        // race, exactly as they would for a client whose provider bundles load
        // concurrently.
        cppmicroservices::ServiceRegistration<test::Interface1> staticDepReg;
        cppmicroservices::ServiceRegistration<test::Interface3> optionalDepReg;

        std::thread activator(
            [&]
            {
                startLine.Wait();
                staticDepReg = bc.RegisterService<test::Interface1>(std::make_shared<Interface1Impl>());
            });
        std::thread depChanger(
            [&]
            {
                startLine.Wait();
                optionalDepReg = bc.RegisterService<test::Interface3>(std::make_shared<Interface3Impl>());
            });
        activator.join();
        depChanger.join();

        auto svcRef = bc.GetServiceReference<test::Interface2>();
        ASSERT_TRUE(svcRef);
        auto svc = bc.GetService<test::Interface2>(svcRef);
        ASSERT_TRUE(svc);

        auto desc = svc->ExtendedDescription();
        svc.reset();

        int duplicateBinds = ExtractCount(desc, "duplicateBindCount");

        ASSERT_EQ(duplicateBinds, 0)
            << "iteration " << i
            << ": the component's bind method was invoked twice for the SAME service on the SAME "
               "'optionalDeps' reference (bind called twice for the same reference). Got: "
            << desc;

        // Reset for the next iteration: removing the static required dependency
        // deactivates the component and destroys the instance, so bind counts start
        // fresh on the next activation.
        optionalDepReg.Unregister();
        staticDepReg.Unregister();
        ASSERT_FALSE(bc.GetServiceReference<test::Interface2>())
            << "iteration " << i << ": component did not deactivate after removing its required dependency.";
    }

    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
}

// Verify that unbind is not called with nullptr during component deactivation
// when the dynamic service was already removed at runtime.
TEST(DynamicReferenceBinding, UnbindNotCalledWithNullptrDuringDeactivation)
{
    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();
    auto bc = framework.GetBundleContext();

    test::InstallAndStartDS(bc);

    auto testBundle = test::InstallAndStartBundle(bc, "TestBundleDSDROM");
    ASSERT_TRUE(testBundle);

    auto staticDepReg = bc.RegisterService<test::Interface1>(
        std::make_shared<Interface1Impl>());
    ASSERT_TRUE(staticDepReg);

    auto svcRef = bc.GetServiceReference<test::Interface2>();
    ASSERT_TRUE(svcRef);
    auto svc = bc.GetService<test::Interface2>(svcRef);
    ASSERT_TRUE(svc);

    // Register an optional service, then remove it.
    auto optionalDepReg = bc.RegisterService<test::Interface3>(
        std::make_shared<Interface3Impl>());
    ASSERT_TRUE(optionalDepReg);

    auto desc = svc->ExtendedDescription();
    EXPECT_NE(desc.find("bindCount=1"), std::string::npos) << "Setup failed: " << desc;

    // Remove the optional service — unbind called with real service via InvokeUnbindMethod.
    optionalDepReg.Unregister();

    desc = svc->ExtendedDescription();
    EXPECT_NE(desc.find("unbindCount=1"), std::string::npos)
        << "Expected 1 real unbind after service removal: " << desc;

    // Deactivate the component by removing the static required dependency.
    // UnbindReferences is called; LocateService returns nullptr for the
    // already-removed optional service.
    staticDepReg.Unregister();

    // Re-activate to query the static nullUnbindCount.
    auto staticDepReg2 = bc.RegisterService<test::Interface1>(
        std::make_shared<Interface1Impl>());
    ASSERT_TRUE(staticDepReg2);

    svcRef = bc.GetServiceReference<test::Interface2>();
    ASSERT_TRUE(svcRef);
    svc = bc.GetService<test::Interface2>(svcRef);
    ASSERT_TRUE(svc);

    desc = svc->ExtendedDescription();

    EXPECT_NE(desc.find("nullUnbindCount=0"), std::string::npos)
        << "Unbind must not be called with nullptr during deactivation. Got: " << desc;

    staticDepReg2.Unregister();
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
}
