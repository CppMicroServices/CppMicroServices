#include <cppmicroservices/LDAPProp.h>
#include "benchmark/benchmark.h"

static void ConstructFilterIncremental(benchmark::State& state)
{
  using namespace cppmicroservices;
  
  LDAPPropExpr expr;
  LDAPPropExpr secondaryExpr;
  for (auto _ : state) {
    expr = LDAPProp("mode") == "cloud";
    secondaryExpr = LDAPProp("minProgLevel") == "Dynamic";
    secondaryExpr |= LDAPProp("minProgLevel") == "Managed";
    expr &= secondaryExpr;
  };
}

static void ConstructFilterNotOperator(benchmark::State& state)
{
  using namespace cppmicroservices;
  
  LDAPPropExpr expr;
  for (auto _ : state) {
    expr  = !LDAPProp("IsDynamic");
    expr |=  LDAPProp("IsDynamic") != "false";
  };
}

// Register functions as benchmarrk
BENCHMARK(ConstructFilterIncremental);
BENCHMARK(ConstructFilterNotOperator);
