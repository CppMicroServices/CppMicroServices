#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/ServiceTracker.h>

#include <chrono>

#include "benchmark/benchmark.h"
#include "fooservice.h"


class ServiceTrackerFixture : public ::benchmark::Fixture
{
public:
  using benchmark::Fixture::SetUp;
  using benchmark::Fixture::TearDown;
    
  void SetUp(const ::benchmark::State&)
  {
    using namespace cppmicroservices;
    
    framework = std::make_shared<Framework>(FrameworkFactory().NewFramework());
    framework->Start();
  }

  void TearDown(const ::benchmark::State&)
  {
    using namespace std::chrono;
    
    framework->Stop();
    framework->WaitForStop(milliseconds::zero());
  }

  ~ServiceTrackerFixture() { framework.reset(); };

  std::shared_ptr<cppmicroservices::Framework> framework;
};

/// Benchmark how long it takes to open a service tracker using a service reference
BENCHMARK_DEFINE_F(ServiceTrackerFixture, OpenServiceTrackerWithSvcRef)(benchmark::State& state)
{
  using namespace std::chrono;
  using namespace benchmark::test;
  using namespace cppmicroservices;
  
  auto fc = framework->GetBundleContext();
  auto serviceReg = fc.RegisterService<Foo>(std::make_shared<FooImpl>());
  ServiceTracker<Foo> fooTracker(fc, serviceReg.GetReference<Foo>());
  for (auto _ : state) {
    auto start = high_resolution_clock::now();
    fooTracker.Open();
    auto end = high_resolution_clock::now();
    auto elapsed_seconds = duration_cast<duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());

    fooTracker.Close();
  }
}

/// Benchmark how long it takes to open a service tracker using a bundle context
BENCHMARK_DEFINE_F(ServiceTrackerFixture, OpenServiceTrackerWithBundleContext)(benchmark::State& state)
{
  using namespace std::chrono;
  using namespace benchmark::test;
  using namespace cppmicroservices;
  
  auto fc = framework->GetBundleContext();
  auto serviceReg = fc.RegisterService<Foo>(std::make_shared<FooImpl>());
  ServiceTracker<Foo> fooTracker(fc);
  for (auto _ : state) {
    auto start = high_resolution_clock::now();
    fooTracker.Open();
    auto end = high_resolution_clock::now();
    auto elapsed_seconds = duration_cast<duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
    fooTracker.Close();
  }
}

/// Benchmark how long it takes to open a service tracker using an interface name string
BENCHMARK_DEFINE_F(ServiceTrackerFixture, OpenServiceTrackerWithInterfaceName)(benchmark::State& state)
{
  using namespace std::chrono;
  using namespace benchmark::test;
  using namespace cppmicroservices;
  
  auto fc = framework->GetBundleContext();
  auto serviceReg = fc.RegisterService<Foo>(std::make_shared<FooImpl>());
  ServiceTracker<Foo> fooTracker(fc, std::string("benchmark::test::Foo"));

  for (auto _ : state) {
    auto start = high_resolution_clock::now();
    fooTracker.Open();
    auto end = high_resolution_clock::now();
    auto elapsed_seconds = duration_cast<duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
    fooTracker.Close();
  }
}

/// Benchmark service tracker scalability
/// How do service trackers perform as the number of service tracker objects grow?
BENCHMARK_DEFINE_F(ServiceTrackerFixture, ServiceTrackerScalability)(benchmark::State& state)
{
  using namespace benchmark::test;
  using namespace cppmicroservices;
  
  auto fc = framework->GetBundleContext();

  int64_t maxServiceTrackers{ state.range(0) };
  std::vector<std::unique_ptr<ServiceTracker<Foo>>> trackers;
  for (int64_t i = 0; i < maxServiceTrackers; ++i) {
    auto fooTracker = std::make_unique<ServiceTracker<Foo>>(fc);
    fooTracker->Open();
    trackers.emplace_back(std::move(fooTracker));
  }

  // how long does it take for N trackers to receive the register service event?
  for (auto _ : state) {
    fc.RegisterService<Foo>(std::make_shared<FooImpl>());
  }

  for (auto& tracker : trackers) {
    tracker->Close();
  }
}

// Register benchmark functions
BENCHMARK_REGISTER_F(ServiceTrackerFixture, OpenServiceTrackerWithSvcRef)->UseManualTime();
BENCHMARK_REGISTER_F(ServiceTrackerFixture, OpenServiceTrackerWithBundleContext)->UseManualTime();
BENCHMARK_REGISTER_F(ServiceTrackerFixture, OpenServiceTrackerWithInterfaceName)->UseManualTime();

// Run this benchmark for each Arg(...) call, passing in the parameter value to the benchmark.
BENCHMARK_REGISTER_F(ServiceTrackerFixture, ServiceTrackerScalability)->Arg(1)
                                                                      ->Arg(4000)
                                                                      ->Arg(10000);
