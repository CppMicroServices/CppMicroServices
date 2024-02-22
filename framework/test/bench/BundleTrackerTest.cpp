#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleTracker.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>

#include <chrono>

#include "TestUtils.h"
#include "benchmark/benchmark.h"

#ifdef GetObject
#  undef GetObject
#endif

using namespace cppmicroservices;

class BundleTrackerFixture : public ::benchmark::Fixture
{
public:
  using benchmark::Fixture::SetUp;
  using benchmark::Fixture::TearDown;

  void SetUp(const ::benchmark::State&)
  {
    framework = std::make_shared<Framework>(FrameworkFactory().NewFramework());
    framework->Start();
  }

  void TearDown(const ::benchmark::State&)
  {
    using namespace std::chrono;

    framework->Stop();
    framework->WaitForStop(milliseconds::zero());
  }

  ~BundleTrackerFixture() { framework.reset(); };

  std::shared_ptr<cppmicroservices::Framework> framework;
  static constexpr BundleTracker<>::BundleStateMaskType all_states =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE,
                                     Bundle::State::STATE_INSTALLED,
                                     Bundle::State::STATE_RESOLVED,
                                     Bundle::State::STATE_STARTING,
                                     Bundle::State::STATE_STOPPING,
                                     Bundle::State::STATE_UNINSTALLED);
};

BENCHMARK_DEFINE_F(BundleTrackerFixture, CreateBundleTracker)
(benchmark::State& state)
{
  auto context = framework->GetBundleContext();
  for (auto _ : state) {
    for (auto i = 0; i < state.range(0); ++i) {
      BundleTracker<> bundleTracker(context, all_states);
    }
  }
}

BENCHMARK_DEFINE_F(BundleTrackerFixture, OpenBundleTracker)
(benchmark::State& state)
{
  auto context = framework->GetBundleContext();
  BundleTracker<> bundleTracker(context, all_states);
  for (auto _ : state) {
    auto start = std::chrono::high_resolution_clock::now();
    bundleTracker.Open();
    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed_time =
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_time.count());

    bundleTracker.Close();
  }
}

BENCHMARK_DEFINE_F(BundleTrackerFixture, BundleTrackerGetObject)
(benchmark::State& state)
{
  auto context = framework->GetBundleContext();
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_INSTALLED);
  BundleTracker<> bundleTracker(context, stateMask);
  bundleTracker.Open();
  Bundle bundle = testing::InstallLib(context, "TestBundleA");

  for (auto _ : state) {
    auto start = std::chrono::high_resolution_clock::now();
    bundleTracker.GetObject(bundle);
    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed_time =
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_time.count());
  }

  bundleTracker.Close();
}

BENCHMARK_DEFINE_F(BundleTrackerFixture, BundleTrackerRemoveMethod)
(benchmark::State& state)
{
  auto context = framework->GetBundleContext();
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE);
  BundleTracker<> bundleTracker(context, stateMask);
  bundleTracker.Open();
  Bundle bundle = testing::InstallLib(context, "TestBundleA");

  for (auto _ : state) {
    // Make bundle enter tracked state
    bundle.Start();

    // Measure performance of Remove(bundle)
    auto start = std::chrono::high_resolution_clock::now();
    bundleTracker.Remove(bundle);
    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed_time =
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_time.count());

    // Make bundle leave tracked state
    bundle.Stop();
  }

  bundleTracker.Close();
}

BENCHMARK_DEFINE_F(BundleTrackerFixture, CloseBundleTracker)
(benchmark::State& state)
{
  auto context = framework->GetBundleContext();
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE);
  BundleTracker<> bundleTracker(context, stateMask);

  for (auto _ : state) {
    bundleTracker.Open();

    auto start = std::chrono::high_resolution_clock::now();
    bundleTracker.Close();
    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed_time =
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_time.count());
  }
}

BENCHMARK_DEFINE_F(BundleTrackerFixture, CloseBundleTrackerWithListeners)
(benchmark::State& state)
{
  auto context = framework->GetBundleContext();
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE);
  BundleTracker<> bundleTracker(context, stateMask);

  for (auto i = 0; i < state.range(0); ++i) {
    context.AddBundleListener([](const BundleEvent&) {});
  }

  for (auto _ : state) {
    bundleTracker.Open();

    auto start = std::chrono::high_resolution_clock::now();
    bundleTracker.Close();
    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed_time =
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_time.count());
  }
}

