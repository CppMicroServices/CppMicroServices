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

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/LDAPProp.h"
#include "cppmicroservices/ServiceEvent.h"
#include "cppmicroservices/ServiceEventListenerHook.h"
#include "cppmicroservices/ServiceFindHook.h"
#include "cppmicroservices/ServiceListenerHook.h"
#include "gtest/gtest.h"

#include "TestUtils.h"
#include "TestingConfig.h"
#include "gmock/gmock.h"

#include <unordered_set>

US_MSVC_PUSH_DISABLE_WARNING(4996)

// conflicts with FrameworkEvent::GetMessage
#undef GetMessage

using namespace cppmicroservices;

namespace
{

    class TestServiceListener
    {
      public:
        void
        ServiceChanged(ServiceEvent const& serviceEvent)
        {
            this->events.push_back(serviceEvent);
        }

        std::vector<ServiceEvent> events;
    };

    class TestFrameworkListener
    {
      public:
        void
        Event(FrameworkEvent const& frameworkEvent)
        {
            events.push_back(frameworkEvent);
        }

        std::vector<FrameworkEvent> events;
    };

    class TestServiceEventListenerHook : public ServiceEventListenerHook
    {
      private:
        int id;
        BundleContext bundleCtx;

      public:
        TestServiceEventListenerHook(int id, BundleContext const& context) : id(id), bundleCtx(context) {}
        using MapType = ShrinkableMap<BundleContext, ShrinkableVector<ServiceListenerHook::ListenerInfo>>;

        void
        Event(ServiceEvent const& /*event*/, MapType& listeners)
        {
            // Check listener content
            ASSERT_TRUE(listeners.find(bundleCtx) != listeners.end());
            ASSERT_GT(static_cast<int>(listeners.size()), 0);
            ShrinkableVector<ServiceListenerHook::ListenerInfo>& listenerInfos = listeners[bundleCtx];

            // listener count should be 2 because the event listener hooks are called with
            // the list of listeners before filtering them according to ther LDAP filter
            if (id == 1)
            {
#ifdef US_BUILD_SHARED_LIBS
                // 2 service listeners expected
                ASSERT_EQ(listenerInfos.size(), 2);
#else
                ASSERT_GE(static_cast<int>(listenerInfos.size()), 2);
#endif
                // test that Listener is not removed
                ASSERT_FALSE(listenerInfos[0].IsRemoved());
                ASSERT_FALSE(listenerInfos[1].IsRemoved());
                ASSERT_FALSE(listenerInfos[0] == listenerInfos[1]);
            }
            else
            {
                // there is already one listener filtered out
#ifdef US_BUILD_SHARED_LIBS
                // 1 service listener expected
                ASSERT_EQ(listenerInfos.size(), 1);
#else
                ASSERT_GE(static_cast<int>(listenerInfos.size()), 1);
#endif
                ASSERT_FALSE(listenerInfos[0].IsRemoved());
            }
            if (listenerInfo.IsNull())
            {
                listenerInfo = listenerInfos[0];
            }
            else
            {
                // Check Equal listener info objects
                ASSERT_EQ(listenerInfo, listenerInfos[0]);
            }

            // Remove the listener without a filter from the list
            for (ShrinkableVector<ServiceListenerHook::ListenerInfo>::iterator infoIter = listenerInfos.begin();
                 infoIter != listenerInfos.end();)
            {
                if (infoIter->GetFilter().empty())
                {
                    infoIter = listenerInfos.erase(infoIter);
                }
                else
                {
                    ++infoIter;
                }
            }
#ifdef US_BUILD_SHARED_LIBS
            // One listener with LDAP filter should remain
            ASSERT_EQ(listenerInfos.size(), 1);
#else
            // One listener with LDAP filter should remain
            ASSERT_GE(static_cast<int>(listenerInfos.size()), 1);
#endif
        }

        ServiceListenerHook::ListenerInfo listenerInfo;
    };

    class TestServiceEventListenerHookFailure : public ServiceEventListenerHook
    {
      public:
        using MapType = ShrinkableMap<BundleContext, ShrinkableVector<ServiceListenerHook::ListenerInfo>>;

        void
        Event(ServiceEvent const&, MapType&)
        {
            throw std::runtime_error("TestServiceEventListenerHookFailure Event exception");
        }
    };

