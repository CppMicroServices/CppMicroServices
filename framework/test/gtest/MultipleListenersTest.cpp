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
#include "cppmicroservices/ServiceEvent.h"

#include "TestUtils.h"
#include "TestingConfig.h"
#include "gtest/gtest.h"

#include <array>
#include <bitset>
#include <future>

US_MSVC_PUSH_DISABLE_WARNING(4996)

using namespace cppmicroservices;

namespace
{
    enum ListenerType : short
    {
        CALLBACK1 = 0,
        CALLBACK2,
        CALLBACK3,
        FUNCTOR,
        MEMFN1,
        MEMFN2,
        LAMBDA1,
        LAMBDA2,
    };

    int count = 0;
    int const BITFIELD_LEN = 8;
    std::bitset<BITFIELD_LEN> bitfield(0x0);

    void
    callback_function_1(FrameworkEvent const&)
    {
        bitfield.set(CALLBACK1);
        ++count;
    }

    void
    callback_function_2(FrameworkEvent const&)
    {
        bitfield.set(CALLBACK2);
        ++count;
    }

    void
    callback_function_3(int val, FrameworkEvent const&)
    {
        US_UNUSED(val);
        bitfield.set(CALLBACK3);
        ++count;
    }

    class CallbackFunctor
    {
      public:
        void
        operator()(FrameworkEvent const&)
        {
            bitfield.set(FUNCTOR);
            ++count;
        }
    };

    class Listener
    {
      public:
        void
        memfn1(FrameworkEvent const&)
        {
            bitfield.set(MEMFN1);
            ++count;
        }

        void
        memfn2(FrameworkEvent const&)
        {
            bitfield.set(MEMFN2);
            ++count;
        }
    };

#ifdef US_ENABLE_THREADING_SUPPORT

    template <typename ListenerType>
    ListenerToken
    AddListener(BundleContext&, ListenerType)
    {
        return ListenerToken();
    }

    template <>
    ListenerToken
    AddListener(BundleContext& fCtx, FrameworkListener listener)
    {
        return fCtx.AddFrameworkListener(listener);
    }

    template <>
    ListenerToken
    AddListener(BundleContext& fCtx, ServiceListener listener)
    {
        return fCtx.AddServiceListener(listener);
    }

    template <>
    ListenerToken
    AddListener(BundleContext& fCtx, BundleListener listener)
    {
        return fCtx.AddBundleListener(listener);
    }

    // Test true concurrent addition and removal of listeners.
    // This is a better simulation of what we want to test - that only the API
    // calls that add listeners or remove listeners are executed at the same time,
    // and not other boilerplate code.
    template <typename ListenerType, typename EventType>
    void
    testConcurrentAddRemove()
    {
        FrameworkFactory factory;
        auto framework = factory.NewFramework();
        framework.Init();
        BundleContext fCtx = framework.GetBundleContext();

        int const numAdditions = 50;
        int const numRemovals = 30;
        std::vector<uint8_t> listenerFlags(numAdditions, 0);
        std::vector<ListenerToken> tokens;

        // Test concurrent addition
        // All threads wait at the 'ready.wait()' point inside the lambdas,
        // until the time when the promise 'go' is set by the main thread.
        {
            std::vector<std::future<ListenerToken>> futures;
            std::promise<void> go;
            std::shared_future<void> ready(go.get_future());
            std::vector<std::promise<void>> readies(numAdditions);

            auto addListener = [&fCtx, &readies, ready](std::vector<uint8_t>& flags, int i) -> ListenerToken
            {
                auto listener = [&flags, i](const EventType&) { flags[i] = 1; };
                readies[i].set_value();
                ready.wait();
                auto token = AddListener<ListenerType>(fCtx, listener);
                return token;
            };

            for (int i = 0; i < numAdditions; i++)
            {
                futures.push_back(std::async(std::launch::async, addListener, std::ref(listenerFlags), i));
            }
            for (auto& r : readies)
            {
                r.get_future().wait();
            }

            go.set_value();

            for (auto& future : futures)
            {
                tokens.push_back(future.get());
            }
        }

        // Test concurrent removal
        // All threads wait at the 'ready.wait()' point inside the lambdas,
        // until the time when the promise 'go' is set by the main thread.
        {
            std::vector<std::future<void>> futures;
            std::promise<void> go;
            std::shared_future<void> ready(go.get_future());
            std::vector<std::promise<void>> readies(numRemovals);

            // Using ListenerToken& because of VS2013 compiler bug
            // https://connect.microsoft.com/VisualStudio/feedback/details/884836
            auto removeListener = [&fCtx, &readies, ready](int i, ListenerToken& token)
            {
                readies[i].set_value();
                ready.wait();
                fCtx.RemoveListener(std::move(token));
            };

            for (int i = 0; i < numRemovals; i++)
            {
                futures.push_back(std::async(std::launch::async, removeListener, i, std::ref(tokens[i])));
            }
            for (auto& r : readies)
            {
                r.get_future().wait();
            }

            go.set_value();

            for (auto& future : futures)
            {
                future.get();
            }
        }

        framework.Start();
        auto bundleA = cppmicroservices::testing::InstallLib(fCtx, "TestBundleA");
        bundleA.Start();

        auto countOnes = std::count(listenerFlags.begin(), listenerFlags.end(), 1);

        ASSERT_EQ(countOnes, (numAdditions - numRemovals));
        bundleA.Stop();
        framework.Stop();
        framework.WaitForStop(std::chrono::seconds(0));
    }
#endif // US_ENABLE_THREADING_SUPPORT

