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
//#include <jsoncons_ext/jsonpath/jsonpath.hpp>

#include <fstream>
#include <iostream>
#include <string>

#include <chrono>
#include <thread>

//#include "../../src/bundle/BundleManifest.h"
#include "../util/BundleManifest.h"
using namespace cppmicroservices;
/*
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
*/
  /*
// case 0: bundle_start='greedy'
  LDAPFilter filt("(bundle_start=greedy)");

  for (auto _ : state) {
    res = filt.Match(props);
  };
 */

  /*
 // case 1: index of array not known
  LDAPFilter filt("(priority=required)");
  for (auto _ : state) {
    vec = ref_any_cast<std::vector<Any>>(props.AtCompoundKey("scr.components"));
    for (const auto & anAny : vec) {
      Any aaa = ref_any_cast<const AnyMap>(anAny).AtCompoundKey("properties.mw.startup_plugins", Any());
      if (!aaa.Empty()) {
        res = res || filt.Match(ref_any_cast<const AnyMap>(aaa));
      }
    }
  };
*/
/*
  //case 2: index of array known
  LDAPFilter filt("(priority=required)");
  std::string key{ "scr.components.0.properties.mw.startup_plugins" };

  for (auto _ : state) {
    Any aaa = props.AtCompoundKey(key, Any());
    if (!aaa.Empty()) {
      res = filt.Match(ref_any_cast<const AnyMap>(aaa));
    }
  };

  
  // case 3: a.b.c.d.e=='found'
  LDAPFilter filt("(e=found)");
  std::string key {"a.b.c.d"};

  for (auto _ : state) {
    Any aaa = props.AtCompoundKey(key, Any());
    if (!aaa.Empty()) {
      res = filt.Match(ref_any_cast<const AnyMap>(aaa));
    }
  };
   */

  //  if (res) { std::cout << "LDAP match found" << std::endl; }
  //  else { std::cout << "LDAP match NOT found " << std::endl; }

  /* (void)res;
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
  // std::string filter_str{"bundle_start=='greedy'"};

  // case 1: index of array not known
  // std::string filter_str = "scr.components[?properties.mw.startup_plugins.priority=='required'].properties.mw.startup_plugins.priority";

  // case 2: index of array known
  std::string filter_str =
    "scr.components[0].properties.mw.startup_plugins.priority=='required'";

  // case 3: a.b.c.d.e == 'e'
  // std::string filter_str{"a.b.c.d.e=='found'"};

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
//
//  // case 1: index of array not known
//  // std::string filter_str {"$.scr.components[?(@.properties.mw.startup_plugins.priority=='required')].properties.mw.startup_plugins.priority"};
//
//  // case 2: index of array known
//   std::string filter_str{"$.scr.components.0.properties.mw.startup_plugins.priority"};
//
//  // case 3: a.b.c.d.e == 'e'
//  // std::string filter_str{"$.a.b.c[?(@.e=='found')].e"};
//
//  for (auto _ : state) {
//    result = jsoncons::jsonpath::json_query(doc, filter_str);
//
//  }
//
////  std::cout << pretty_print(result) << std::endl;
//}

static void AnyMapToJsoncons(benchmark::State& state)    // used in proporties.cpp to convert anymap to json_props
{
  auto framework = FrameworkFactory().NewFramework();
  framework.Start();
  auto context = framework.GetBundleContext();
  auto bundle = testing::InstallLib(context, "dummyService");
  if (bundle.GetSymbolicName() != "dummyService") {
    state.SkipWithError("Error: Couldn't find installed bundle with symbolic "
                        "name 'dummyService'");
  }

  for (auto _ : state) {
    jsoncons::json j(jsoncons::json::parse(Any(bundle.GetHeaders()).ToJSON()));
  }
}

static void JsonconsToAnyMap(benchmark::State& state) {
  
  std::string path(US_FRAMEWORK_SOURCE_DIR);
  path += "/test/bundles/dummyService/resources/manifest.json";
  std::ifstream is(path);
  jsoncons::json doc = jsoncons::json::parse(is);
  BundleManifest bm;

  for (auto _ : state) {
    std::string str;
    doc.dump(str);
    std::istringstream sstr(str);
    bm.Parse(sstr);
  }
    
}

static void ParseManifestToString(benchmark::State& state)  // ask
{
  std::string path(US_FRAMEWORK_SOURCE_DIR);
  path += "/test/bundles/dummyService/resources/manifest.json";

  for (auto _ : state) {
    std::ifstream t(path);
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
  }
}

static void ParseManifestToAnyMap(benchmark::State& state) { //ask
  std::string path(US_FRAMEWORK_SOURCE_DIR);
  path += "/test/bundles/dummyService/resources/manifest.json";

  for (auto _ : state) {
    BundleManifest bm;
    std::ifstream is(path);
    bm.Parse(is);
  }
  
}

static void ParseManifestToJsoncons(benchmark::State& state)  // used in proporties.cpp
{
  std::string path(US_FRAMEWORK_SOURCE_DIR);
  path += "/test/bundles/dummyService/resources/manifest.json";
  for (auto _ : state) {
    std::ifstream is(path);
    jsoncons::json doc(jsoncons::json::parse(is));
  }
}

static void ParseManifestToJsonconsFromStringStream(benchmark::State& state)
{
  std::string path(US_FRAMEWORK_SOURCE_DIR);
  path += "/test/bundles/dummyService/resources/manifest.json";
  std::ifstream t(path);
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());

  for (auto _ : state) {
    std::istringstream is(str);
    jsoncons::json doc(jsoncons::json::parse(is));
  }
}

static void ParseManifestToAnyMapFromStringStream(benchmark::State& state) {
  std::string path(US_FRAMEWORK_SOURCE_DIR);
  path += "/test/bundles/dummyService/resources/manifest.json";
  std::ifstream t(path);
  std::string str((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());
  
  //BundleManifest bm;
  
  for (auto _ : state) {
    BundleManifest bm;
    std::istringstream is(str);
    bm.Parse(is);
  }
}


static void ParseManifestToBoth(benchmark::State& state) {
  std::string path(US_FRAMEWORK_SOURCE_DIR);
  path += "/test/bundles/dummyService/resources/manifest.json";
  BundleManifest bm;
  
  for (auto _ : state) {
    std::ifstream t(path);
    std::string str((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
    
    std::istringstream iss1(str);
    std::istringstream iss2(str);
    jsoncons::json doc(jsoncons::json::parse(iss1));
    bm.Parse(iss2);
  }
  
}

static void ParseManifestToJsonconsThenToAnyMap(benchmark::State& state) {
  std::string path(US_FRAMEWORK_SOURCE_DIR);
  path += "/test/bundles/dummyService/resources/manifest.json";
  BundleManifest bm;

  for (auto _ : state) {
    std::ifstream is(path);
    jsoncons::json doc(jsoncons::json::parse(is));
    std::string str;
    doc.dump(str);
    std::istringstream sstr(str);
    bm.Parse(sstr);
  }
}

static void ParseManifestToAnyMapThenToJsoncons(benchmark::State& state) {
  std::string path(US_FRAMEWORK_SOURCE_DIR);
  path += "/test/bundles/dummyService/resources/manifest.json";
  BundleManifest bm;

  for (auto _ : state) {
    std::ifstream is(path);
    bm.Parse(is);
    jsoncons::json j(jsoncons::json::parse(Any(bm.GetHeaders()).ToJSON()));
  }
}

static void LDAPNestedServiceReference(benchmark::State& state)
{
  using namespace benchmark::test;

  // register service with custom properties
  ScopedFramework scopedFramework;
  ServiceProperties props;

  LDAPFilter filt("(priority=required)");

  auto services =
    scopedFramework.framework.GetBundleContext().GetServiceReference(
      "cppmicroservices::TestBundleLQService");

   for (const auto& service : services) {
    try {
      const AnyMap serviceProps = [&service] {
        const auto props = ref_any_cast<const AnyMap>(service.GetProperty("service"));
        return ref_any_cast<const AnyMap>(props.AtCompoundKey("nestedProperty"));
      }();
	
                 for (auto _ : state) {
                (void)filt.Match(serviceProps);
                }
    } catch (...) {
      }
   }
}

BENCHMARK(LDAPNested);
//BENCHMARK(JsonPathNested);
BENCHMARK(JMESPathNested);
BENCHMARK(AnyMapToJsoncons);
BENCHMARK(JsonconsToAnyMap);

BENCHMARK(ParseManifestToString);
BENCHMARK(ParseManifestToAnyMap);
BENCHMARK(ParseManifestToAnyMapFromStringStream);
BENCHMARK(ParseManifestToJsoncons);
BENCHMARK(ParseManifestToJsonconsFromStringStream);

BENCHMARK(ParseManifestToBoth);
BENCHMARK(ParseManifestToJsonconsThenToAnyMap);
BENCHMARK(ParseManifestToAnyMapThenToJsoncons);
*/
