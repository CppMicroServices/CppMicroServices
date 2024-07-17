#include <chrono>
#include <future>
#include <memory>
#include <thread>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "../../src/util/CFRLogger.h"
#include "cppmicroservices/logservice/LogService.hpp"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

using cppmicroservices::logservice::LogService;
using cppmicroservices::logservice::SeverityLevel;
      
namespace cppmicroservices
{
    namespace cfrimpl
    {
        /**
         * This class is used in tests where the logger is required and the test
         * needs to verify what is sent to the logger
         */
         class MockLogger : public cppmicroservices::logservice::LogService
        {
          public:
            MOCK_METHOD2(Log, void(cppmicroservices::logservice::SeverityLevel, std::string const&));
            MOCK_METHOD3(Log,
                         void(cppmicroservices::logservice::SeverityLevel,
                              std::string const&,
                              std::exception_ptr const));
            MOCK_METHOD3(Log,
                         void(cppmicroservices::ServiceReferenceBase const&,
                              cppmicroservices::logservice::SeverityLevel,
                              std::string const&));
            MOCK_METHOD4(Log,
                         void(cppmicroservices::ServiceReferenceBase const&,
                              cppmicroservices::logservice::SeverityLevel,
                              std::string const&,
                              std::exception_ptr const));
            MOCK_CONST_METHOD1(getLogger, std::shared_ptr<cppmicroservices::logservice::Logger>(std::string const&));
            MOCK_CONST_METHOD2(getLogger,
                               std::shared_ptr<cppmicroservices::logservice::Logger>(cppmicroservices::Bundle const&,
                                                                                     std::string const&));
         };
    }
}

namespace cppmicroservices
{
    namespace cfrimpl
    {
        // The fixture for testing class CFRLogger.
        class CFRLoggerTest : public ::testing::Test
        {
          protected:
            CFRLoggerTest() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {}
            virtual ~CFRLoggerTest() = default;

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

	TEST_F(CFRLoggerTest, VerifyWithoutLoggerService)
        {
            cppmicroservices::cfrimpl::CFRLogger logger;
            cppmicroservices::ServiceReferenceU dummyRef;
            // check that calling log method is safe even if a LogService is unavailable
            EXPECT_NO_THROW({
                logger.Log(SeverityLevel::LOG_DEBUG, "sample log message");
                logger.Log(SeverityLevel::LOG_DEBUG,
                           "sample log message",
                           std::make_exception_ptr(std::runtime_error("error occured")));
                logger.Log(dummyRef, SeverityLevel::LOG_DEBUG, "sample log message");
                logger.Log(dummyRef,
                           SeverityLevel::LOG_DEBUG,
                           "sample log message",
                           std::make_exception_ptr(std::runtime_error("error occured")));
            });
        }

     TEST_F(CFRLoggerTest, VerifyWithLoggerService)
        {
            EXPECT_NO_THROW({
                // Register a mock logger implementaion
                auto mockLogger = std::make_shared<MockLogger>();
                auto bundleContext = GetFramework().GetBundleContext();
            
               auto reg = bundleContext.RegisterService<LogService>(mockLogger);
                // set expectations
                EXPECT_CALL(*(mockLogger.get()), Log(SeverityLevel::LOG_DEBUG, testing::_)).Times(1);
               EXPECT_CALL(*(mockLogger.get()), Log(SeverityLevel::LOG_ERROR, testing::_, testing::_)).Times(1);
               EXPECT_CALL(*(mockLogger.get()), Log(testing::_, SeverityLevel::LOG_WARNING, testing::_)).Times(1);
               EXPECT_CALL(*(mockLogger.get()), Log(testing::_, SeverityLevel::LOG_ERROR, testing::_, testing::_))
                   .Times(1);
               EXPECT_CALL(*(mockLogger.get()), getLogger("test")).Times(1);
               const cppmicroservices::Bundle mockBundle;
               EXPECT_CALL(*(mockLogger.get()), getLogger(mockBundle, "test")).Times(1);

                cppmicroservices::cfrimpl::CFRLogger logger(bundleContext);
                logger.Log(SeverityLevel::LOG_DEBUG, "test");
                logger.Log(SeverityLevel::LOG_ERROR,
                           "some sample error message",
                           std::make_exception_ptr(std::runtime_error("error occured")));
                cppmicroservices::ServiceReferenceU dummyRef;
                logger.Log(dummyRef, SeverityLevel::LOG_WARNING, "some sample warning message");
                logger.Log(dummyRef,
                           SeverityLevel::LOG_ERROR,
                           "some sample error message with service reference",
                           std::make_exception_ptr(std::runtime_error("error occured")));
                auto resultLogger = logger.getLogger("test");
                auto resultBundleLogger = logger.getLogger(mockBundle, "test");
            });
        }
    }
}




