#include <cppmicroservices/AnyMap.h>
#include <cppmicroservices/LDAPFilter.h>
#include <cppmicroservices/LDAPProp.h>

#include "benchmark/benchmark.h"

using namespace cppmicroservices;

// --- Benchmark Group 1: Filter Parsing (measures item #1 - parse caching) ---

static void BM_ParseSimpleFilter(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(LDAPFilter("(name=value)"));
    }
}
BENCHMARK(BM_ParseSimpleFilter);

static void BM_ParseComplexFilter(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(LDAPFilter("(&(objectClass=com.example.MyService)(version>=2)(!(deprecated=true)))"));
    }
}
BENCHMARK(BM_ParseComplexFilter);

static void BM_ParseDeeplyNestedFilter(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(LDAPFilter(
            "(|(&(a=1)(b=2)(c=3))(&(d=4)(e=5)(f=6))(&(g=7)(h=8)(i=9)))"));
    }
}
BENCHMARK(BM_ParseDeeplyNestedFilter);

static void BM_ReusePreParsedFilter(benchmark::State& state)
{
    LDAPFilter filter("(&(objectClass=com.example.MyService)(version>=2)(!(deprecated=true)))");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["objectClass"] = std::string("com.example.MyService");
    props["version"] = 3;
    props["deprecated"] = false;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_ReusePreParsedFilter);

// --- Benchmark Group 2: Case-insensitive map lookup (measures item #2) ---

static void BM_CaseInsensitiveMap_Match(benchmark::State& state)
{
    LDAPFilter filter("(BundlePriority=high)");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["bundlepriority"] = std::string("high");
    props["other1"] = std::string("val1");
    props["other2"] = std::string("val2");
    props["other3"] = std::string("val3");
    props["other4"] = std::string("val4");

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_CaseInsensitiveMap_Match);

static void BM_UnorderedMap_CaseInsensitive_Match(benchmark::State& state)
{
    LDAPFilter filter("(BundlePriority=high)");
    AnyMap props(AnyMap::UNORDERED_MAP);
    props["bundlepriority"] = std::string("high");
    props["other1"] = std::string("val1");
    props["other2"] = std::string("val2");
    props["other3"] = std::string("val3");
    props["other4"] = std::string("val4");
    props["other5"] = std::string("val5");
    props["other6"] = std::string("val6");
    props["other7"] = std::string("val7");
    props["other8"] = std::string("val8");
    props["other9"] = std::string("val9");
    props["other10"] = std::string("val10");

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_UnorderedMap_CaseInsensitive_Match);

static void BM_OrderedMap_CaseInsensitive_Match(benchmark::State& state)
{
    LDAPFilter filter("(BundlePriority=high)");
    AnyMap props(AnyMap::ORDERED_MAP);
    props["bundlepriority"] = std::string("high");
    props["other1"] = std::string("val1");
    props["other2"] = std::string("val2");
    props["other3"] = std::string("val3");
    props["other4"] = std::string("val4");
    props["other5"] = std::string("val5");
    props["other6"] = std::string("val6");
    props["other7"] = std::string("val7");
    props["other8"] = std::string("val8");
    props["other9"] = std::string("val9");
    props["other10"] = std::string("val10");

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_OrderedMap_CaseInsensitive_Match);

static void BM_LargeMap_CaseInsensitive_Match(benchmark::State& state)
{
    LDAPFilter filter("(TargetKey=found)");
    AnyMap props(AnyMap::UNORDERED_MAP);
    for (int i = 0; i < 50; ++i)
    {
        props["property_key_" + std::to_string(i)] = std::string("value_" + std::to_string(i));
    }
    props["targetkey"] = std::string("found");

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_LargeMap_CaseInsensitive_Match);

// --- Benchmark Group 3: Wildcard pattern matching (measures item #3) ---

static void BM_WildcardSimple(benchmark::State& state)
{
    LDAPFilter filter("(name=hello*)");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["name"] = std::string("hello world this is a test string");

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_WildcardSimple);

static void BM_WildcardMiddle(benchmark::State& state)
{
    LDAPFilter filter("(name=hello*world)");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["name"] = std::string("hello beautiful wonderful amazing world");

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_WildcardMiddle);

static void BM_WildcardMultiple(benchmark::State& state)
{
    LDAPFilter filter("(name=*foo*bar*baz*)");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["name"] = std::string("prefix_foo_middle_bar_another_baz_suffix");

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_WildcardMultiple);

static void BM_WildcardMultiple_NoMatch(benchmark::State& state)
{
    LDAPFilter filter("(name=*foo*bar*baz*)");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["name"] = std::string("this string has foo and bar but not the third word");

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_WildcardMultiple_NoMatch);

static void BM_WildcardWorstCase(benchmark::State& state)
{
    LDAPFilter filter("(name=*a*a*a*a*b)");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["name"] = std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_WildcardWorstCase);

// --- Benchmark Group 4: std::function overhead (measures item #4) ---

static void BM_EvaluateMultipleLeaves(benchmark::State& state)
{
    LDAPFilter filter("(&(a=1)(b=2)(c=3)(d=4)(e=5))");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["a"] = std::string("1");
    props["b"] = std::string("2");
    props["c"] = std::string("3");
    props["d"] = std::string("4");
    props["e"] = std::string("5");

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_EvaluateMultipleLeaves);

static void BM_EvaluateOR_FirstMatch(benchmark::State& state)
{
    LDAPFilter filter("(|(x=1)(y=2)(z=3))");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["x"] = std::string("1");

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_EvaluateOR_FirstMatch);

static void BM_EvaluateOR_LastMatch(benchmark::State& state)
{
    LDAPFilter filter("(|(x=1)(y=2)(z=3))");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["z"] = std::string("3");

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_EvaluateOR_LastMatch);

// --- Benchmark Group 5: Realistic service lookup simulation ---

static void BM_RealisticServiceLookup(benchmark::State& state)
{
    LDAPFilter filter("(&(objectClass=com.example.ILogger)(level>=2)(!(disabled=true)))");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["objectClass"] = std::string("com.example.ILogger");
    props["level"] = 3;
    props["disabled"] = false;
    props["service.id"] = 42;
    props["service.ranking"] = 10;
    props["component.name"] = std::string("LoggerImpl");

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_RealisticServiceLookup);

static void BM_RealisticServiceLookup_NoMatch(benchmark::State& state)
{
    LDAPFilter filter("(&(objectClass=com.example.ILogger)(level>=2)(!(disabled=true)))");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["objectClass"] = std::string("com.example.ILogger");
    props["level"] = 1;
    props["disabled"] = false;
    props["service.id"] = 42;
    props["service.ranking"] = 10;
    props["component.name"] = std::string("LoggerImpl");

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_RealisticServiceLookup_NoMatch);

static void BM_ParseAndMatch_Repeated(benchmark::State& state)
{
    std::string filterStr = "(&(objectClass=com.example.ILogger)(level>=2))";
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["objectClass"] = std::string("com.example.ILogger");
    props["level"] = 3;

    for (auto _ : state)
    {
        LDAPFilter filter(filterStr);
        benchmark::DoNotOptimize(filter.Match(props));
    }
}
BENCHMARK(BM_ParseAndMatch_Repeated);
