#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <gtest/gtest.h>

#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/util/FileSystem.h>

#include <chrono>
#include <functional>
#include <limits>
#include <memory>
#include <ostream>
#include <regex>
#include <string>
#include <thread>
#include <utility>

#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

#include "LogServiceImpl.hpp"

namespace ls = cppmicroservices::logservice;

static const std::string sinkFormat = "[%T] [%P:%t] %n (%^%l%$): %v";
static const std::string log_preamble("\\[([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})\\] "
                                      "\\[([0-9]{1,9}):([0-9]{1,9})\\] cppmicroservices::testing::logservice "
                                      "\\((debug|trace|info|warning|error)\\): ");
static const std::string svcRef_preamble("ServiceReference: ");
static const std::string exception_preamble("Exception logged: ");

class LogServiceImplTests : public ::testing::Test
{
  public:
    LogServiceImplTests() { _impl = std::make_shared<ls::LogServiceImpl>("cppmicroservices::testing::logservice"); }

    void
    SetUp() override
    {
        _sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
        _sink->set_pattern(sinkFormat);
        _impl->AddSink(_sink);
    }

    void
    TearDown() override
    {
        _sink.reset();
        _impl.reset();
    }

    bool
    ContainsRegex(std::string const& regex)
    {
        std::string text = oss.str();
        std::smatch m;
        bool found = std::regex_search(text, m, std::regex(regex));
        oss.str("");
        return found;
    }

    std::shared_ptr<ls::LogServiceImpl>
    GetLogger()
    {
        return _impl;
    }
    std::ostringstream&
    GetStream()
    {
        return oss;
    }

  private:
    std::ostringstream oss;
    spdlog::sink_ptr _sink;
    std::shared_ptr<ls::LogServiceImpl> _impl;
};

TEST_F(LogServiceImplTests, ProperLoggerUsage)
{
    auto logger = GetLogger();

    logger->Log(ls::SeverityLevel::LOG_DEBUG, "Bonjour!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Bonjour!"));

    logger->Log(ls::SeverityLevel::LOG_INFO, "Hola!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola!"));

    logger->Log(ls::SeverityLevel::LOG_WARNING, "Hello!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hello!"));

    logger->Log(ls::SeverityLevel::LOG_ERROR, "Salve!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Salve!"));

    logger->Log(cppmicroservices::ServiceReferenceU {}, ls::SeverityLevel::LOG_DEBUG, "Test debug message");
    EXPECT_TRUE(
        ContainsRegex(log_preamble + "Test debug message(\\n)" + svcRef_preamble + "Invalid service reference"));

    logger->Log(cppmicroservices::ServiceReferenceU {}, ls::SeverityLevel::LOG_INFO, "Test info message");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test info message(\\n)" + svcRef_preamble + "Invalid service reference"));

    logger->Log(cppmicroservices::ServiceReferenceU {}, ls::SeverityLevel::LOG_WARNING, "Test warning message");
    EXPECT_TRUE(
        ContainsRegex(log_preamble + "Test warning message(\\n)" + svcRef_preamble + "Invalid service reference"));

    logger->Log(cppmicroservices::ServiceReferenceU {}, ls::SeverityLevel::LOG_ERROR, "Test error message");
    EXPECT_TRUE(
        ContainsRegex(log_preamble + "Test error message(\\n)" + svcRef_preamble + "Invalid service reference"));

    logger->Log(ls::SeverityLevel::LOG_DEBUG,
                "Test debug message",
                std::make_exception_ptr<std::runtime_error>(std::runtime_error("uh oh")));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test debug message(\\n)" + exception_preamble + "(.)+ uh oh"));

    logger->Log(ls::SeverityLevel::LOG_INFO,
                "Test info message",
                std::make_exception_ptr<std::runtime_error>(std::runtime_error("uh oh")));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test info message(\\n)" + exception_preamble + "(.)+ uh oh"));

    logger->Log(ls::SeverityLevel::LOG_WARNING,
                "Test warning message",
                std::make_exception_ptr<std::runtime_error>(std::runtime_error("uh oh")));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test warning message(\\n)" + exception_preamble + "(.)+ uh oh"));

    logger->Log(ls::SeverityLevel::LOG_ERROR,
                "Test error message",
                std::make_exception_ptr<std::runtime_error>(std::runtime_error("uh oh")));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test error message(\\n)" + exception_preamble + "(.)+ uh oh"));

    logger->Log(cppmicroservices::ServiceReferenceU {},
                ls::SeverityLevel::LOG_DEBUG,
                "Test debug message",
                std::exception_ptr {});
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test debug message(\\n)" + svcRef_preamble
                              + "Invalid service reference(\\n)" + exception_preamble + "none"));

    logger->Log(cppmicroservices::ServiceReferenceU {},
                ls::SeverityLevel::LOG_INFO,
                "Test info message",
                std::exception_ptr {});
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test info message(\\n)" + svcRef_preamble
                              + "Invalid service reference(\\n)" + exception_preamble + "none"));

    logger->Log(cppmicroservices::ServiceReferenceU {},
                ls::SeverityLevel::LOG_WARNING,
                "Test warning message",
                std::exception_ptr {});
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test warning message(\\n)" + svcRef_preamble
                              + "Invalid service reference(\\n)" + exception_preamble + "none"));

    logger->Log(cppmicroservices::ServiceReferenceU {},
                ls::SeverityLevel::LOG_ERROR,
                "Test error message",
                std::exception_ptr {});
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test error message(\\n)" + svcRef_preamble
                              + "Invalid service reference(\\n)" + exception_preamble + "none"));
}