    class MultipleListenersTest : public ::testing::Test
    {
      public:
        MultipleListenersTest() : framework(FrameworkFactory().NewFramework()) {}

        ~MultipleListenersTest() override
        {
            // when using gtest_repeat flag, this clears count within the
            // namespace to avoid failures on repetition.
            count = 0;
        }

        void
        SetUp() override
        {
            framework.Init();
            fCtx = framework.GetBundleContext();
        }

        void
        TearDown() override
        {
            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        }

      protected:
        Framework framework;
        BundleContext fCtx;
    };

} // namespace

TEST_F(MultipleListenersTest, testMultipleListeners)
{
    auto lambda1 = [](FrameworkEvent const&)
    {
        bitfield.set(LAMBDA1);
        ++count;
    };
    auto lambda2 = [](FrameworkEvent const&)
    {
        bitfield.set(LAMBDA2);
        ++count;
    };
    CallbackFunctor cb;
    Listener l1;
    Listener l2;

    // 1. Add all listeners
    fCtx.AddFrameworkListener(callback_function_1);
    fCtx.AddFrameworkListener(&callback_function_2);
    fCtx.AddFrameworkListener(&l1, &Listener::memfn1);
    fCtx.AddFrameworkListener(&l2, &Listener::memfn2);
    fCtx.AddFrameworkListener(cb);
    fCtx.AddFrameworkListener(lambda1);
    fCtx.AddFrameworkListener(lambda2);
    fCtx.AddFrameworkListener(CallbackFunctor());
    fCtx.AddFrameworkListener(std::bind(callback_function_3, 42, std::placeholders::_1));
    framework.Start(); // generate framework event (started)

    // Test if all the listeners are triggered.
    ASSERT_TRUE(bitfield.all());
    // Test the listeners count.
    ASSERT_EQ(count, 9);
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());

    // 2. Add all listeners and try removing listeners using their name
    // This removal using the names is deprecated and will be removed in the next major release.
    framework.Init();
    fCtx = framework.GetBundleContext();
    bitfield.reset();
    count = 0;
    // Add listeners of each variety
    fCtx.AddFrameworkListener(callback_function_1);
    fCtx.AddFrameworkListener(&callback_function_2);
    fCtx.AddFrameworkListener(&l1, &Listener::memfn1);
    fCtx.AddFrameworkListener(&l2, &Listener::memfn2);
    fCtx.AddFrameworkListener(cb);
    fCtx.AddFrameworkListener(lambda1);
    fCtx.AddFrameworkListener(lambda2);
    fCtx.AddFrameworkListener(CallbackFunctor());
    auto bind1 = std::bind(callback_function_3, 42, std::placeholders::_1);
    fCtx.AddFrameworkListener(bind1);
    // Remove listeners
    fCtx.RemoveFrameworkListener(callback_function_1);
    fCtx.RemoveFrameworkListener(&callback_function_2);
    fCtx.RemoveFrameworkListener(&l1, &Listener::memfn1);
    fCtx.RemoveFrameworkListener(&l2, &Listener::memfn2);
    fCtx.RemoveFrameworkListener(cb);
    fCtx.RemoveFrameworkListener(bind1);
    fCtx.RemoveFrameworkListener(lambda1);
    fCtx.RemoveFrameworkListener(lambda2);
    // Remove few listeners one more time.
    fCtx.RemoveFrameworkListener(callback_function_1);
    fCtx.RemoveFrameworkListener(&callback_function_2);
    fCtx.RemoveFrameworkListener(&l1, &Listener::memfn1);
    fCtx.RemoveFrameworkListener(&l2, &Listener::memfn2);
    fCtx.RemoveFrameworkListener(cb);
    fCtx.RemoveFrameworkListener(bind1);
    framework.Start(); // generate framework event (started)

    // Test if none of the listeners are registered.
    ASSERT_TRUE(bitfield.none());
    // Test the listeners count.
    ASSERT_EQ(count, 0);
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());

    // 3. Add all listeners and remove them using tokens
    framework.Init();
    fCtx = framework.GetBundleContext();
    auto token1 = fCtx.AddFrameworkListener(callback_function_1);
    auto token2 = fCtx.AddFrameworkListener(&callback_function_2);
    auto token3 = fCtx.AddFrameworkListener(&l1, &Listener::memfn1);
    auto token4 = fCtx.AddFrameworkListener(&l1, &Listener::memfn2);
    auto token5 = fCtx.AddFrameworkListener(&l2, &Listener::memfn1);
    auto token6 = fCtx.AddFrameworkListener(&l2, &Listener::memfn2);
    auto token7 = fCtx.AddFrameworkListener(cb);
    auto token8 = fCtx.AddFrameworkListener(lambda1);
    auto token9 = fCtx.AddFrameworkListener(lambda2);
    auto token10 = fCtx.AddFrameworkListener(CallbackFunctor());
    auto token11 = fCtx.AddFrameworkListener(bind1);
    // Remove all added listeners using tokens.
    fCtx.RemoveListener(std::move(token1));
    fCtx.RemoveListener(std::move(token2));
    fCtx.RemoveListener(std::move(token3));
    fCtx.RemoveListener(std::move(token4));
    fCtx.RemoveListener(std::move(token5));
    fCtx.RemoveListener(std::move(token6));
    fCtx.RemoveListener(std::move(token7));
    fCtx.RemoveListener(std::move(token8));
    fCtx.RemoveListener(std::move(token9));
    fCtx.RemoveListener(std::move(token10));
    fCtx.RemoveListener(std::move(token11));
    // Remove all added listeners again using the tokens. These should all be no-op.
    fCtx.RemoveListener(std::move(token1));
    fCtx.RemoveListener(std::move(token2));
    fCtx.RemoveListener(std::move(token3));
    fCtx.RemoveListener(std::move(token4));
    fCtx.RemoveListener(std::move(token5));
    fCtx.RemoveListener(std::move(token6));
    fCtx.RemoveListener(std::move(token7));
    fCtx.RemoveListener(std::move(token8));
    fCtx.RemoveListener(std::move(token9));
    fCtx.RemoveListener(std::move(token10));
    fCtx.RemoveListener(std::move(token11));
    // This should result in no output because all the listeners were successfully removed
    framework.Start(); // generate framework event (started)

    // Test if none of the listeners are registered.
    ASSERT_TRUE(bitfield.none());
    // Test the listeners count.
    ASSERT_EQ(count, 0);
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());

    // 4. Test the move ability
    framework.Init();
    fCtx = framework.GetBundleContext();
    token1 = fCtx.AddFrameworkListener(callback_function_1);
    // Check validity of a token1
    ASSERT_TRUE(token1);
    token2 = fCtx.AddFrameworkListener(&callback_function_2);
    token3 = fCtx.AddFrameworkListener(&l1, &Listener::memfn1);
    token3 = fCtx.AddFrameworkListener(&l2, &Listener::memfn2);
    token4 = std::move(token1); // move assignment
    // Check invalidity of a moved - from token1
    ASSERT_FALSE(token1);
    auto token2_(std::move(token2)); // move construction
    // Check invalidity of a moved-from token2
    ASSERT_FALSE(token2);
    ListenerToken emptytoken; // default construction
    // Check invalidity of a newly constructed token
    ASSERT_FALSE(emptytoken);
    fCtx.RemoveListener(std::move(token4));
    fCtx.RemoveListener(std::move(token2_));
    fCtx.RemoveListener(std::move(emptytoken)); // This should do nothing.
    framework.Start();                          // generate framework event (started)
    // Test if only member functions 1 & 2 are registered.
    ASSERT_EQ(bitfield.to_string(), "00110000");
    // Test the listeners count.
    ASSERT_EQ(count, 2);
}

