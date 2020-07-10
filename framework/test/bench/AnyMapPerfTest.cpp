#include "benchmark/benchmark.h"

#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/AnyMap.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/FrameworkEvent.h>

#include <iostream>
#include <cassert>

#include "TestUtils.h"

using namespace cppmicroservices;

class AnyMapPerfTestFixture : public ::benchmark::Fixture
{
public:
  using benchmark::Fixture::SetUp;
  using benchmark::Fixture::TearDown;

  void SetUp(const ::benchmark::State&)
  {
    framework      = std::make_shared<Framework>(FrameworkFactory().NewFramework());
    framework->Start();
    auto context   = framework->GetBundleContext();
    testBundle     = testing::InstallLib(context, "DataOnlyTestBundle");
    testBundle.Start();
  }

  void TearDown(const ::benchmark::State&)
  {
    using namespace std::chrono;
    
    if (testBundle) {
      testBundle.Stop();
    }
    framework->Stop();
    framework->WaitForStop(milliseconds::zero());
  }

  ~AnyMapPerfTestFixture() = default;

  Bundle testBundle;
  std::shared_ptr<Framework> framework;
};

// util function to construct a compound key in dot notation.
// constructNestedKey(0, "foo", "bar")  --> ""
// constructNestedKey(1, "foo", "bar")  --> "bar"
// constructNestedKey(2, "foo", "bar")  --> "foo.bar"
// constructNestedKey(3, "foo", "bar")  --> "foo.foo.bar"
// constructNestedKey(4, "foo", "bar")  --> "foo.foo.foo.bar"
std::string constructNestedKey(unsigned int depth
                               , const std::string& mapname
                               , const std::string& lastkeyname)
{
  switch (depth) {
    case 0:
      return "";
      break;
    case 1:
      return lastkeyname;
      break;
    default:
      return mapname + "." + constructNestedKey(depth - 1, mapname, lastkeyname);
      break;
  }
}

BENCHMARK_DEFINE_F(AnyMapPerfTestFixture, HappyPath)(benchmark::State& state)
{
  const auto&  bundleProps = testBundle.GetHeaders();
  const Any&         testData    = bundleProps.at("Test_AtCompoundKey");
  assert(!testData.Empty());
  const AnyMap&      testAnyMap  = ref_any_cast<AnyMap>(testData);
  unsigned int depth       = static_cast<unsigned int>(state.range(0));
  std::string  key(constructNestedKey(depth, "relativelylongkeyname_map", "relativelylongkeyname_element"));

  for (auto _ : state) {
    try {
      (void)testAnyMap.AtCompoundKey(key);
    }
    catch (...) {
      state.SkipWithError("Exception thrown from AtCompoundKey");
      break;
    }
  }
}

BENCHMARK_DEFINE_F(AnyMapPerfTestFixture, ErrorPath)(benchmark::State& state)
{
  const auto&  bundleProps = testBundle.GetHeaders();
  const Any&         testData    = bundleProps.at("Test_AtCompoundKey");
  assert(!testData.Empty());
  const AnyMap&      testAnyMap  = ref_any_cast<AnyMap>(testData);
  unsigned int depth       = static_cast<unsigned int>(state.range(0));
  std::string  key(constructNestedKey(depth, "relativelylongkeyname_map", "relativelylongkeyname_unknown"));

  for (auto _ : state) {
    try {
      (void)testAnyMap.AtCompoundKey(key);
      state.SkipWithError("Exception not thrown from AtCompoundKey for error path");
      break;
    }
    catch (...) {
      // exception is expected
    }
  }
}

BENCHMARK_DEFINE_F(AnyMapPerfTestFixture, HappyPath_NoThrowOverload)(benchmark::State& state)
{
  const auto&  bundleProps = testBundle.GetHeaders();
  const Any&         testData    = bundleProps.at("Test_AtCompoundKey");
  assert(!testData.Empty());
  const AnyMap&      testAnyMap  = ref_any_cast<AnyMap>(testData);
  unsigned int depth       = static_cast<unsigned int>(state.range(0));
  std::string  key(constructNestedKey(depth, "relativelylongkeyname_map", "relativelylongkeyname_element"));

  Any a;
  for (auto _ : state) {
    try {
      auto value = testAnyMap.AtCompoundKey(key, a);
    }
    catch (...) {
      state.SkipWithError("Exception not expected from AtCompoundKey with default value");
      break;
    }
  }
}

BENCHMARK_DEFINE_F(AnyMapPerfTestFixture, ErrorPath_NoThrowOverload)(benchmark::State& state)
{
  const auto&  bundleProps = testBundle.GetHeaders();
  const Any&         testData    = bundleProps.at("Test_AtCompoundKey");
  assert(!testData.Empty());
  const AnyMap&      testAnyMap  = ref_any_cast<AnyMap>(testData);
  unsigned int depth       = static_cast<unsigned int>(state.range(0));
  std::string  key(constructNestedKey(depth
                                      , "relativelylongkeyname_map"
                                      , "relativelylongkeyname_unknown"));

  Any a;
  for (auto _ : state) {
    try {
      auto value = testAnyMap.AtCompoundKey(key, a);
    }
    catch (...) {
      state.SkipWithError("Exception not expected from AtCompoundKey with default value");
      break;
    }
  }
}



// Register functions as benchmarrk
BENCHMARK_REGISTER_F(AnyMapPerfTestFixture, HappyPath)->Arg(1)
                                                      ->Arg(3)
                                                      ->Arg(7)
                                                      ->Arg(11)
                                                      ->Arg(15)
                                                      ->Arg(18)
                                                      ->Arg(20);
BENCHMARK_REGISTER_F(AnyMapPerfTestFixture, ErrorPath)->Arg(1)
                                                      ->Arg(3)
                                                      ->Arg(7)
                                                      ->Arg(11)
                                                      ->Arg(15)
                                                      ->Arg(18)
                                                      ->Arg(20);
BENCHMARK_REGISTER_F(AnyMapPerfTestFixture, HappyPath_NoThrowOverload)->Arg(1)
                                                                      ->Arg(3)
                                                                      ->Arg(7)
                                                                      ->Arg(11)
                                                                      ->Arg(15)
                                                                      ->Arg(18)
                                                                      ->Arg(20);
BENCHMARK_REGISTER_F(AnyMapPerfTestFixture, ErrorPath_NoThrowOverload)->Arg(1)
                                                                      ->Arg(3)
                                                                      ->Arg(7)
                                                                      ->Arg(11)
                                                                      ->Arg(15)
                                                                      ->Arg(18)
                                                                      ->Arg(20);
