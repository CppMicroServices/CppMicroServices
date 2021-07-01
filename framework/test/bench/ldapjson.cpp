#include <cppmicroservices/AnyMap.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/LDAPFilter.h>
#include <cppmicroservices/LDAPProp.h>

#include "TestUtils.h"
#include "benchmark/benchmark.h"

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

#include <string>
#include <fstream>
#include <iostream>

#include <chrono>
#include <thread>

using namespace cppmicroservices;

static void LDAPNested(benchmark::State& state)
{
  auto framework = FrameworkFactory().NewFramework();
  framework.Start();
  auto context = framework.GetBundleContext();
  auto bundle = testing::InstallLib(context, "dummyService");
  if (bundle.GetSymbolicName() != "dummyService") {
    state.SkipWithError("Error: Couldn't find installed bundle with symbolic "
                        "name 'dummyService'");
  }
  
  auto props = bundle.GetHeaders();
  
  bool res = false;
  std::vector<Any> vec;
  
// case 0: bundle_start='greedy'
  LDAPFilter filt("(bundle_start=greedy)");

  for (auto _ : state) {
    res = filt.Match(props);
  };

// case 1: index of array not known
//  LDAPFilter filt("(priority=required)");
//  for (auto _ : state) {
//    vec = ref_any_cast<std::vector<Any>>(props.AtCompoundKey("scr.components"));
//    for (const auto & anAny : vec) {
//      res = res || filt.Match(ref_any_cast<const AnyMap>(ref_any_cast<const AnyMap>(anAny).AtCompoundKey("properties.mw.startup_plugins")));
//    }
//  };

// case 2: index of array known
//  LDAPFilter filt("(priority=required)");
//  std::string key {"scr.components.0.properties.mw.startup_plugins"};
//
//  for (auto _ : state) {
//    res = filt.Match(cppmicroservices::ref_any_cast<const cppmicroservices::AnyMap>(props.AtCompoundKey(key)));
//  };
  
// case 3: a.b.c.d.e=='found'
//  LDAPFilter filt("(e=found)");
//  std::string key {"a.b.c.d"};
//
//  for (auto _ : state) {
//    res = filt.Match(cppmicroservices::ref_any_cast<const cppmicroservices::AnyMap>(props.AtCompoundKey(key)));
//  };
  
//  if (res) { std::cout << "LDAP match found" << std::endl; }
//  else { std::cout << "LDAP match NOT found " << std::endl; }
}


static void JMESPathNested(benchmark::State& state)
{
  std::string path(US_FRAMEWORK_SOURCE_DIR);
  path += "/test/bundles/dummyService/resources/manifest.json";

  //std::string filter_str = "bundle_configuration[?mode=='cloud']";
  //std::string filter_str = "[a.b.c.d][?e=='found'].e";
  //std::string filter_str = "[@][?bundle_start=='greedy'].bundle_start";
  //std::string filter_str = "a.b.c.d.e=='found'";
    
  std::ifstream is(path);
  jsoncons::json doc = jsoncons::json::parse(is);
  jsoncons::json result;
  
  // case 0: bundle_start = greedy
  std::string filter_str{"bundle_start=='greedy'"};

  // case 1: index of array not known
//  std::string filter_str = "scr.components[?properties.mw.startup_plugins.priority=='required'].properties.mw.startup_plugins.priority";

  // case 2: index of array known
//  std::string filter_str = "scr.components[0].properties.mw.startup_plugins.priority=='required'";
  
  // case 3: a.b.c.d.e == 'e'
//  std::string filter_str{"a.b.c.d.e=='found'"};
  
  for (auto _ : state) {
    result = jsoncons::jmespath::search(doc, filter_str);
  }
  
//  std::cout << pretty_print(result) << std::endl;
}


//static void JsonPathNested(benchmark::State& state)
//{
//  std::string path(US_FRAMEWORK_SOURCE_DIR);
//  path += "/test/bundles/dummyService/resources/manifest.json";
//
//  //std::string filter_str = "$.bundle_configuration[?(@.mode=='cloud')]";
//  //std::string filter_str{"$.a.b.c[?(@.e=='found')].e"};
//  //std::string filter_str{"$..[?(@.e=='found')]"};
//  //std::string filter_str{"$..[?(@.bundle_start=='greedy')]"};
//
//  std::ifstream is(path);
//  jsoncons::json doc = jsoncons::json::parse(is);
//  jsoncons::json result;
//
//  // case 0: bundle_start = greedy --> NOT POSSIBLE
//  for (auto _ : state) {
//    std::this_thread::sleep_for(std::chrono::nanoseconds(10000));
//  }
//
//// case 1: index of array not known
////  std::string filter_str {"$.scr.components[?(@.properties.mw.startup_plugins.priority=='required')].properties.mw.startup_plugins.priority"};
//
//// case 2: index of array known
////  std::string filter_str{"$.scr.components.0.properties.mw.startup_plugins.priority"};
//
//// case 3: a.b.c.d.e == 'e'
////  std::string filter_str{"$.a.b.c[?(@.e=='found')].e"};
////
////  for (auto _ : state) {
////    result = jsoncons::jsonpath::json_query(doc, filter_str);
////  }
//
////  std::cout << pretty_print(result) << std::endl;
//}

BENCHMARK(LDAPNested);
//BENCHMARK(JsonPathNested);
BENCHMARK(JMESPathNested);