    class TestServiceFindHook : public ServiceFindHook
    {
      private:
        BundleContext bundleCtx;

      public:
        TestServiceFindHook(BundleContext const& context) : bundleCtx(context) {}

        void
        Find(BundleContext const& context,
             std::string const& /*name*/,
             std::string const& /*filter*/,
             ShrinkableVector<ServiceReferenceBase>& references)
        {
            ASSERT_EQ(context, bundleCtx);

            references.clear();
        }
    };

    // Test failure modes for FindHook
    class TestServiceFindHookFailure : public ServiceFindHook
    {
      public:
        void
        Find(BundleContext const&, std::string const&, std::string const&, ShrinkableVector<ServiceReferenceBase>&)
        {
            throw std::runtime_error("TestServiceFindHookFailure Find exception");
        }
    };

    class TestServiceListenerHook : public ServiceListenerHook
    {
      private:
        BundleContext bundleCtx;

      public:
        TestServiceListenerHook(BundleContext const& context) : bundleCtx(context) {}

        void
        Added(std::vector<ListenerInfo> const& listeners)
        {
            for (std::vector<ListenerInfo>::const_iterator iter = listeners.begin(); iter != listeners.end(); ++iter)
            {
                if (iter->IsRemoved() || iter->GetBundleContext().GetBundle() != bundleCtx.GetBundle())
                {
                    continue;
                }
                listenerInfos.insert(*iter);
                lastAdded = listeners.back();
            }
        }

        void
        Removed(std::vector<ListenerInfo> const& listeners)
        {
            for (std::vector<ListenerInfo>::const_iterator iter = listeners.begin(); iter != listeners.end(); ++iter)
            {
                listenerInfos.erase(*iter);
            }
            lastRemoved = listeners.back();
        }

        std::unordered_set<ListenerInfo> listenerInfos;
        ListenerInfo lastAdded;
        ListenerInfo lastRemoved;
    };

    class TestServiceListenerHookFailure : public ServiceListenerHook
    {
      public:
        void
        Added(std::vector<ListenerInfo> const&)
        {
            throw std::runtime_error("TestServiceListenerHookFailure Added exception");
        }

        void
        Removed(std::vector<ListenerInfo> const&)
        {
            throw std::runtime_error("TestServiceListenerHookFailure Removed exception");
        }
    };

    class ServiceHooksTest : public ::testing::Test
    {
      protected:
        Framework framework;
        BundleContext context;

      public:
        ServiceHooksTest() : framework(FrameworkFactory().NewFramework()) {};

        ~ServiceHooksTest() override = default;

        void
        SetUp() override
        {
            framework.Start();
            auto bundles = framework.GetBundleContext().GetBundles();
            ASSERT_FALSE(bundles.empty());
            for (auto& b : bundles)
            {
                if (b.GetSymbolicName() == "main")
                {
                    b.Start();
                    break;
                }
            }
            context = cppmicroservices::testing::GetBundle("main", framework.GetBundleContext()).GetBundleContext();
        }

        void
        TearDown() override
        {
            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        }
    };

} // end unnamed namespace

class MockServiceEventListenerHook : public ServiceEventListenerHook
{
  public:
    MockServiceEventListenerHook() = default;

    using MapType = ShrinkableMap<BundleContext, ShrinkableVector<ServiceListenerHook::ListenerInfo>>;

    MOCK_METHOD(void, Event, (ServiceEvent const& event, MapType& listeners), (override));
};

class MockServiceFindHook : public ServiceFindHook
{
  public:
    MockServiceFindHook() = default;

    MOCK_METHOD(void,
                Find,
                (BundleContext const& context,
                 std::string const& name,
                 std::string const& filter,
                 ShrinkableVector<ServiceReferenceBase>& references),
                (override));
};

class MockServiceListenerHook : public ServiceListenerHook
{
  public:
    MockServiceListenerHook() = default;

    MOCK_METHOD(void, Added, (std::vector<ListenerInfo> const& listeners));
    MOCK_METHOD(void, Removed, (std::vector<ListenerInfo> const& listeners));
};

