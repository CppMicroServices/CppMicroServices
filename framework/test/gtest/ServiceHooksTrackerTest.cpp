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

#include "TestUtils.h"
#include "TestingConfig.h"
#include "gtest/gtest.h"

#include <unordered_set>

#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/ServiceInterface.h"
#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/ServiceTracker.h"

#include "ServiceControlInterface.h"

#include <chrono>
#include <future>
#include <memory>
#include <thread>
#include <unordered_map>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
US_MSVC_PUSH_DISABLE_WARNING(4996)

// conflicts with FrameworkEvent::GetMessage
#undef GetMessage

using namespace cppmicroservices;

namespace
{
        class FooService
    {
      public:
        virtual ~FooService() = default;
    };

    class FooServiceImpl final : public FooService
    {
      public:
        ~FooServiceImpl() = default;
    };

    class CustomFooTracker final : public cppmicroservices::ServiceTrackerCustomizer<FooService>
    {
        /**
         * Called when a service is started.
         */
        std::shared_ptr<FooService>
        AddingService(::cppmicroservices::ServiceReference<FooService> const& reference) override
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            return reference.GetBundle().GetBundleContext().GetService(reference);
        }

        /**
         * Called when a service is modified. We don't care about this.
         */
        void
        ModifiedService(::cppmicroservices::ServiceReference<FooService> const&,
                        std::shared_ptr<FooService> const&) override
        {
        }

        /**
         * Called when a service is removed.
         */
        void
        RemovedService(::cppmicroservices::ServiceReference<FooService> const&,
                       std::shared_ptr<FooService> const&) override
        {
        }
    };

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

            ordering.push_back(id);
        }

        ServiceListenerHook::ListenerInfo listenerInfo;

        static std::vector<int> ordering;
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

    std::vector<int> TestServiceEventListenerHook::ordering;

    class TestServiceFindHook : public ServiceFindHook
    {
      private:
        int id;
        BundleContext bundleCtx;

      public:
        TestServiceFindHook(int id, BundleContext const& context) : id(id), bundleCtx(context) {}

        void
        Find(BundleContext const& context,
             std::string const& /*name*/,
             std::string const& /*filter*/,
             ShrinkableVector<ServiceReferenceBase>& references)
        {
            ASSERT_EQ(context, bundleCtx);

            references.clear();
            ordering.push_back(id);
        }

        static std::vector<int> ordering;
    };

    std::vector<int> TestServiceFindHook::ordering;

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
        int id;
        BundleContext bundleCtx;

      public:
        TestServiceListenerHook(int id, BundleContext const& context, std::shared_ptr<cppmicroservices::ServiceTracker<FooService>>_tracker) : id(id), bundleCtx(context), tracker(_tracker) {}

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
                ordering.push_back(id);
            }
        }

        void
        Removed(std::vector<ListenerInfo> const& listeners)
        {
            for (std::vector<ListenerInfo>::const_iterator iter = listeners.begin(); iter != listeners.end(); ++iter)
            {
                listenerInfos.erase(*iter);
                ordering.push_back(id * 10);
            }
            lastRemoved = listeners.back();
            tracker->Close();
        }

        static std::vector<int> ordering;

        std::unordered_set<ListenerInfo> listenerInfos;
        ListenerInfo lastAdded;
        ListenerInfo lastRemoved;
        std::shared_ptr<cppmicroservices::ServiceTracker<FooService>> tracker;
    };

    std::vector<int> TestServiceListenerHook::ordering;

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

    class ServiceHooksTrackerTest : public ::testing::Test
    {
      protected:
        Framework framework;
        BundleContext context;

      public:
        ServiceHooksTrackerTest() : framework(FrameworkFactory().NewFramework()) {};

        ~ServiceHooksTrackerTest() override = default;

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

TEST_F(ServiceHooksTrackerTest, TestBasic)
{
    auto customTracker = std::make_unique<CustomFooTracker>();
    auto tracker = std::make_shared<cppmicroservices::ServiceTracker<FooService>>(context, customTracker.get());
    tracker->Open();

    TestServiceListener serviceListener1;
    context.AddServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);

    auto serviceListenerHook1 = std::make_shared<TestServiceListenerHook>(1, context, tracker);
    ServiceProperties hookProps1;
    hookProps1[Constants::SERVICE_RANKING] = 0;
    ServiceRegistration<ServiceListenerHook> listenerHookReg1
        = context.RegisterService<ServiceListenerHook>(serviceListenerHook1, hookProps1);

    tracker->Close();
    listenerHookReg1.Unregister();
}

US_MSVC_POP_WARNING
