#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <chrono>

#include "benchmark/benchmark.h"
#include "TestUtils.h"

class BundleInstallFixture
  : public ::benchmark::Fixture
{
public:
  using benchmark::Fixture::SetUp;
  using benchmark::Fixture::TearDown;

  void SetUp(const ::benchmark::State&)
  {
  }

  ~BundleInstallFixture() = default;
    
protected:
  void InstallWithCppFramework(benchmark::State& state, const std::string& bundleName)
  {
    using namespace std::chrono;
    using namespace cppmicroservices;
        
    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();
    auto context = framework.GetBundleContext();
    for (auto _ : state) {
      auto start   = high_resolution_clock::now();
      auto bundle  = testing::InstallLib(context, bundleName);
      auto end     = high_resolution_clock::now();
      auto elapsed = duration_cast<duration<double>>(end - start);
      state.SetIterationTime(elapsed.count());
#ifdef US_BUILD_SHARED_LIBS
      bundle.Uninstall();
#endif
    }

    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }
};
BENCHMARK_DEFINE_F(BundleInstallFixture, BundleInstallCppFramework)(benchmark::State& state)
{
  InstallWithCppFramework(state, "dummyService");
}

BENCHMARK_DEFINE_F(BundleInstallFixture, LargeBundleInstallCppFramework)(benchmark::State& state)
{
  InstallWithCppFramework(state, "largeBundle");
}

// Register functions as benchmark
BENCHMARK_REGISTER_F(BundleInstallFixture, BundleInstallCppFramework)->UseManualTime();
BENCHMARK_REGISTER_F(BundleInstallFixture, LargeBundleInstallCppFramework)->UseManualTime();