BENCHMARK_DEFINE_F(BundleTrackerFixture, StartBundle)
(benchmark::State& state)
{
  auto context = framework->GetBundleContext();
  Bundle bundle = testing::InstallLib(context, "TestBundleA");

  for (auto _ : state) {
    auto start = std::chrono::high_resolution_clock::now();
    bundle.Start();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_time =
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_time.count());

    bundle.Stop();
  }
}

BENCHMARK_DEFINE_F(BundleTrackerFixture, BundleTrackerScalability)
(benchmark::State& state)
{
  auto context = framework->GetBundleContext();
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE);

  // Create and open BundleTrackers
  auto maxBundleTrackers{ state.range(0) };
  std::vector<std::unique_ptr<BundleTracker<>>> trackers;
  for (auto i = 0; i < maxBundleTrackers; ++i) {
    auto bundleTracker = std::make_unique<BundleTracker<>>(context, stateMask);
    bundleTracker->Open();
    trackers.emplace_back(std::move(bundleTracker));
  }

  Bundle bundle = testing::InstallLib(context, "TestBundleA");
  for (auto _ : state) {
    // Measure performance of starting a bundle,
    // where each BundleTracker issues an AddingBundle callback
    auto start = std::chrono::high_resolution_clock::now();
    bundle.Start();
    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed_time =
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_time.count());

    bundle.Stop();
  }

  for (auto& tracker : trackers) {
    tracker->Close();
  }
}

#ifdef PERFORM_LARGE_BUNDLETRACKER_TEST
BENCHMARK_DEFINE_F(BundleTrackerFixture, BundleTrackerBundleScalability)
(benchmark::State& state)
{
  std::string bundleBasePath = "bundles\\bundle_";

  // Generate paths to each bundle
  uint32_t count = 1;
  std::vector<std::string> bundleStrs(state.range(0), bundleBasePath);
  std::transform(bundleStrs.begin(),
                 bundleStrs.end(),
                 bundleStrs.begin(),
                 [&count](std::string& s) -> std::string {
                   return s.append(std::to_string(count++));
                 });

  // Install all bundles
  std::vector<Bundle> bundles;
  for (auto bundleStr : bundleStrs) {
    bundles.emplace_back(testing::InstallLib(context, bundleStr));
  }

  auto context = framework->GetBundleContext();
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE,
                                     Bundle::State::STATE_RESOLVED,
                                     Bundle::State::STATE_STARTING);
  auto bundleTracker = BundleTracker<>(context, stateMask);
  bundleTracker.Open();

  // Measure performance of many lifecycle changes with a BundleTracker open
  for (auto _ : state) {
    for (auto bundle : bundles) {
      bundle.Start();
    }
    for (auto bundle : bundles) {
      bundle.Stop();
    }
  }
  bundleTracker.Close();
}
#endif

BENCHMARK_REGISTER_F(BundleTrackerFixture, CreateBundleTracker)
  ->Range(1, 10000);
BENCHMARK_REGISTER_F(BundleTrackerFixture, OpenBundleTracker)->UseManualTime();
BENCHMARK_REGISTER_F(BundleTrackerFixture, BundleTrackerGetObject)
  ->UseManualTime();
BENCHMARK_REGISTER_F(BundleTrackerFixture, BundleTrackerRemoveMethod)
  ->UseManualTime();
BENCHMARK_REGISTER_F(BundleTrackerFixture, CloseBundleTracker)->UseManualTime();
BENCHMARK_REGISTER_F(BundleTrackerFixture, CloseBundleTrackerWithListeners)
  ->RangeMultiplier(128)
  ->Range(1, 500000)
  ->UseManualTime();
BENCHMARK_REGISTER_F(BundleTrackerFixture, StartBundle)->UseManualTime();
BENCHMARK_REGISTER_F(BundleTrackerFixture, BundleTrackerScalability)
  ->RangeMultiplier(4)
  ->Range(0, 1000)
  ->UseManualTime();

#ifdef PERFORM_LARGE_BUNDLETRACKER_TEST
BENCHMARK_REGISTER_F(BundleTrackerFixture, BundleTrackerBundleScalability)
  ->RangeMultiplier(2)
  ->Range(1, 8000);
#endif