TEST_F(ServiceHooksTest, TestListenerHook)
{
    TestServiceListener serviceListener1;
    TestServiceListener serviceListener2;
    context.AddServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);
    context.AddServiceListener(&serviceListener2,
                               &TestServiceListener::ServiceChanged,
                               LDAPProp(Constants::OBJECTCLASS) == "bla");

    auto serviceListenerHook1 = std::make_shared<TestServiceListenerHook>(context);
    ServiceProperties hookProps1;
    hookProps1[Constants::SERVICE_RANKING] = 0;
    ServiceRegistration<ServiceListenerHook> listenerHookReg1
        = context.RegisterService<ServiceListenerHook>(serviceListenerHook1, hookProps1);

    auto serviceListenerHook2 = std::make_shared<TestServiceListenerHook>(context);
    ServiceProperties hookProps2;
    hookProps2[Constants::SERVICE_RANKING] = 10;
    ServiceRegistration<ServiceListenerHook> listenerHookReg2
        = context.RegisterService<ServiceListenerHook>(serviceListenerHook2, hookProps2);

#ifdef US_BUILD_SHARED_LIBS
    // check if hooks got notified about the existing listeners
    ASSERT_EQ(serviceListenerHook1->listenerInfos.size(), 2);
#endif

    const std::size_t listenerInfoSizeOld = serviceListenerHook1->listenerInfos.size() - 2;

    context.AddServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);
    auto lastAdded = serviceListenerHook1->lastAdded;

    context.AddServiceListener(&serviceListener1,
                               &TestServiceListener::ServiceChanged,
                               LDAPProp(Constants::OBJECTCLASS) == "blub");
    // Test same ListenerInfo object
    ASSERT_EQ(lastAdded, serviceListenerHook1->lastRemoved);
    ASSERT_FALSE(lastAdded == serviceListenerHook1->lastAdded);

    context.RemoveServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);
    context.RemoveServiceListener(&serviceListener2, &TestServiceListener::ServiceChanged);

    // Removed listener infos
    ASSERT_EQ(serviceListenerHook1->listenerInfos.size(), listenerInfoSizeOld);

    listenerHookReg2.Unregister();
    listenerHookReg1.Unregister();
}

TEST_F(ServiceHooksTest, TestFindHook)
{
    auto serviceFindHook1 = std::make_shared<TestServiceFindHook>(context);
    ServiceProperties hookProps1;
    hookProps1[Constants::SERVICE_RANKING] = 0;
    ServiceRegistration<ServiceFindHook> findHookReg1
        = context.RegisterService<ServiceFindHook>(serviceFindHook1, hookProps1);

    auto serviceFindHook2 = std::make_shared<TestServiceFindHook>(context);
    ServiceProperties hookProps2;
    hookProps2[Constants::SERVICE_RANKING] = 10;
    ServiceRegistration<ServiceFindHook> findHookReg2
        = context.RegisterService<ServiceFindHook>(serviceFindHook2, hookProps2);

    TestServiceListener serviceListener;
    context.AddServiceListener(&serviceListener, &TestServiceListener::ServiceChanged);

    auto bundle = cppmicroservices::testing::InstallLib(context, "TestBundleA");
    ASSERT_TRUE(bundle);

    bundle.Start();

    ASSERT_EQ(serviceListener.events.size(), 1);

    std::vector<ServiceReferenceU> refs = context.GetServiceReferences("cppmicroservices::TestBundleAService");
    ASSERT_TRUE(refs.empty());

    ServiceReferenceU ref = context.GetServiceReference("cppmicroservices::TestBundleAService");
    // Invalid reference (filtered out)
    ASSERT_FALSE(ref);

    findHookReg2.Unregister();
    findHookReg1.Unregister();

    refs = context.GetServiceReferences("cppmicroservices::TestBundleAService");
    // Non-empty references
    ASSERT_FALSE(refs.empty());

    ref = context.GetServiceReference("cppmicroservices::TestBundleAService");
    ASSERT_TRUE(ref);

    bundle.Stop();

    context.RemoveServiceListener(&serviceListener, &TestServiceListener::ServiceChanged);
}

