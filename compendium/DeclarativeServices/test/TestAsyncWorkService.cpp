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

#include "gtest/gtest.h"

#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>

#include "ConcurrencyTestUtil.hpp"
#include "Mocks.hpp"
#include "TestInterfaces/Interfaces.hpp"
#include "TestUtils.hpp"
#include "boost/asio/async_result.hpp"
#include "boost/asio/packaged_task.hpp"
#include "boost/asio/post.hpp"
#include "boost/asio/thread_pool.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp"

#include <future>

namespace test {

namespace async = cppmicroservices::async::detail;
namespace scr = cppmicroservices::service::component::runtime;

class TestAsyncWorkService : public testing::Test
{
public:
  TestAsyncWorkService()
    : ::testing::Test()
    , framework(cppmicroservices::FrameworkFactory().NewFramework())
  {}

  //void TestBody() override {}

  void SetUp() override
  {
    framework.Start();
    context = framework.GetBundleContext();

#if defined(US_BUILD_SHARED_LIBS)
    auto dsPluginPath = test::GetDSRuntimePluginFilePath();
    auto dsbundles = context.InstallBundles(dsPluginPath);
    for (auto& bundle : dsbundles) {
      bundle.Start();
    }
#endif

#ifndef US_BUILD_SHARED_LIBS
    auto dsbundles = context.GetBundles();
    for (auto& bundle : dsbundles) {
      try {
        bundle.Start();
      } catch (std::exception& e) {
        std::cerr << "    " << e.what();
      }
      std::cerr << std::endl;
    }
#endif
    auto sRef = context.GetServiceReference<scr::ServiceComponentRuntime>();
    ASSERT_TRUE(sRef);
    dsRuntimeService = context.GetService<scr::ServiceComponentRuntime>(sRef);
    ASSERT_TRUE(dsRuntimeService);
  }

  void TearDown() override
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Bundle GetTestBundle(const std::string& symbolicName)
  {
    auto bundles = context.GetBundles();

    for (auto& bundle : bundles) {
      auto bundleSymbolicName = bundle.GetSymbolicName();
      if (symbolicName == bundleSymbolicName) {
        return bundle;
      }
    }
    return cppmicroservices::Bundle();
  }

  cppmicroservices::Bundle StartTestBundle(const std::string& symName)
  {
    cppmicroservices::Bundle testBundle = GetTestBundle(symName);
    EXPECT_EQ(static_cast<bool>(testBundle), true);
    testBundle.Start();
    EXPECT_EQ(testBundle.GetState(), cppmicroservices::Bundle::STATE_ACTIVE)
      << " failed to start bundle with symbolic name" + symName;
    return testBundle;
  }

