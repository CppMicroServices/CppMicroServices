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

#include "LoggerFactoryImpl.hpp"
#include "LoggerImpl.hpp"

#include "cppmicroservices/logservice/LoggerFactory.hpp"
#include "cppmicroservices/logservice/Logger.hpp"

namespace ls = cppmicroservices::logservice;

static const std::string sinkFormat = "[%T] [%P:%t] %n (%^%l%$): %v";
static const std::string log_preamble("\\[([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})\\] "
                                      "\\[([0-9]{1,9}):([0-9]{1,9})\\] cppmicroservices::testing::logger "
                                      "\\((debug|trace|info|warning|error|critical)\\): ");
static const std::string svcRef_preamble("ServiceReference: ");
static const std::string exception_preamble("Exception logged: ");

class LoggerImplTests : public ::testing::Test
{
  public:
    LoggerImplTests()
    {
        std::shared_ptr<ls::LoggerFactory> lf = std::make_shared<ls::LoggerFactoryImpl>();
        _impl = lf->getLogger("cppmicroservices::testing::logger");
	_logimpl = std::dynamic_pointer_cast<ls::LoggerImpl>(_impl);
    }

    void
    SetUp() override
    {
        _sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
        _sink->set_pattern(sinkFormat);
        _logimpl->AddSink(_sink);
    }

    void
    TearDown() override
    {
        _sink.reset();
        _logimpl.reset();
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

    std::shared_ptr<ls::Logger>
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
    std::shared_ptr<ls::Logger> _impl;
    std::shared_ptr<ls::LoggerImpl> _logimpl;
};

TEST_F(LoggerImplTests, ProperLoggerUsage)
{
    std::shared_ptr<ls::Logger> logger = GetLogger();

    logger->audit("Hello!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hello!"));

    logger->audit("Hello {}", "World!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hello World!"));

    logger->audit("Hello {}{}", "World", "!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hello World!"));

    logger->audit("Test audit message", cppmicroservices::ServiceReferenceU {});
    EXPECT_TRUE(
        ContainsRegex(log_preamble + "Test audit message(\\n)" + svcRef_preamble + "Invalid service reference"));

    logger->audit("Test audit message",
                std::make_exception_ptr<std::runtime_error>(std::runtime_error("uh oh")));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test audit message(\\n)" + exception_preamble + "(.)+ uh oh"));

    logger->audit("Test audit message", cppmicroservices::ServiceReferenceU {},  std::make_exception_ptr<std::runtime_error>(std::runtime_error("uh oh")));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test audit message(\\n)" + svcRef_preamble + "Invalid service reference(\\n)" + exception_preamble + "(.)+ uh oh"));

    logger->debug("Hola!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola!"));

    logger->debug("Hola {}", "World!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola World!"));

    logger->debug("Hola {}{}", "World", "!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola World!"));

    logger->info("Hola!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola!"));

    logger->info("Hola {}", "World!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola World!"));

    logger->info("Hola {}{}", "World", "!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola World!"));

    logger->error("Hola!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola!"));

    logger->error("Hola {}", "World!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola World!"));

    logger->error("Hola {}{}", "World", "!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola World!"));

    logger->error("Test error message", cppmicroservices::ServiceReferenceU {});
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test error message(\\n)" + svcRef_preamble + "Invalid service reference"));

    logger->warn("Hola!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola!"));

    logger->warn("Hola {}", "World!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola World!"));

    logger->warn("Hola {}{}", "World", "!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola World!"));

    logger->warn("Test warning message",
                std::make_exception_ptr<std::runtime_error>(std::runtime_error("uh oh")));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test warning message(\\n)" + exception_preamble + "(.)+ uh oh"));

    logger->trace("Hola!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola!"));

    logger->trace("Hola {}", "World!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola World!"));

    logger->trace("Hola {}{}", "World", "!");
    EXPECT_TRUE(ContainsRegex(log_preamble + "Hola World!"));

    logger->trace("Test trace message",
                std::make_exception_ptr<std::runtime_error>(std::runtime_error("uh oh")));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test trace message(\\n)" + exception_preamble + "(.)+ uh oh"));

    logger->trace("Test trace message", cppmicroservices::ServiceReferenceU {});
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test trace message(\\n)" + svcRef_preamble + "Invalid service reference"));

    logger->trace("Test trace message", cppmicroservices::ServiceReferenceU {},  std::make_exception_ptr<std::runtime_error>(std::runtime_error("uh oh")));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test trace message(\\n)" + svcRef_preamble + "Invalid service reference(\\n)" + exception_preamble + "(.)+ uh oh"));

}

TEST_F(LoggerImplTests, InvalidLoggerUsage)
{
    std::shared_ptr<ls::Logger> logger = GetLogger();

    std::exception_ptr exp = nullptr;
    EXPECT_NO_THROW(logger->info("Test invalid exception_ptr", exp));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test invalid exception_ptr(\\n)" + exception_preamble + "none"));

    EXPECT_NO_THROW(logger->info("Test invalid ServiceReferenceBase object", cppmicroservices::ServiceReferenceU {},
                                nullptr));
    EXPECT_TRUE(ContainsRegex(log_preamble + "Test invalid ServiceReferenceBase object(\\n)" + svcRef_preamble
                              + "Invalid service reference(\\n)" + exception_preamble + "none"));
}

TEST_F(LoggerImplTests, ThreadSafety)
{
	std::shared_ptr<ls::Logger> logger = GetLogger();
    auto& oss = GetStream();

    int const iterations = 100;
    std::vector<std::thread> threads;
    for (int i = 0; i < iterations; i++)
    {
        threads.emplace_back(
            [&logger]()
            { logger->info("Test concurrent log calls");
            });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    std::regex regexp(log_preamble + "Test concurrent log calls");
    std::string stream(oss.str());
    auto regex_iter_end = std::sregex_iterator();

    auto regex_iter_begin = std::sregex_iterator(stream.begin(), stream.end(), regexp);
    std::ptrdiff_t num_found = std::distance(regex_iter_begin, regex_iter_end);
    ASSERT_TRUE(num_found == iterations);
}

