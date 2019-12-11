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

#include <chrono>
#include <thread>
#include <memory>
#include <future>

#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/BundleContext.h"
#include "Mocks.hpp"
#include "../src/SCRLogger.hpp"

using cppmicroservices::logservice::LogService;
using cppmicroservices::logservice::SeverityLevel;

namespace cppmicroservices {
namespace scrimpl {

// The fixture for testing class SCRLogger.
class SCRLoggerTest
  : public ::testing::Test
{
protected:
  SCRLoggerTest() : framework(cppmicroservices::FrameworkFactory().NewFramework())
  { }
  virtual ~SCRLoggerTest() = default;

  virtual void SetUp() {
    framework.Start();
  }

  virtual void TearDown() {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Framework& GetFramework() { return framework; }
private:
  cppmicroservices::Framework framework;
};

TEST_F(SCRLoggerTest, VerifyWithoutLoggerService)
{
  auto bundleContext = GetFramework().GetBundleContext();
  cppmicroservices::scrimpl::SCRLogger logger(bundleContext);
  cppmicroservices::ServiceReferenceU dummyRef;
  // check that calling log method is safe even if a LogService is unavailable
  EXPECT_NO_THROW({
      logger.Log(SeverityLevel::LOG_DEBUG, "sample log message");
      logger.Log(SeverityLevel::LOG_DEBUG, "sample log message", std::make_exception_ptr(std::runtime_error("error occured")));
      logger.Log(dummyRef, SeverityLevel::LOG_DEBUG, "sample log message");
      logger.Log(dummyRef, SeverityLevel::LOG_DEBUG, "sample log message", std::make_exception_ptr(std::runtime_error("error occured")));
    });
}

TEST_F(SCRLoggerTest, VerifyWithLoggerService)
{
  EXPECT_NO_THROW({
      // Register a mock logger implementaion
      auto mockLogger = std::make_shared<MockLogger>();
      auto bundleContext = GetFramework().GetBundleContext();
      auto reg = bundleContext.RegisterService<LogService>(mockLogger);
      // set expectations
      EXPECT_CALL(*(mockLogger.get()), Log(SeverityLevel::LOG_DEBUG, testing::_))
        .Times(1);
      EXPECT_CALL(*(mockLogger.get()), Log(SeverityLevel::LOG_ERROR, testing::_, testing::_))
        .Times(1);
      EXPECT_CALL(*(mockLogger.get()), Log(testing::_, SeverityLevel::LOG_WARNING, testing::_))
        .Times(1);
      EXPECT_CALL(*(mockLogger.get()), Log(testing::_, SeverityLevel::LOG_ERROR, testing::_, testing::_))
        .Times(1);
      // exercise methods on instance of SCRLogger
      cppmicroservices::scrimpl::SCRLogger logger(bundleContext);
      logger.Log(SeverityLevel::LOG_DEBUG, "some sample debug message");
      logger.Log(SeverityLevel::LOG_ERROR, "some sample error message", std::make_exception_ptr(std::runtime_error("error occured")));
      cppmicroservices::ServiceReferenceU dummyRef;
      logger.Log(dummyRef, SeverityLevel::LOG_WARNING, "some sample warning message");
      logger.Log(dummyRef, SeverityLevel::LOG_ERROR, "some sample error message with service reference", std::make_exception_ptr(std::runtime_error("error occured")));
    });
}

TEST_F(SCRLoggerTest, VerifyLoggerServiceStaticBinding)
{
  EXPECT_NO_THROW({
      auto mockLogger1 = std::make_shared<MockLogger>();
      auto mockLogger2 = std::make_shared<MockLogger>();
      auto mockLogger3 = std::make_shared<MockLogger>();
      // setup expectations.
      // mockLogger1 received first two calls because it is registered first.
      // mockLogger2 does not receive any calls because it is never bound to SCRLogger.
      // mockLogger3 receives 1 call because it is registered after mockLogger1 is unbound.
      EXPECT_CALL(*(mockLogger1.get()), Log(SeverityLevel::LOG_DEBUG, testing::_))
        .Times(2);
      EXPECT_CALL(*(mockLogger2.get()), Log(SeverityLevel::LOG_DEBUG, testing::_))
        .Times(0);
      EXPECT_CALL(*(mockLogger3.get()), Log(SeverityLevel::LOG_DEBUG, "3. sample debug message"))
        .Times(1);
      auto bundleContext = GetFramework().GetBundleContext();
      cppmicroservices::scrimpl::SCRLogger logger(bundleContext);
      auto reg1 = bundleContext.RegisterService<LogService>(mockLogger1);
      auto reg2 = bundleContext.RegisterService<LogService>(mockLogger2);
      logger.Log(SeverityLevel::LOG_DEBUG, "1. sample debug message");
      logger.Log(SeverityLevel::LOG_DEBUG, "2. sample debug message");
      reg1.Unregister();
      auto reg3 = bundleContext.RegisterService<LogService>(mockLogger3);
      logger.Log(SeverityLevel::LOG_DEBUG, "3. sample debug message");
      reg2.Unregister();
      reg3.Unregister();
    });
}

TEST_F(SCRLoggerTest, VerifyMultiThreadedAccess)
{
  // log from multiple threads while one thread is continously registering
  // and unregistering the log service.
  EXPECT_NO_THROW({
      auto bundleContext = GetFramework().GetBundleContext();
      cppmicroservices::scrimpl::SCRLogger logger(bundleContext);
      std::promise<void> startPromise;
      std::shared_future<void> start(startPromise.get_future());
      std::promise<void> stopPromise;
      std::shared_future<void> stop(stopPromise.get_future());
      int numThreads = 20;
      std::vector<std::promise<void>> readies(numThreads);
      std::vector<std::future<void>> logging_futures(numThreads);
      try
      {
        for(int i =0; i<numThreads; i++)
        {
          // launch a task to continously log until stop signal is received
          logging_futures[i] = std::async(std::launch::async,
                                          [&logger, i, start, stop, &readies]() {
                                            readies[i].set_value();
                                            start.wait();
                                            do
                                            {
                                              logger.Log(SeverityLevel::LOG_DEBUG, "sample debug message");
                                            }while(stop.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready);
                                          });
        }
        for(int i =0; i< numThreads; i++)
        {
          readies[i].get_future().wait();
        }
        startPromise.set_value();
        // on a separate thread, register and unregister mock logger service.
        auto serviceReg = std::async(std::launch::async,
                                     [start, stop, &bundleContext]() {
                                       start.wait();
                                       auto mockLogger = std::make_shared<MockLogger>();
                                       EXPECT_CALL(*(mockLogger.get()), Log(SeverityLevel::LOG_DEBUG, testing::_))
                                         .Times(testing::AtLeast(1));
                                       do
                                       {
                                         auto reg1 = bundleContext.RegisterService<LogService>(mockLogger);
                                         std::this_thread::sleep_for(std::chrono::seconds(1));
                                         reg1.Unregister();
                                       }while(stop.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready);
                                     });
        std::this_thread::sleep_for(std::chrono::seconds(30));
        stopPromise.set_value(); // stop all threads
        serviceReg.get();
        for(int i =0; i< numThreads; i++)
        {
          logging_futures[i].get();
        }
      }
      catch(...)
      {
        startPromise.set_value();
        stopPromise.set_value();
        throw;
      }
    });
  
}
}
}