TEST_F(LogServiceImplTests, InvalidLoggerUsage)
{
    auto logger = GetLogger();

    ASSERT_NO_THROW(logger->Log(ls::SeverityLevel::LOG_INFO, "Test invalid exception_ptr", nullptr));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test invalid exception_ptr(\\n)" + exception_preamble + "none"));

    ASSERT_NO_THROW(logger->Log(static_cast<ls::SeverityLevel>(-1), "Test invalid negative severity level"));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test invalid negative severity level")); 

    logger->Log(static_cast<ls::SeverityLevel>(-1), "Test invalid negative severity level");
    EXPECT_TRUE(ContainsRegex("trace"));

    ASSERT_NO_THROW(logger->Log(static_cast<ls::SeverityLevel>(std::numeric_limits<unsigned int>::max()),
                                "Test invalid maximum severity level"));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test invalid maximum severity level"));

    logger->Log(static_cast<ls::SeverityLevel>(std::numeric_limits<unsigned int>::max()),
                                "Test invalid maximum severity level");
    EXPECT_TRUE(ContainsRegex("trace"));

    ASSERT_NO_THROW(logger->Log(cppmicroservices::ServiceReferenceU {},
                                ls::SeverityLevel::LOG_INFO,
                                "Test invalid ServiceReferenceBase object",
                                nullptr));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test invalid ServiceReferenceBase object(\\n)" + svcRef_preamble
                              + "Invalid service reference(\\n)" + exception_preamble + "none"));
}

TEST_F(LogServiceImplTests, ThreadSafety)
{
    auto logger = GetLogger();
    auto& oss = GetStream();

    int const iterations = 100;
    std::vector<std::thread> threads;
    for (int i = 0; i < iterations; i++)
    {
        threads.push_back(std::thread(
            [&logger]()
            {
                logger->Log(cppmicroservices::ServiceReferenceU {},
                            ls::SeverityLevel::LOG_INFO,
                            "Test concurrent log calls",
                            nullptr);
            }));
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    std::regex regexp(log_preamble + "Test concurrent log calls(\\n)" + svcRef_preamble
                      + "Invalid service reference(\\n)" + exception_preamble + "none");
    std::string stream(oss.str());
    auto regex_iter_end = std::sregex_iterator();

    auto regex_iter_begin = std::sregex_iterator(stream.begin(), stream.end(), regexp);
    std::ptrdiff_t num_found = std::distance(regex_iter_begin, regex_iter_end);
    ASSERT_TRUE(num_found == iterations);
}
