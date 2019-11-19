#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <chrono>
#include <future>

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

  static std::vector<cppmicroservices::Bundle> ConcurrentInstallHelper(
    ::cppmicroservices::Framework& framework,
    const std::vector<std::string>& bundlesToInstall)
  {
    using namespace cppmicroservices;
    auto fc = framework.GetBundleContext();
    std::vector<cppmicroservices::Bundle> bundles;
    for (size_t i = 0; i < bundlesToInstall.size(); i++) {
      auto bundle = testing::InstallLib(fc, bundlesToInstall[i]);
      bundles.push_back(bundle);
    }
    
    return bundles;
  }
  
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

  void InstallConcurrently(benchmark::State& state, uint32_t numThreads)
  {
    using namespace std::chrono;

    std::string bundleBasePath = "bundles\\test_bundle_";
    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();

    // Generate paths to each bundle
    std::vector<std::string> str5kBundles;
    for (uint32_t count = 1; count <= 5000; count++) {
      std::string copiedBundlePath = std::string(bundleBasePath);
      copiedBundlePath.append(std::to_string(count));
      str5kBundles.push_back(copiedBundlePath);
    }

    // Split up bundles per thread
    std::vector<std::vector<std::string>> bundlesToInstallPerThread;
    uint32_t currentIndex = 0;
    for (uint32_t i = 0; i < numThreads; i++) {
      std::vector<std::string> tempListOfBundles;
      if (i == numThreads - 1) {
        for (uint32_t j = 0; j < str5kBundles.size() / numThreads +
                                   (str5kBundles.size() % numThreads);
             j++, currentIndex++) {
          tempListOfBundles.push_back(str5kBundles[currentIndex]);
        }
      } else {
        for (uint32_t j = 0; j < str5kBundles.size() / numThreads;
             j++, currentIndex++) {
          tempListOfBundles.push_back(str5kBundles[currentIndex]);
        }
      }
      if (i == numThreads - 1) {
        tempListOfBundles.push_back(str5kBundles[0]);
      }
      bundlesToInstallPerThread.push_back(tempListOfBundles);
    }

    std::vector<std::future<std::vector<cppmicroservices::Bundle>>> results(numThreads);
    for (auto _ : state) {
      auto start = high_resolution_clock::now();
      for (uint32_t i = 0; i < bundlesToInstallPerThread.size(); i++) {
        results[i] = std::async(std::launch::async,
				ConcurrentInstallHelper,
                                std::ref(framework),
                                std::cref(bundlesToInstallPerThread[i]));
      }

      for (uint32_t i = 0; i < bundlesToInstallPerThread.size(); i++) {
        results[i].wait();
      }

      auto end = high_resolution_clock::now();
      auto elapsed_seconds = duration_cast<duration<double>>(end - start);
      state.SetIterationTime(elapsed_seconds.count());
    }

    framework.Stop();
    framework.WaitForStop(milliseconds::zero());
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

// Register functions as benchmark
BENCHMARK_REGISTER_F(BundleInstallFixture, BundleInstallCppFramework)->UseManualTime();
BENCHMARK_REGISTER_F(BundleInstallFixture, LargeBundleInstallCppFramework)->UseManualTime();
BENCHMARK_REGISTER_F(BundleInstallFixture, ConcurrentBundleInstall1Thread)->UseManualTime();
BENCHMARK_REGISTER_F(BundleInstallFixture, ConcurrentBundleInstall2Threads)->UseManualTime();
BENCHMARK_REGISTER_F(BundleInstallFixture, ConcurrentBundleInstall4Threads)->UseManualTime();
BENCHMARK_REGISTER_F(BundleInstallFixture, ConcurrentBundleInstallMaxThreads)->UseManualTime();