  std::shared_ptr<scr::ServiceComponentRuntime> dsRuntimeService;
  cppmicroservices::Framework framework;
  cppmicroservices::BundleContext context;
};

class TestAsyncWorkServiceEndToEnd
  : public ::testing::TestWithParam<
      std::shared_ptr<cppmicroservices::async::detail::AsyncWorkService>>
{
public:
  TestAsyncWorkServiceEndToEnd()
    : framework(cppmicroservices::FrameworkFactory().NewFramework())
  {}

  void SetUp() override
  {
    framework.Start();
    auto context = framework.GetBundleContext();

    ::test::InstallAndStartDS(context);

    auto sRef = context.GetServiceReference<scr::ServiceComponentRuntime>();
    ASSERT_TRUE(sRef);
    dsRuntimeService = context.GetService<scr::ServiceComponentRuntime>(sRef);
    ASSERT_TRUE(dsRuntimeService);
  }

  void TearDown() override
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  std::shared_ptr<scr::ServiceComponentRuntime> dsRuntimeService;
  cppmicroservices::Framework framework;
};

class AsyncWorkServiceInline
  : public cppmicroservices::async::detail::AsyncWorkService
{
public:
  AsyncWorkServiceInline()
    : cppmicroservices::async::detail::AsyncWorkService()
  {}

  void post(std::packaged_task<void()>&& task) override { task(); }
};

class AsyncWorkServiceStdAsync
  : public cppmicroservices::async::detail::AsyncWorkService
{
public:
  AsyncWorkServiceStdAsync()
    : cppmicroservices::async::detail::AsyncWorkService()
  {}

  void post(std::packaged_task<void()>&& task) override
  {
    std::future<void> f = std::async(
      std::launch::async, [task = std::move(task)]() mutable { task(); });
  }
};

class AsyncWorkServiceThreadPool
  : public cppmicroservices::async::detail::AsyncWorkService
{
public:
  AsyncWorkServiceThreadPool(int nThreads)
    : cppmicroservices::async::detail::AsyncWorkService()
  {
    threadpool = std::make_shared<boost::asio::thread_pool>(nThreads);
  }

  ~AsyncWorkServiceThreadPool() override
  {
    try {
      if (threadpool) {
        try {
          threadpool->join();
        } catch (...) {
          //
        }
      }
    } catch (...) {
      //
    }
  }

  void post(std::packaged_task<void()>&& task) override
  {
    using Sig = void();
    using Result = boost::asio::async_result<decltype(task), Sig>;
    using Handler = typename Result::completion_handler_type;

    Handler handler(std::forward<decltype(task)>(task));
    Result result(handler);

    boost::asio::post(threadpool->get_executor(),
                      [handler = std::move(handler)]() mutable { handler(); });
  }

private:
  std::shared_ptr<boost::asio::thread_pool> threadpool;
};

TEST_F(TestAsyncWorkService, TestAsyncWorkServiceWithoutUserService)
{
  // Create SCRAsyncWorkService, just make sure Adding service isn't called, look
  // at SCRLoggerTest for example
  cppmicroservices::scrimpl::SCRAsyncWorkService scrAsyncWorkService(
    framework.GetBundleContext());
  EXPECT_NO_THROW({
    std::packaged_task<void()> myTask([]() {
      int v = 1 + 2;
      US_UNUSED(v);
    });
    std::future<void> f = myTask.get_future();
    scrAsyncWorkService.post(std::move(myTask));
    f.get();
  });
}

TEST_F(TestAsyncWorkService, TestUserServiceUsedAfterInstall)
{
  EXPECT_NO_THROW({
    auto mockAsyncWorkService =
      std::make_shared<cppmicroservices::async::MockAsyncWorkService>();
    auto bundleContext = framework.GetBundleContext();
    auto reg =
      bundleContext
        .RegisterService<cppmicroservices::async::detail::AsyncWorkService>(
          mockAsyncWorkService);
    EXPECT_CALL(*mockAsyncWorkService, post(::testing::_)).Times(1);

    cppmicroservices::scrimpl::SCRAsyncWorkService scrAsyncWorkService(
      bundleContext);

    std::packaged_task<void()> myTask([]() {
      int v = 1 + 2;
      US_UNUSED(v);
    });
    // We don't manage the future and wait because post is mocked and has no default behavior.
    scrAsyncWorkService.post(std::move(myTask));
  });
}

TEST_F(TestAsyncWorkService, TestFallbackUsedAfterUnregister)
{
  // don't hold onto the shared pointer to the service object, unregister, reset(), ...
  EXPECT_NO_THROW({
    auto mockAsyncWorkService =
      std::make_shared<cppmicroservices::async::MockAsyncWorkService>();
    auto bundleContext = framework.GetBundleContext();
    auto reg =
      bundleContext
        .RegisterService<cppmicroservices::async::detail::AsyncWorkService>(
          mockAsyncWorkService);
    EXPECT_CALL(*mockAsyncWorkService, post(::testing::_)).Times(1);

    cppmicroservices::scrimpl::SCRAsyncWorkService scrAsyncWorkService(
      bundleContext);

    std::packaged_task<void()> myTask([]() {
      int v = 1 + 2;
      US_UNUSED(v);
    });
    // We don't manage the future and wait because post is mocked and has no default behavior.
    scrAsyncWorkService.post(std::move(myTask));

    reg.Unregister();

    EXPECT_CALL(*mockAsyncWorkService, post(::testing::_)).Times(0);

    std::packaged_task<void()> myTask2([]() {
      int v = 1 + 2;
      US_UNUSED(v);
    });
    scrAsyncWorkService.post(std::move(myTask2));
  });
}

TEST_F(TestAsyncWorkService,
       TestUseAsyncWorkServiceDuringConcurrentBundleOperations)
{
  EXPECT_NO_THROW({
    auto bundleContext = framework.GetBundleContext();
    cppmicroservices::scrimpl::SCRAsyncWorkService scrAsyncWorkService(
      bundleContext);
    std::promise<void> startPromise;
    std::shared_future<void> start(startPromise.get_future());
    std::promise<void> stopPromise;
    std::shared_future<void> stop(stopPromise.get_future());
    int numThreads = 20;
    std::vector<std::promise<void>> readies(numThreads);
    std::vector<std::future<void>> asyncworkservice_futures(numThreads);
    try {
      for (int i = 0; i < numThreads; i++) {
        asyncworkservice_futures[i] =
          std::async(std::launch::async,
                     [&scrAsyncWorkService, i, start, stop, &readies]() {
                       readies[i].set_value();
                       start.wait();
                       do {
                         std::packaged_task<void()> task([]() {
                           int v = 1 + 2;
                           US_UNUSED(v);
                         });
                         scrAsyncWorkService.post(std::move(task));
                       } while (stop.wait_for(std::chrono::milliseconds(1)) !=
                                std::future_status::ready);
                     });
      }
      for (int i = 0; i < numThreads; i++) {
        readies[i].get_future().wait();
      }
      startPromise.set_value();

      auto serviceReg =
        std::async(std::launch::async, [start, stop, &bundleContext]() {
          start.wait();
          auto mockAsyncWorkService =
            std::make_shared<cppmicroservices::async::MockAsyncWorkService>();
          EXPECT_CALL(*mockAsyncWorkService, post(::testing::_))
            .Times(::testing::AtLeast(1));
          do {
            auto reg1 = bundleContext.RegisterService<
              cppmicroservices::async::detail::AsyncWorkService>(
              mockAsyncWorkService);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            reg1.Unregister();
          } while (stop.wait_for(std::chrono::milliseconds(1)) !=
                   std::future_status::ready);
        });
      std::this_thread::sleep_for(std::chrono::seconds(30));
      stopPromise.set_value();
      serviceReg.get();
      for (int i = 0; i < numThreads; i++) {
        asyncworkservice_futures[i].get();
      }
    } catch (...) {
      startPromise.set_value();
      stopPromise.set_value();
      throw;
    }
  });
}

INSTANTIATE_TEST_SUITE_P(
  AsyncWorkServiceEndToEndParameterized,
  TestAsyncWorkServiceEndToEnd,
  testing::Values(std::make_shared<AsyncWorkServiceInline>(),
                  std::make_shared<AsyncWorkServiceStdAsync>(),
                  std::make_shared<AsyncWorkServiceThreadPool>(1),
                  std::make_shared<AsyncWorkServiceThreadPool>(2)));

TEST_P(TestAsyncWorkServiceEndToEnd, TestEndToEndBehaviorWithAsyncWorkService)
{
  std::vector<std::string> bundlesToInstall = { "DSGraph01", "DSGraph02",
                                                "DSGraph03", "DSGraph04",
                                                "DSGraph05", "DSGraph06",
                                                "DSGraph07" };
  std::vector<cppmicroservices::Bundle> installedBundles;

  EXPECT_NO_THROW({
    auto const& param = GetParam();

    auto ctx = framework.GetBundleContext();

    auto reg =
      ctx.RegisterService<cppmicroservices::async::detail::AsyncWorkService>(
        param);

    for (const auto& bundleName : bundlesToInstall) {
      installedBundles.emplace_back(
        ::test::InstallAndStartBundle(ctx, bundleName));
    }

    for (auto& bundle : installedBundles) {
      bundle.Stop();
    }
  });
}

};