TEST_F(ServiceHooksTest, TestEventListenerHook)
{
    TestServiceListener serviceListener1;
    TestServiceListener serviceListener2;
    context.AddServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);
    context.AddServiceListener(&serviceListener2,
                               &TestServiceListener::ServiceChanged,
                               LDAPProp(Constants::OBJECTCLASS) == "bla");

    auto serviceEventListenerHook1 = std::make_shared<TestServiceEventListenerHook>(1, context);
    ServiceProperties hookProps1;
    hookProps1[Constants::SERVICE_RANKING] = 10;
    ServiceRegistration<ServiceEventListenerHook> eventListenerHookReg1
        = context.RegisterService<ServiceEventListenerHook>(serviceEventListenerHook1, hookProps1);

    auto serviceEventListenerHook2 = std::make_shared<TestServiceEventListenerHook>(2, context);
    ServiceProperties hookProps2;
    hookProps2[Constants::SERVICE_RANKING] = 0;
    ServiceRegistration<ServiceEventListenerHook> eventListenerHookReg2
        = context.RegisterService<ServiceEventListenerHook>(serviceEventListenerHook2, hookProps2);

    // service event of service event listener hook
    ASSERT_TRUE(serviceListener1.events.empty());
    // no service event for filtered listener
    ASSERT_TRUE(serviceListener2.events.empty());

    auto bundle = cppmicroservices::testing::InstallLib(context, "TestBundleA");
    ASSERT_TRUE(bundle);

    bundle.Start();

    bundle.Stop();

    // Check that there is no service event due to service event listener hook
    ASSERT_TRUE(serviceListener1.events.empty());
    // Test that there is no service event for filtered listener
    // due to service event listener hook
    ASSERT_TRUE(serviceListener2.events.empty());

    eventListenerHookReg2.Unregister();
    eventListenerHookReg1.Unregister();

    context.RemoveServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);
    context.RemoveServiceListener(&serviceListener2, &TestServiceListener::ServiceChanged);
}

TEST_F(ServiceHooksTest, TestFindHookFailure)
{
    auto findHookReg
        = framework.GetBundleContext().RegisterService<ServiceFindHook>(std::make_shared<TestServiceFindHookFailure>());

    auto bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    ASSERT_TRUE(bundle);

    TestFrameworkListener listener;
    auto fwkListenerToken = framework.GetBundleContext().AddFrameworkListener(
        std::bind(&TestFrameworkListener::Event, &listener, std::placeholders::_1));

    bundle.Start();

    std::vector<ServiceReferenceU> refs
        = framework.GetBundleContext().GetServiceReferences("cppmicroservices::TestBundleAService");

    // Test for correct number of Framework events
    ASSERT_EQ(1, listener.events.size());

    std::for_each(listener.events.begin(),
                  listener.events.end(),
                  [](FrameworkEvent const& evt)
                  {
                      // Test for the existence of an exception
                      ASSERT_NE(evt.GetThrowable(), nullptr);
                      // Test for the correct framework event type
                      ASSERT_EQ(evt.GetType(), FrameworkEvent::Type::FRAMEWORK_WARNING);
                      std::string msg(evt.GetMessage());
                      // Test for the correct event message
                      ASSERT_NE(std::string::npos, msg.find("Failed to call find hook #"));
                  });

    bundle.Stop();
    findHookReg.Unregister();
    framework.GetBundleContext().RemoveListener(std::move(fwkListenerToken));
}

TEST_F(ServiceHooksTest, TestEventListenerHookFailure)
{
    auto eventListenerHookReg = framework.GetBundleContext().RegisterService<ServiceEventListenerHook>(
        std::make_shared<TestServiceEventListenerHookFailure>());

    auto bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    ASSERT_TRUE(bundle);

    TestFrameworkListener listener;
    auto fwkListenerToken = framework.GetBundleContext().AddFrameworkListener(
        std::bind(&TestFrameworkListener::Event, &listener, std::placeholders::_1));

    bundle.Start();

    // Test for correct number of Framework events
    ASSERT_EQ(1, listener.events.size());

    std::for_each(listener.events.begin(),
                  listener.events.end(),
                  [](FrameworkEvent const& evt)
                  {
                      // Test for the existence of an exception
                      ASSERT_NE(evt.GetThrowable(), nullptr);
                      // Test for the correct framework event type
                      ASSERT_EQ(evt.GetType(), FrameworkEvent::Type::FRAMEWORK_WARNING);
                      std::string msg(evt.GetMessage());
                      // Test for the correct event message
                      ASSERT_NE(std::string::npos, msg.find("Failed to call event hook  #"));
                  });

    bundle.Stop();
    eventListenerHookReg.Unregister();
    framework.GetBundleContext().RemoveListener(std::move(fwkListenerToken));
}