TEST_F(MultipleListenersTest, testListenerTypes)
{
    class TestListener
    {
      public:
        TestListener() : service_count(0), bundle_count(0), framework_count(0) {}

        void
        serviceChanged(ServiceEvent const& /*evt*/)
        {
            ++service_count;
        }
        void
        bundleChanged(BundleEvent const& /*evt*/)
        {
            ++bundle_count;
        }
        void
        frameworkChanged(FrameworkEvent const& /*evt*/)
        {
            ++framework_count;
        }

        std::vector<ListenerToken> tokens;
        int service_count;
        int bundle_count;
        int framework_count;
    };

    auto framework2 = FrameworkFactory().NewFramework();

    framework2.Init();
    auto fCtx = framework2.GetBundleContext();

    TestListener tListen;

    tListen.tokens.push_back(fCtx.AddServiceListener(&tListen, &TestListener::serviceChanged));
    tListen.tokens.push_back(fCtx.AddServiceListener(&tListen, &TestListener::serviceChanged));
    tListen.tokens.push_back(fCtx.AddBundleListener(&tListen, &TestListener::bundleChanged));
    tListen.tokens.push_back(fCtx.AddBundleListener(&tListen, &TestListener::bundleChanged));
    tListen.tokens.push_back(fCtx.AddFrameworkListener(&tListen, &TestListener::frameworkChanged));
    tListen.tokens.push_back(fCtx.AddFrameworkListener(&tListen, &TestListener::frameworkChanged));

    fCtx.RemoveListener(std::move(tListen.tokens[0]));
    fCtx.RemoveListener(std::move(tListen.tokens[2]));
    fCtx.RemoveListener(std::move(tListen.tokens[4]));
    framework2.Start();

    auto bundleA = cppmicroservices::testing::InstallLib(fCtx, "TestBundleA");
    // Test for existing bundle TestBundleA
    ASSERT_TRUE(bundleA);
    bundleA.Start();

    // Test for number of times service listeners got triggered
    ASSERT_EQ(tListen.service_count, 1);
    // Test for number of times bundle listeners got triggered
    EXPECT_GT(static_cast<int>(tListen.bundle_count), 0);
    // Test for number of times framework listeners got triggered
    ASSERT_EQ(tListen.framework_count, 1);

    framework2.Stop();
    framework2.WaitForStop(std::chrono::milliseconds::zero());

#ifdef US_ENABLE_THREADING_SUPPORT
    testConcurrentAddRemove<FrameworkListener, FrameworkEvent>();
    testConcurrentAddRemove<BundleListener, BundleEvent>();
    testConcurrentAddRemove<ServiceListener, ServiceEvent>();
#endif
}

#ifdef US_ENABLE_THREADING_SUPPORT
// Test the addition of thousand listeners asynchronously.
TEST_F(MultipleListenersTest, testConcurrentAdd)
{
    BundleContext fCtx = framework.GetBundleContext();

    int const numAdditions = 1001;
    std::vector<ListenerToken> tokens;
    std::vector<std::future<ListenerToken>> futures;
    int count = 0;

    auto addListener = [&fCtx, &count]() -> ListenerToken
    {
        auto listener = [&count](const FrameworkEvent&) { ++count; };
        auto token = fCtx.AddFrameworkListener(listener);
        return token;
    };

    for (int i = 0; i < numAdditions; i++)
    {
        futures.push_back(std::async(std::launch::async, addListener));
    }
    for (auto& future : futures)
    {
        tokens.push_back(future.get());
    }
    for (auto& token : tokens)
    {
        fCtx.RemoveListener(std::move(token));
    }

    framework.Start();
    ASSERT_EQ(count, 0);
}

#endif

US_MSVC_POP_WARNING
