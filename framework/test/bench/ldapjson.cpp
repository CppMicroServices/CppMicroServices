#include <cppmicroservices/AnyMap.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/LDAPFilter.h>
#include <cppmicroservices/LDAPProp.h>

#include "TestUtils.h"
#include "benchmark/benchmark.h"
//#include "fooservice.h"

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

//extern "C"
//{
//#include <jq.h>
//#include <jv.h>
//}

#include <string>
#include <fstream>
#include <iostream>

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
  //LDAPFilter filt("(mode=cloud)");
  //std::string key {"bundle_configuration.0"};
  
//  LDAPFilter filt("(priority=required)");
//  std::string key {"scr.components.0.properties.mw.startup_plugins"};

  
  LDAPFilter filt("(e=found)");
  std::string key {"a.b.c.d"};
  
//  auto props_b = cppmicroservices::ref_any_cast<const cppmicroservices::AnyMap>(props.AtCompoundKey(key));
//  bool res = filt.Match(props_b);
//  if (res) {
//    std::cout << "LDAP MATCH FOUND!" << std::endl;
//  } else {
//    std::cout << "LDAP MATCH NOT FOUND!" << std::endl;
//  }
  
  for (auto _ : state) {
    auto props_a = cppmicroservices::ref_any_cast<const cppmicroservices::AnyMap>(props.AtCompoundKey(key));
    (void)filt.Match(props_a);
  };
}

static void JsonPathNested(benchmark::State& state)
{
  std::string path(US_FRAMEWORK_SOURCE_DIR);
  path += "/test/bundles/dummyService/resources/manifest.json";
  
  //std::string filter_str = "$.bundle_configuration[?(@.mode=='cloud')]";
 // std::string filter_str {"$.scr.components[?(@.properties.mw.startup_plugins.priority=='required')]"};
  std::string filter_str{"$.a.b.c[?(@.e=='found')].e"};
  //std::string filter_str{"$..[?(@.e=='found')]"};
  
  std::ifstream is(path);
  jsoncons::json doc = jsoncons::json::parse(is);
//  jsoncons::json result = jsoncons::jsonpath::json_query(doc, filter_str);
//  std::cout << pretty_print(result) << std::endl;
  
  for (auto _ : state) {
    (void)jsoncons::jsonpath::json_query(doc, filter_str);
  }
}

static void JMESPathNested(benchmark::State& state)
{
  std::string path(US_FRAMEWORK_SOURCE_DIR);
  path += "/test/bundles/dummyService/resources/manifest.json";
  
  //std::string filter_str = "bundle_configuration[?mode=='cloud']";
  //std::string filter_str = "scr.components[?properties.mw.startup_plugins.priority=='required']";
  std::string filter_str = "[a.b.c.d][?e=='found'].e";
  
  std::ifstream is(path);
  jsoncons::json doc = jsoncons::json::parse(is);
//  jsoncons::json result = jsoncons::jmespath::search(doc, filter_str);
//  std::cout << pretty_print(result) << std::endl;
  
  for (auto _ : state) {
    (void)jsoncons::jmespath::search(doc, filter_str);
  }
}









