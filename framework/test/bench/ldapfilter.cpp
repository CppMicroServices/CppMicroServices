#include <cppmicroservices/AnyMap.h>
#include <cppmicroservices/LDAPFilter.h>
#include <cppmicroservices/LDAPProp.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>

#include "benchmark/benchmark.h"
#include "fooservice.h"
#include "TestUtils.h"

using namespace cppmicroservices;

static void ConstructFilterFromString(benchmark::State& state)
{
  for (auto _ : state) {
    LDAPFilter filter("(plugins_priority=required)");
  };
}

static void ConstructNonTrivialFilterFromString(benchmark::State& state)
{
  for (auto _ : state) {
    LDAPFilter filter("( |(cn=Babs *)(sn=1) )");
  };
}

LDAPFilter GetSimpleLDAPFilter()
{
  return LDAPFilter ("(bundle_priority=high)");
}

LDAPFilter GetComplexLDAPFilter()
{
  LDAPPropExpr expr;
  expr = LDAPProp("bundle_priority") != "true";
  expr &= LDAPProp("bundle_start");
  return LDAPFilter(expr);
}


template <class Filter>
static void MatchFilterWithAnyMap(benchmark::State& state, Filter filter)
{
  AnyMap props(AnyMap::UNORDERED_MAP);
  props["bundle_priority"] = std::string("high");
  props["bundle_start"] = std::string("greedy");
  props["Status"] = false;

  for (auto _ : state) {
    (void)filter.Match(props);
  }
}

// A simple RAII class that wraps the framework and shuts it down
// when the instance goes out of scope.
struct ScopedFramework
{
  ScopedFramework()
    : framework(FrameworkFactory().NewFramework())
  {
    framework.Start();
  }

  ~ScopedFramework()
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  Framework framework;
};

template <class Filter>
static void MatchFilterWithBundle(benchmark::State& state, Filter filter)
{
  auto framework = FrameworkFactory().NewFramework();
  framework.Start();
  auto context = framework.GetBundleContext();
  auto bundle  = testing::InstallLib(context, "dummyService");
  if (bundle.GetSymbolicName() != "dummyService") {
    state.SkipWithError("Error: Couldn't find installed bundle with symbolic name 'dummyService'");
  }

  for (auto _ : state) {
    (void)filter.Match(bundle);
  }
}

template <class Filter>
static void MatchFilterWithServiceReference(benchmark::State& state, Filter filter)
{
  using namespace benchmark::test;
  
  // register service with custom properties
  ScopedFramework scopedFramework;
  ServiceProperties props;
  auto framework           = scopedFramework.framework;
  props["bundle_priority"] = std::string("high");
  props["bundle_start"]    = std::string("greedy");
  props["Status"]          = false;
  auto s1                  = std::make_shared<FooImpl>();
  (void)framework.GetBundleContext().RegisterService<Foo>(s1, props);

  auto sr = framework.GetBundleContext().GetServiceReference<Foo>();
  // match service reference
  if (!sr) {
    state.SkipWithError("Error: Couldn't find service reference for interface 'Foo' in the framework.");
  }

  for (auto _ : state) {
    (void)filter.Match(sr);
  }
}

// Register functions as benchmark
BENCHMARK(ConstructFilterFromString);
BENCHMARK(ConstructNonTrivialFilterFromString);
BENCHMARK_CAPTURE(MatchFilterWithAnyMap, Simple, GetSimpleLDAPFilter());
BENCHMARK_CAPTURE(MatchFilterWithAnyMap, Complex, GetComplexLDAPFilter());
BENCHMARK_CAPTURE(MatchFilterWithBundle, Simple, GetSimpleLDAPFilter());
BENCHMARK_CAPTURE(MatchFilterWithBundle, Complex, GetComplexLDAPFilter());
BENCHMARK_CAPTURE(MatchFilterWithServiceReference, Simple, GetSimpleLDAPFilter());
BENCHMARK_CAPTURE(MatchFilterWithServiceReference, Complex, GetComplexLDAPFilter());
