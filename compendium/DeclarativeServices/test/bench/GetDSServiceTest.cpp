#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/ServiceReference.h>

#include "../TestUtils.hpp"
#include <TestInterfaces/Interfaces.hpp>

#include <benchmark/benchmark.h>

#include <chrono>
#include <memory>

namespace
{
    class GetDSServiceFixture : public ::benchmark::Fixture
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
        }

        void
        TearDown(::benchmark::State const&)
        {
            framework->Stop();
            framework->WaitForStop(std::chrono::milliseconds::zero());
        }

        ~GetDSServiceFixture() = default;

        std::shared_ptr<cppmicroservices::Framework> framework;
        cppmicroservices::BundleContext context;
    };

    BENCHMARK_DEFINE_F(GetDSServiceFixture, GetService)(benchmark::State& state)
    {
        auto bundle = test::InstallAndStartBundle(context, "TestBundleDSTOI1");
        for (auto _ : state)
        {
            auto sRef = context.GetServiceReference<test::Interface1>();
            (void)context.GetService<test::Interface1>(sRef);
        }
        bundle.Stop();
    }
} // namespace

BENCHMARK_REGISTER_F(GetDSServiceFixture, GetService);