TEST_F(ServiceHooksTest, TestListenerHookFailure)
{
    auto listenerHookReg = framework.GetBundleContext().RegisterService<ServiceListenerHook>(
        std::make_shared<TestServiceListenerHookFailure>());

    TestFrameworkListener listener;
    auto fwkListenerToken = framework.GetBundleContext().AddFrameworkListener(
        std::bind(&TestFrameworkListener::Event, &listener, std::placeholders::_1));

    auto listenerToken = framework.GetBundleContext().AddServiceListener([](ServiceEvent const&) {});

    framework.GetBundleContext().RemoveListener(std::move(listenerToken));

    // Test for correct number of Framework events
    ASSERT_EQ(2, listener.events.size());

    std::for_each(listener.events.begin(),
                  listener.events.end(),
                  [](FrameworkEvent const& evt)
                  {
                      // Test for the existence of an exception
                      ASSERT_NE(evt.GetThrowable(), nullptr);
                      // Test for the correct framework event type
                      ASSERT_EQ(evt.GetType(), FrameworkEvent::Type::FRAMEWORK_WARNING);
                      std::string msg(evt.GetMessage());
                      // Test for the correct event message
                      ASSERT_NE(std::string::npos, msg.find("Failed to call listener hook #"));
                  });

    listenerHookReg.Unregister();
    framework.GetBundleContext().RemoveListener(std::move(fwkListenerToken));
}

TEST_F(ServiceHooksTest, TestListenerHookCallbackOrdering)
{
    TestServiceListener serviceListener1;
    TestServiceListener serviceListener2;
    context.AddServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);
    context.AddServiceListener(&serviceListener2,
                               &TestServiceListener::ServiceChanged,
                               LDAPProp(Constants::OBJECTCLASS) == "bla");

    auto serviceListenerHook1 = std::make_shared<MockServiceListenerHook>();
    auto serviceListenerHook2 = std::make_shared<MockServiceListenerHook>();

#ifdef US_BUILD_SHARED_LIBS
    ::testing::InSequence s;
    EXPECT_CALL(*serviceListenerHook1, Added(::testing::_));
    EXPECT_CALL(*serviceListenerHook2, Added(::testing::_));
    EXPECT_CALL(*serviceListenerHook2, Removed(::testing::_));
    EXPECT_CALL(*serviceListenerHook1, Removed(::testing::_));
    EXPECT_CALL(*serviceListenerHook2, Added(::testing::_));
    EXPECT_CALL(*serviceListenerHook1, Added(::testing::_));

    EXPECT_CALL(*serviceListenerHook2, Removed(::testing::_));
    EXPECT_CALL(*serviceListenerHook1, Removed(::testing::_));
    EXPECT_CALL(*serviceListenerHook2, Added(::testing::_));
    EXPECT_CALL(*serviceListenerHook1, Added(::testing::_));

    EXPECT_CALL(*serviceListenerHook2, Removed(::testing::_));
    EXPECT_CALL(*serviceListenerHook1, Removed(::testing::_));
    EXPECT_CALL(*serviceListenerHook2, Removed(::testing::_));
    EXPECT_CALL(*serviceListenerHook1, Removed(::testing::_));
#endif

    ServiceProperties hookProps1;
    hookProps1[Constants::SERVICE_RANKING] = 0;
    ServiceRegistration<ServiceListenerHook> listenerHookReg1
        = context.RegisterService<ServiceListenerHook>(serviceListenerHook1, hookProps1);

    ServiceProperties hookProps2;
    hookProps2[Constants::SERVICE_RANKING] = 10;
    ServiceRegistration<ServiceListenerHook> listenerHookReg2
        = context.RegisterService<ServiceListenerHook>(serviceListenerHook2, hookProps2);

    context.AddServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);

    context.AddServiceListener(&serviceListener1,
                               &TestServiceListener::ServiceChanged,
                               LDAPProp(Constants::OBJECTCLASS) == "blub");

    context.RemoveServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);
    context.RemoveServiceListener(&serviceListener2, &TestServiceListener::ServiceChanged);

    listenerHookReg2.Unregister();
    listenerHookReg1.Unregister();
}

