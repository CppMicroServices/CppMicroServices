#include <gtest/gtest.h>

#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/ServiceInterface.h>
#include <cppmicroservices/ServiceRegistration.h>

#include <chrono>
#include <memory>
#include <sstream>
#include <string>

#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

#include "LogServiceFactory.hpp"
#include "LogServiceImpl.hpp"

namespace ls = cppmicroservices::logservice;

namespace
{
    // Logs one message through the LogService the factory hands to the given bundle and
    // returns the name of the logger that wrote it.
    std::string
    LoggerNameFor(ls::impl::LogServiceFactory& factory, cppmicroservices::Bundle const& bundle)
    {
        auto const interfaces = factory.GetService(bundle, cppmicroservices::ServiceRegistrationU {});
        auto const service = std::dynamic_pointer_cast<ls::LogServiceImpl>(
            cppmicroservices::ExtractInterface<ls::LogService>(interfaces));
        if (!service)
        {
            return {};
        }

        std::ostringstream oss;
        spdlog::sink_ptr sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
        sink->set_pattern("%n|%v");
        service->AddSink(sink);
        service->Log(ls::SeverityLevel::LOG_INFO, "message");

        std::string const text = oss.str();
        return text.substr(0, text.find('|'));
    }
} // namespace

TEST(LogServiceFactoryTests, LoggerIsNamedAfterRequestingBundle)
{
    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();
    auto const bundle = framework.GetBundleContext().GetBundle();

    ls::impl::LogServiceFactory factory;
    EXPECT_EQ(ls::impl::LogServiceFactory::GetLoggerName(bundle), "LogService." + bundle.GetSymbolicName());
    EXPECT_EQ(LoggerNameFor(factory, bundle), "LogService." + bundle.GetSymbolicName());

    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
}

TEST(LogServiceFactoryTests, ProvidesBothInterfaces)
{
    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();
    auto const bundle = framework.GetBundleContext().GetBundle();

    ls::impl::LogServiceFactory factory;
    auto const interfaces = factory.GetService(bundle, cppmicroservices::ServiceRegistrationU {});
    ASSERT_TRUE(interfaces);
    EXPECT_TRUE(cppmicroservices::ExtractInterface<ls::LogService>(interfaces));
    EXPECT_TRUE(cppmicroservices::ExtractInterface<ls::LoggerFactory>(interfaces));

    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
}

// Registered through the framework, each GetService call from a bundle goes through the
// factory, and the registration is visible under both interface names.
TEST(LogServiceFactoryTests, RegistersAsServiceFactory)
{
    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();
    auto context = framework.GetBundleContext();

    auto registration = context.RegisterService<ls::LogService, ls::LoggerFactory>(
        std::static_pointer_cast<cppmicroservices::ServiceFactory>(std::make_shared<ls::impl::LogServiceFactory>()));

    auto const logServiceRef = context.GetServiceReference<ls::LogService>();
    ASSERT_TRUE(logServiceRef);
    ASSERT_TRUE(context.GetServiceReference<ls::LoggerFactory>());

    auto const service = std::dynamic_pointer_cast<ls::LogServiceImpl>(context.GetService(logServiceRef));
    ASSERT_TRUE(service);

    std::ostringstream oss;
    spdlog::sink_ptr sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    sink->set_pattern("%n|%v");
    service->AddSink(sink);
    service->Log(ls::SeverityLevel::LOG_INFO, "message");
    EXPECT_EQ(oss.str().rfind("LogService." + context.GetBundle().GetSymbolicName() + "|", 0), 0u);

    registration.Unregister();
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
}
