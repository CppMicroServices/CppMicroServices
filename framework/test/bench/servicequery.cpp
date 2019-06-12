#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/ServiceReference.h>

#include <chrono>

#include "benchmark/benchmark.h"

#include "fooservice.h"

class ServiceFixture : public ::benchmark::Fixture
{
public:
  using benchmark::Fixture::SetUp;
  using benchmark::Fixture::TearDown;

  void SetUp(const ::benchmark::State&)
  {
    using namespace cppmicroservices;
    using namespace benchmark::test;
    
    framework = std::make_shared<Framework>(FrameworkFactory().NewFramework());
    framework->Start();
    (void)framework->GetBundleContext().RegisterService<Foo>(std::make_shared<FooImpl>());
  }
    
  void TearDown(const ::benchmark::State&)
  {
    using namespace std::chrono;
    
    framework->Stop();
    framework->WaitForStop(milliseconds::zero());
  }

  ~ServiceFixture() = default;

  std::shared_ptr<cppmicroservices::Framework> framework;
};

BENCHMARK_DEFINE_F(ServiceFixture, GetServiceReferenceByInterface)(benchmark::State& state)
{
  for (auto _ : state) {
    (void)framework->GetBundleContext().GetServiceReference<benchmark::test::Foo>();
  }
}

BENCHMARK_DEFINE_F(ServiceFixture, GetServiceReferenceByClassName)(benchmark::State& state)
{
  for (auto _ : state) {
    (void)framework->GetBundleContext().GetServiceReference("benchmark::test::Foo");
  }
}

BENCHMARK_DEFINE_F(ServiceFixture, GetAllServiceReferencesByInterface)(benchmark::State& state)
{
  for (auto _ : state) {
    (void)framework->GetBundleContext().GetServiceReferences<benchmark::test::Foo>();
  }
}

BENCHMARK_DEFINE_F(ServiceFixture, GetAllServiceReferencesByClassName)(benchmark::State& state)
{
  for (auto _ : state) {
    (void)framework->GetBundleContext().GetServiceReferences("benchmark::test::Foo");
  }
}

BENCHMARK_DEFINE_F(ServiceFixture, GetAllServiceReferencesByClassNameAndLDAPFilter)(benchmark::State& state)
{
  for (auto _ : state) {
    (void)framework->GetBundleContext().GetServiceReferences("benchmark::test::Foo", "(objectclass=Foo)");
  }
}

BENCHMARK_DEFINE_F(ServiceFixture, GetAllServiceReferencesByInterfaceAndLDAPFilter)(benchmark::State& state)
{
  for (auto _ : state) {
    (void)framework->GetBundleContext().GetServiceReferences<benchmark::test::Foo>("(objectclass=Foo)");
  }
}

// Register benchmark functions
BENCHMARK_REGISTER_F(ServiceFixture, GetServiceReferenceByInterface);
BENCHMARK_REGISTER_F(ServiceFixture, GetServiceReferenceByClassName);
BENCHMARK_REGISTER_F(ServiceFixture, GetAllServiceReferencesByInterface);
BENCHMARK_REGISTER_F(ServiceFixture, GetAllServiceReferencesByClassName);
BENCHMARK_REGISTER_F(ServiceFixture, GetAllServiceReferencesByClassNameAndLDAPFilter);
BENCHMARK_REGISTER_F(ServiceFixture, GetAllServiceReferencesByInterfaceAndLDAPFilter);