TEST_F(ServiceHooksTest, TestFindHookCallbackOrdering)
{
    auto serviceFindHook1 = std::make_shared<MockServiceFindHook>();
    auto serviceFindHook2 = std::make_shared<MockServiceFindHook>();

    ::testing::InSequence s;
    EXPECT_CALL(*serviceFindHook2, Find(::testing::_, ::testing::_, ::testing::_, ::testing::_));
    EXPECT_CALL(*serviceFindHook1, Find(::testing::_, ::testing::_, ::testing::_, ::testing::_));
    EXPECT_CALL(*serviceFindHook2, Find(::testing::_, ::testing::_, ::testing::_, ::testing::_));
    EXPECT_CALL(*serviceFindHook1, Find(::testing::_, ::testing::_, ::testing::_, ::testing::_));

    ServiceProperties hookProps1;
    hookProps1[Constants::SERVICE_RANKING] = 0;
    ServiceRegistration<ServiceFindHook> findHookReg1
        = context.RegisterService<ServiceFindHook>(serviceFindHook1, hookProps1);

    ServiceProperties hookProps2;
    hookProps2[Constants::SERVICE_RANKING] = 10;
    ServiceRegistration<ServiceFindHook> findHookReg2
        = context.RegisterService<ServiceFindHook>(serviceFindHook2, hookProps2);

    TestServiceListener serviceListener;
    context.AddServiceListener(&serviceListener, &TestServiceListener::ServiceChanged);

    auto bundle = cppmicroservices::testing::InstallLib(context, "TestBundleA");

    bundle.Start();

    std::vector<ServiceReferenceU> refs = context.GetServiceReferences("cppmicroservices::TestBundleAService");
    ServiceReferenceU ref = context.GetServiceReference("cppmicroservices::TestBundleAService");

    findHookReg2.Unregister();
    findHookReg1.Unregister();

    bundle.Stop();

    context.RemoveServiceListener(&serviceListener, &TestServiceListener::ServiceChanged);
}

TEST_F(ServiceHooksTest, TestEventListenerHookCallbackOrdering)
{
    TestServiceListener serviceListener1;
    TestServiceListener serviceListener2;
    context.AddServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);
    context.AddServiceListener(&serviceListener2,
                               &TestServiceListener::ServiceChanged,
                               LDAPProp(Constants::OBJECTCLASS) == "bla");

    auto serviceEventListenerHook1 = std::make_shared<MockServiceEventListenerHook>();
    auto serviceEventListenerHook2 = std::make_shared<MockServiceEventListenerHook>();

    ::testing::InSequence s;
    EXPECT_CALL(*serviceEventListenerHook1, Event(::testing::_, ::testing::_)).Times(2);
    EXPECT_CALL(*serviceEventListenerHook2, Event(::testing::_, ::testing::_));
    EXPECT_CALL(*serviceEventListenerHook1, Event(::testing::_, ::testing::_));
    EXPECT_CALL(*serviceEventListenerHook2, Event(::testing::_, ::testing::_));
    EXPECT_CALL(*serviceEventListenerHook1, Event(::testing::_, ::testing::_));
    EXPECT_CALL(*serviceEventListenerHook2, Event(::testing::_, ::testing::_));
    EXPECT_CALL(*serviceEventListenerHook1, Event(::testing::_, ::testing::_));

    ServiceProperties hookProps1;
    hookProps1[Constants::SERVICE_RANKING] = 10;
    ServiceRegistration<ServiceEventListenerHook> eventListenerHookReg1
        = context.RegisterService<ServiceEventListenerHook>(serviceEventListenerHook1, hookProps1);

    ServiceProperties hookProps2;
    hookProps2[Constants::SERVICE_RANKING] = 0;
    ServiceRegistration<ServiceEventListenerHook> eventListenerHookReg2
        = context.RegisterService<ServiceEventListenerHook>(serviceEventListenerHook2, hookProps2);

    auto bundle = cppmicroservices::testing::InstallLib(context, "TestBundleA");

    bundle.Start();

    bundle.Stop();

    eventListenerHookReg2.Unregister();
    eventListenerHookReg1.Unregister();

    context.RemoveServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);
    context.RemoveServiceListener(&serviceListener2, &TestServiceListener::ServiceChanged);
}

US_MSVC_POP_WARNING