//static int process(jq_state *jq, jv value, int flags, int dumpopts) {
//    int ret = 14; // No valid results && -e -> exit(4)
//    jq_start(jq, value, flags);
//    jv result;
//    //printf("\n");
//  (void)dumpopts;
//    while (jv_is_valid(result = jq_next(jq)))
//    {
//        //print the result
//        (void)jv_get_kind(result);
//        //printf("\n");
//
//        /*
//        * this will dump the result to the console. Here instead of doing this we can check the type of the result object using  jv_get_kind
//        * and then extract the key-value strings using jv API's (see jv_string_value API in jv.h)
//        */
//        //jv_dump(result, dumpopts);
//
//    }
//
//    if (jq_halted(jq)) {
//        // jq program invoked `halt` or `halt_error`
//        jv exit_code = jq_get_exit_code(jq);
//        if (!jv_is_valid(exit_code))
//            ret = 0;
//        else if (jv_get_kind(exit_code) == JV_KIND_NUMBER)
//            ret = jv_number_value(exit_code);
//        else
//            ret = 5;
//        jv_free(exit_code);
//        jv error_message = jq_get_error_message(jq);
//        if (jv_get_kind(error_message) == JV_KIND_STRING) {
//            fprintf(stderr, "%s", jv_string_value(error_message));
//        }
//        else if (jv_get_kind(error_message) == JV_KIND_NULL) {
//            // Halt with no output
//        }
//        else if (jv_is_valid(error_message)) {
//            error_message = jv_dump_string(jv_copy(error_message), 0);
//            fprintf(stderr, "%s\n", jv_string_value(error_message));
//        } // else no message on stderr; use --debug-trace to see a message
//        fflush(stderr);
//        jv_free(error_message);
//    }
//    else if (jv_invalid_has_msg(jv_copy(result))) {
//        // Uncaught jq exception
//        jv msg = jv_invalid_get_msg(jv_copy(result));
//        jv input_pos = jq_util_input_get_position(jq);
//        if (jv_get_kind(msg) == JV_KIND_STRING) {
//            fprintf(stderr, "jq: error (at %s): %s\n",
//                jv_string_value(input_pos), jv_string_value(msg));
//        }
//        else {
//            msg = jv_dump_string(msg, 0);
//            fprintf(stderr, "jq: error (at %s) (not a string): %s\n",
//                jv_string_value(input_pos), jv_string_value(msg));
//        }
//        ret = 5;
//        jv_free(input_pos);
//        jv_free(msg);
//    }
//    jv_free(result);
//    return ret;
//}
//
////static void debug_cb(void *data, jv input) {
////    int dumpopts = *(int *)data;
////    jv_dumpf(JV_ARRAY(jv_string("DEBUG:"), input), stderr, dumpopts & ~(JV_PRINT_PRETTY));
////    fprintf(stderr, "\n");
////}











//static void JQNested(benchmark::State& state)
//{
//  int ret = 0;
//  jq_state *jq = NULL;
//
//  /*
//  * Argument filters for extracting the values from JSON file
//  */
//  //const char* filter3 = ".scr.components[].properties.mw.startup_plugins | select (.priority==\"required\")";
//  const char* filter3 = ".a.b.c.d | select (.e==\"found\")";
//  int compiled = 0;
//  int jq_flags = 0;
//
//  /*
//  * initialize a jq_state object
//  */
//  jq = jq_init();
//  if (jq == NULL)
//  {
//    printf("jq_init failed !");
//    return;
//  }
//  //else
//  //{
////      printf("jq_init successess !\n");
////      printf("\n The current filter is : %s \n", filter3);
//
//      /*
//      * create and initialize a jq_util_input_state object for parsing the input json datafile
//      */
//
//
//
//
//  //}
//
//  jq_util_input_state *input_state = jq_util_input_init(NULL, NULL);
//  jq_util_input_add_input(input_state, "/Users/kevinlee/Desktop/manifest.json");
//
//  int dumpopts = JV_PRINT_INDENT_FLAGS(2);
//
//  for (auto _ : state) {
//    /*
//    * compile the input arguments
//    */
//    compiled = jq_compile(jq, filter3);
//    if (!compiled)
//    {
//      printf("NOT COMPILED");
//      return;
//    }
//
//        // Let jq program read from inputs
//        int parser_flags = 0;
//
//        //set an input state parser
//        jq_util_input_set_parser(input_state, jv_parser_new(parser_flags),0);
//        jq_set_input_cb(jq, jq_util_input_next_input_cb, input_state);
//        dumpopts |= JV_PRINT_COLOR;
//    /*
//    * Iterate through the input state using the compiled arguments and extract the valid values form the input json file
//    */
//    jv value;
//    while (jq_util_input_errors(input_state) == 0 &&
//        (jv_is_valid((value = jq_util_input_next_input(input_state))) || jv_invalid_has_msg(jv_copy(value)))) {
//
//        if (jv_is_valid(value)) {
//            (void)jv_get_kind(value);
//            ret = process(jq, value, jq_flags, dumpopts);
//            continue;
//        }
//
//        // Parse error
//        jv msg = jv_invalid_get_msg(value);
//        fprintf(stderr, "ignoring parse error: %s\n", jv_string_value(msg));
//        jv_free(msg);
//    }
//  }
//}

BENCHMARK(LDAPNested);
BENCHMARK(JsonPathNested);
BENCHMARK(JMESPathNested);
//BENCHMARK(JQNested);
