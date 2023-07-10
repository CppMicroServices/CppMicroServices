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

    // Test failure modes for FindHo

    class TestServiceListenerHook : public ServiceListenerHook
    {
      private:
      public:
        TestServiceListenerHook(std::shared_ptr<cppmicroservices::ServiceTracker<FooService>> _tracker)
            : tracker(_tracker)
        {
        }

        void
        Added(std::vector<ListenerInfo> const& listeners)
        {
            US_UNUSED(listeners);
        }

        void
        Removed(std::vector<ListenerInfo> const& listeners)
        {
            US_UNUSED(listeners);
            tracker->Close();
        }
        std::shared_ptr<cppmicroservices::ServiceTracker<FooService>> tracker;
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

    auto serviceListenerHook1 = std::make_shared<TestServiceListenerHook>(tracker);
    ServiceProperties hookProps1;
    hookProps1[Constants::SERVICE_RANKING] = 0;
    ServiceRegistration<ServiceListenerHook> listenerHookReg1
        = context.RegisterService<ServiceListenerHook>(serviceListenerHook1, hookProps1);

    tracker->Close();
    listenerHookReg1.Unregister();
}

US_MSVC_POP_WARNING
