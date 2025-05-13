#include "../TestUtils.hpp"
#include <TestInterfaces/Interfaces.hpp>
#include <benchmark/benchmark.h>
#include <cassert>
#include <cppmicroservices/AnyMap.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/ServiceReference.h>
#include <cppmicroservices/cm/ConfigurationAdmin.hpp>

#include <chrono>
#include <memory>

namespace
{
    class GetConfigurationTest : public ::benchmark::Fixture
    {
      public:
        using benchmark::Fixture::SetUp;
        using benchmark::Fixture::TearDown;

        void
        SetUp(::benchmark::State const&)
        {
            using namespace cppmicroservices;
            framework = std::make_shared<Framework>(FrameworkFactory().NewFramework());
            framework->Start();

            context = framework->GetBundleContext();

            test::InstallAndStartDS(context);
            test::InstallAndStartConfigAdmin(context);
        }

        void
        TearDown(::benchmark::State const&)
        {
            framework->Stop();
            framework->WaitForStop(std::chrono::milliseconds::zero());
        }

        ~GetConfigurationTest() = default;

        std::shared_ptr<cppmicroservices::Framework> framework;
        cppmicroservices::BundleContext context;
    };

    BENCHMARK_DEFINE_F(GetConfigurationTest, createConfiguration)(benchmark::State& state)
    {
        auto const configAdmin
            = context.GetService(context.GetServiceReference<cppmicroservices::service::cm::ConfigurationAdmin>());

        size_t iter = 0;
        for (auto _ : state)
        {
            auto configuration = configAdmin->GetConfiguration("someConfig" + std::to_string(iter));
            cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
            props["anInt"] = 5;

            configuration->Update(props).get();
            iter += 1;
        }
    }

    BENCHMARK_DEFINE_F(GetConfigurationTest, updateConfigurationUsedByService)(benchmark::State& state)
    {
        auto const configAdmin
            = context.GetService(context.GetServiceReference<cppmicroservices::service::cm::ConfigurationAdmin>());
        auto bundle = test::InstallAndStartBundle(context, "TestBundleDSCA01");
        auto configuration = configAdmin->GetConfiguration("sample::ServiceComponentCA01");
        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        props["val"] = size_t(0);
        configuration->Update(props).get();

        auto service = context.GetService(context.GetServiceReference<test::CAInterface>());
        assert(0 == cppmicroservices::ref_any_cast<size_t>(service->GetProperties().find("val")->second));
        size_t iter = 0;
        for (auto _ : state)
        {
            cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
            props["val"] = iter;

            configuration->Update(props).get();
            iter += 1;
        }
    }
} // namespace

BENCHMARK_REGISTER_F(GetConfigurationTest, createConfiguration);
BENCHMARK_REGISTER_F(GetConfigurationTest, updateConfigurationUsedByService);
