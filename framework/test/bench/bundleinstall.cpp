#include <chrono>
#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <future>

#include "TestUtils.h"
#include "benchmark/benchmark.h"

class BundleInstallFixture : public ::benchmark::Fixture
{
public:
  using benchmark::Fixture::SetUp;
  using benchmark::Fixture::TearDown;

  void SetUp(const ::benchmark::State&) {}

  ~BundleInstallFixture() = default;

  static std::vector<cppmicroservices::Bundle> ConcurrentInstallHelper(
    cppmicroservices::Framework& framework,
    const std::vector<std::string>& bundlesToInstall)
  {
    using namespace cppmicroservices;
    auto fc = framework.GetBundleContext();

    std::vector<cppmicroservices::Bundle> bundles;
    for (const auto& b : bundlesToInstall) {
      auto bundle = testing::InstallLib(fc, b);
      bundles.push_back(bundle);
    }

    return bundles;
  }

protected:
  void InstallWithCppFramework(benchmark::State& state,
                               const std::string& bundleName)
  {
    using namespace std::chrono;
    using namespace cppmicroservices;

    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();
    auto context = framework.GetBundleContext();
    for (auto _ : state) {
      auto start = high_resolution_clock::now();
      auto bundle = testing::InstallLib(context, bundleName);
      auto end = high_resolution_clock::now();
      auto elapsed = duration_cast<duration<double>>(end - start);
      state.SetIterationTime(elapsed.count());
#ifdef US_BUILD_SHARED_LIBS
      bundle.Uninstall();
#endif
    }

    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  void InstallConcurrently(benchmark::State& state, uint32_t numThreads)
  {
    using namespace std::chrono;

    std::string bundleBasePath = "bundles\\test_bundle_";
    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();

    // Generate paths to each bundle
    uint32_t count = 1;
    std::vector<std::string> str5kBundles(5000, bundleBasePath);
    std::transform(str5kBundles.begin(), str5kBundles.end(), str5kBundles.begin(),
      [&count](std::string& s) -> std::string {
        return s.append(std::to_string(count++));
      });

    // Split up bundles per thread
    uint32_t numBundlesToInstall = uint32_t(str5kBundles.size()) / numThreads;
    std::vector<std::string>::iterator lowerBound = str5kBundles.begin();
    std::vector<std::string>::iterator upperBound = str5kBundles.begin() + numBundlesToInstall;

    std::vector<std::vector<std::string>> bundlesToInstallPerThread;
    for (uint32_t i = 0; i < numThreads; i++) {
      bundlesToInstallPerThread.push_back(
        std::vector<std::string>(lowerBound, upperBound));

      lowerBound = upperBound;
      if (i + 1 == numThreads - 1) {
        upperBound = str5kBundles.end();
      } else if (i + 1 < numThreads) {
        // Don't do this if it is the last iteration of the for-loop,
        // if performed, exception is thrown.
        upperBound += numBundlesToInstall;
      }
    }

    std::vector<std::future<std::vector<cppmicroservices::Bundle>>> results;
    for (auto _ : state) {
      auto start = high_resolution_clock::now();

      for (const auto& bundlesToInstall : bundlesToInstallPerThread) {
        results.push_back(std::async(std::launch::async,
                                     ConcurrentInstallHelper,
                                     std::ref(framework),
                                     std::cref(bundlesToInstall)));
      }

      for (auto& res : results) {
        res.wait();
      }

      auto end = high_resolution_clock::now();
      auto elapsed_seconds = duration_cast<duration<double>>(end - start);
      state.SetIterationTime(elapsed_seconds.count());
    }

    framework.Stop();
    framework.WaitForStop(milliseconds::zero());
  }
};
BENCHMARK_DEFINE_F(BundleInstallFixture, BundleInstallCppFramework)
(benchmark::State& state)
{
  InstallWithCppFramework(state, "dummyService");
}

BENCHMARK_DEFINE_F(BundleInstallFixture, LargeBundleInstallCppFramework)
(benchmark::State& state)
{
  InstallWithCppFramework(state, "largeBundle");
}

#if defined(PERFORM_LARGE_CONCURRENCY_TEST)
BENCHMARK_DEFINE_F(BundleInstallFixture, ConcurrentBundleInstall1Thread)
(benchmark::State& state)
{
  InstallConcurrently(state, 1);
}

BENCHMARK_DEFINE_F(BundleInstallFixture, ConcurrentBundleInstall2Threads)
(benchmark::State& state)
{
  InstallConcurrently(state, 2);
}

BENCHMARK_DEFINE_F(BundleInstallFixture, ConcurrentBundleInstall4Threads)
(benchmark::State& state)
{
  InstallConcurrently(state, 4);
}

BENCHMARK_DEFINE_F(BundleInstallFixture, ConcurrentBundleInstallMaxThreads)
(benchmark::State& state)
{
  InstallConcurrently(state, std::thread::hardware_concurrency());
}
#endif

// Register functions as benchmark
BENCHMARK_REGISTER_F(BundleInstallFixture, BundleInstallCppFramework)
  ->UseManualTime();
BENCHMARK_REGISTER_F(BundleInstallFixture, LargeBundleInstallCppFramework)
  ->UseManualTime();
#if defined(PERFORM_LARGE_CONCURRENCY_TEST)
BENCHMARK_REGISTER_F(BundleInstallFixture, ConcurrentBundleInstall1Thread)
  ->UseManualTime();
BENCHMARK_REGISTER_F(BundleInstallFixture, ConcurrentBundleInstall2Threads)
  ->UseManualTime();
BENCHMARK_REGISTER_F(BundleInstallFixture, ConcurrentBundleInstall4Threads)
  ->UseManualTime();
BENCHMARK_REGISTER_F(BundleInstallFixture, ConcurrentBundleInstallMaxThreads)
  ->UseManualTime();
#endif
