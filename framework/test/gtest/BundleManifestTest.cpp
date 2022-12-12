/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include "TestUtils.h"
#include "TestingConfig.h"
#include "cppmicroservices/Any.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/util/FileSystem.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../../src/bundle/BundleManifest.h"
#include "cppmicroservices/BundleResourceStream.h"

#include <iostream>

US_MSVC_PUSH_DISABLE_WARNING(4996)

using namespace cppmicroservices;

namespace
{

#ifdef US_BUILD_SHARED_LIBS
    std::string
    libName(std::string const& libBase)
    {
        return (US_LIB_PREFIX + libBase + US_LIB_POSTFIX + US_LIB_EXT);
    }

    std::string
    fullLibPath(std::string const& libBase)
    {
        namespace cu = cppmicroservices::util;
        namespace ct = cppmicroservices::testing;

        return (ct::LIB_PATH + cu::DIR_SEP + libName(libBase));
    }
#endif

    /**
     * recursively compare the content of headers with deprecated. "headers" is an AnyMap, which
     * stores any hierarchical values in AnyMaps also. The purpose of this function is to
     * store the data in a std::map hierarchy instead.
     */
    bool
    compare_deprecated_properties(AnyMap const& headers, std::map<std::string, Any> const& deprecated)
    {
        try
        {
            for (auto const& h : headers)
            {
                if (typeid(AnyMap) == h.second.Type())
                {
                    // If the headers contain a submap, we need to recurse to compare the
                    // values in the submaps since the deprecated properties are stored in a
                    // std::map.
                    auto subHeaders = any_cast<AnyMap>(h.second);
                    auto subDeprecated = any_cast<std::map<std::string, Any>>(deprecated.at(h.first));
                    return compare_deprecated_properties(subHeaders, subDeprecated);
                }
                else
                {
                    auto const& deprecatedValue = deprecated.at(h.first);

                    // There's no way to compare the values contained in the "Any"s. So,
                    // first make sure the type ids match
                    if (h.second.Type() != deprecatedValue.Type())
                        return false;

                    // And if the types match, make sure that the values
                    // match. Unfortunately the only way to get the value out is via an
                    // any_cast which requires the actual C++ type be known, and since we
                    // don't, we have to compare the string representations.
                    if (h.second.ToString() != deprecatedValue.ToString())
                        return false;
                }
            }
        }
        catch (...)
        {
            // The ".at" method on maps can throw, and can the any_cast<> operation. These
            // will only throw if there's some sort of mismatch between the content of
            // "headers" and "deprecated".
            return false;
        }
        return true;
    }

} // namespace

class BundleManifestTest : public ::testing::Test
{
  protected:
    BundleManifestTest() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {}
    virtual ~BundleManifestTest() = default;

    virtual void
    SetUp()
    {
        framework.Start();
    }

    virtual void
    TearDown()
    {
        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }

    cppmicroservices::Framework framework;
};

TEST_F(BundleManifestTest, UnicodeProperty)
{
    // 1. building static libraries (test bundle is included in the executable)
    // 2. using MINGW evironment (MinGW linker fails to link DLL with unicode path)
    // 3. using a compiler with no support for C++11 unicode string literals
#if !defined(US_BUILD_SHARED_LIBS) || defined(__MINGW32__) || !defined(US_CXX_UNICODE_LITERALS)
    SUCCEED() << "Skipping test point for unicode path";
#else
    std::string path_utf8 = testing::LIB_PATH + cppmicroservices::util::DIR_SEP + u8"くいりのまちとこしくそ"
                            + cppmicroservices::util::DIR_SEP + US_LIB_PREFIX + "TestBundleU" + US_LIB_POSTFIX
                            + US_LIB_EXT;

    auto bundles = bc.InstallBundles(path_utf8);
    ASSERT_EQ(bundles.size(), 1) << "Failed to install bundle using a unicode path";
    auto bundle = bundles.at(0);
    std::string expectedValue = u8"电脑 くいりのまちとこしくそ";
    std::string actualValue = bundle.GetHeaders().at("unicode.sample").ToString();
    ASSERT_STREQ(expectedValue, actualValue) << "Unicode data from manifest.json doesn't match expected value.";
#endif
}

TEST_F(BundleManifestTest, InstallBundleWithDeepManifest)
{
    auto bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleWithDeepManifest");
    auto const& headers = bundle.GetHeaders();
    ASSERT_THAT(headers.at(Constants::BUNDLE_SYMBOLICNAME).ToString(), ::testing::StrEq("TestBundleWithDeepManifest"))
        << "Bundle symblic name doesn't match.";

    // The same key/value must continue to exist in the deprecated properties map.
    auto deprecatedProperties = bundle.GetProperties();
    ASSERT_TRUE(compare_deprecated_properties(headers, deprecatedProperties)) << "Deprecated properties mismatch";
}

TEST_F(BundleManifestTest, ParseManifest)
{
    auto bundleM = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleM");

    ASSERT_TRUE(bundleM) << "Failed to install TestBundleM";

    auto const& headers = bundleM.GetHeaders();

    EXPECT_THAT(headers.at(Constants::BUNDLE_SYMBOLICNAME).ToString(), ::testing::StrEq("TestBundleM"));
    EXPECT_THAT(headers.at(Constants::BUNDLE_DESCRIPTION).ToString(), ::testing::StrEq("My Bundle description"));
    EXPECT_THAT(headers.at(Constants::BUNDLE_VERSION).ToString(), ::testing::StrEq("1.0.0"));

    // We should also check to make sure that the deprecated properties have been set up
    // correctly.
    auto deprecatedProperties = bundleM.GetProperties();
    ASSERT_TRUE(compare_deprecated_properties(headers, deprecatedProperties)) << "Deprecated properties mismatch";

    EXPECT_THAT(bundleM.GetSymbolicName(), ::testing::StrEq("TestBundleM"));
    EXPECT_EQ(bundleM.GetVersion(), BundleVersion(1, 0, 0));

    Any integer = headers.at("number");
    ASSERT_EQ(integer.Type(), typeid(int));

    Any doubleKeyValue = headers.at("double");
    ASSERT_EQ(doubleKeyValue.Type(), typeid(double));

    Any anyVector = headers.at("vector");
    ASSERT_EQ(anyVector.Type(), typeid(std::vector<Any>));
    std::vector<Any>& vec = ref_any_cast<std::vector<Any>>(anyVector);
    ASSERT_EQ(vec.size(), 3ul);
    ASSERT_EQ(vec[0].Type(), typeid(std::string));
    ASSERT_THAT(vec[0].ToString(), ::testing::StrEq("first"));
    ASSERT_EQ(vec[1].Type(), typeid(int));
    ASSERT_EQ(any_cast<int>(vec[1]), 2);

    Any anyMap = headers.at("map");
    ASSERT_EQ(anyMap.Type(), typeid(AnyMap));
    AnyMap& m = ref_any_cast<AnyMap>(anyMap);
    ASSERT_EQ(m.size(), 3ul);
    ASSERT_EQ(m["string"].Type(), typeid(std::string));
    ASSERT_THAT(m["string"].ToString(), ::testing::StrEq("hi"));
    ASSERT_EQ(m["number"].Type(), typeid(int));
    ASSERT_EQ(any_cast<int>(m["number"]), 4);
    ASSERT_EQ(m["list"].Type(), typeid(std::vector<Any>));
    ASSERT_EQ(any_cast<std::vector<Any>>(m["list"]).size(), 2ul);
}

namespace cppmicroservices
{

    struct TestBundleAService
    {
        virtual ~TestBundleAService() {}
    };

} // namespace cppmicroservices

#ifdef US_BUILD_SHARED_LIBS
TEST_F(BundleManifestTest, DirectManifestInstall)
{
    auto ctx = framework.GetBundleContext();

    cppmicroservices::AnyMap manifests(cppmicroservices::any_map::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    cppmicroservices::AnyMap::unordered_any_cimap testBundleAManifest = {
        {"bundle.symbolic_name", std::string("TestBundleA")},
        {    "bundle.activator",                       true}
    };
    manifests["TestBundleA"] = cppmicroservices::AnyMap(testBundleAManifest);

    auto const libPath = fullLibPath("TestBundleA");
    auto const& bundles = ctx.InstallBundles(libPath, manifests);

    ASSERT_EQ(1, bundles.size());
    for (auto b : bundles)
    {
        if (b.GetSymbolicName() != "TestBundleA")
            continue;
        auto headers = b.GetHeaders();
        auto manifest = cppmicroservices::any_cast<cppmicroservices::AnyMap>(manifests["TestBundleA"]);

        // check to make sure that all the headers in the manifest are there
        for (auto m : manifest)
        {
            ASSERT_EQ(m.second.ToString(), headers[m.first].ToString());
        }
        break;
    }
}

TEST_F(BundleManifestTest, DirectManifestInstallNoSymbolicName)
{
    auto ctx = framework.GetBundleContext();

    cppmicroservices::AnyMap manifests(cppmicroservices::any_map::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    cppmicroservices::AnyMap::unordered_any_cimap testBundleAManifest = {
  // We need to have at least one entry in the manifest to check and make sure it has what's
  // required. This is because if the manifest is empty, it is assumed that we are NOT injecting a
  // manifest and we go through the standard install by reading the manifest from the bundle
  // itself.
        {"foo", std::string("bar")}
    };
    manifests["TestBundleA"] = cppmicroservices::AnyMap(testBundleAManifest);

    auto const libPath = fullLibPath("TestBundleA");

    EXPECT_THROW({ ctx.InstallBundles(libPath, manifests); }, std::runtime_error);
}

TEST_F(BundleManifestTest, DirectManifestInstallBadLocation)
{
    auto ctx = framework.GetBundleContext();

    cppmicroservices::AnyMap manifests(cppmicroservices::any_map::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    cppmicroservices::AnyMap::unordered_any_cimap testBundleAManifest = {
        {"bundle.symbolic_name", std::string("TestBundleA")},
        {    "bundle.activator",                       true}
    };
    manifests["TestBundleA"] = cppmicroservices::AnyMap(testBundleAManifest);

    auto const libPath = fullLibPath("TestBundleA");

    EXPECT_THROW({ ctx.InstallBundles("/non/existent/path/to/bundle.dylib", manifests); }, std::runtime_error);
}

TEST_F(BundleManifestTest, DirectManifestInstallMulti)
{
    // Support the static linking case in which we have multiple bundles in one location that need to
    // be installed, so the "manifests" passed in contains a vector of bundle manifests, one for each
    // bundle at the location.
    auto ctx = framework.GetBundleContext();

    cppmicroservices::AnyMap manifests(cppmicroservices::any_map::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    cppmicroservices::AnyMap::unordered_any_cimap testBundleAManifest = {
        {                   "A",                        1.5},
        {                   "B",        std::string("Test")},
        {"bundle.symbolic_name", std::string("TestBundleA")},
        {    "bundle.activator",                       true}
    };
    manifests["TestBundleA"] = cppmicroservices::AnyMap(testBundleAManifest);
    cppmicroservices::AnyMap::unordered_any_cimap testBundleBManifest = {
        {"bundle.symbolic_name", std::string("TestBundleB")},
        {    "bundle.activator",                       true}
    };
    manifests["TestBundleB"] = cppmicroservices::AnyMap(testBundleBManifest);

    auto const libPath = fullLibPath("TestBundleA");
    auto const& bundles = ctx.InstallBundles(libPath, manifests);
    ASSERT_EQ(2, bundles.size());
    for (auto const& b : bundles)
    {
        auto headers = b.GetHeaders();
        auto manifest = cppmicroservices::any_cast<cppmicroservices::AnyMap>(manifests[b.GetSymbolicName()]);
        for (auto m : manifest)
        {
            // check to make sure that all the headers in the manifest are there
            ASSERT_EQ(m.second.ToString(), headers[m.first].ToString());
        }
    }
}

TEST_F(BundleManifestTest, DirectManifestInstallAndStart)
{
    auto ctx = framework.GetBundleContext();

    cppmicroservices::AnyMap manifests(cppmicroservices::any_map::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    cppmicroservices::AnyMap::unordered_any_cimap testBundleAManifest = {
        {                   "A",                        1.5},
        {                   "B",        std::string("Test")},
        {"bundle.symbolic_name", std::string("TestBundleA")},
        {    "bundle.activator",                       true}
    };
    manifests["TestBundleA"] = cppmicroservices::AnyMap(testBundleAManifest);

    auto location = fullLibPath("TestBundleA");
    auto const& bundles = ctx.InstallBundles(location, manifests);
    ASSERT_EQ(1, bundles.size());
    auto b = bundles[0];
    b.Start();
    auto headers = b.GetHeaders();
    auto manifest = cppmicroservices::any_cast<cppmicroservices::AnyMap>(manifests["TestBundleA"]);
    for (auto m : manifest)
    // check to make sure that all the headers in the manifest are there
    {
        ASSERT_EQ(m.second.ToString(), headers[m.first].ToString());
    }
    auto ref = ctx.GetServiceReference<cppmicroservices::TestBundleAService>();
    auto svc = ctx.GetService(ref);
    ASSERT_TRUE(!!svc);
}

TEST_F(BundleManifestTest, DirectManifestInstallAndStartMulti)
{
    namespace cppms = cppmicroservices;
    namespace sc = std::chrono;
    using ucimap = cppms::AnyMap::unordered_any_cimap;

    auto ctx = framework.GetBundleContext();

    cppms::AnyMap manifests(cppms::any_map::UNORDERED_MAP_CASEINSENSITIVE_KEYS);

    ucimap aManifest = {
        {"bundle.symbolic_name", std::string("TestBundleA")},
        {    "bundle.activator",                       true}
    };
    ucimap aLocationMap = {
        {"TestBundleA", cppms::AnyMap(aManifest)}
    };
    manifests[libName("TestBundleA")] = cppms::AnyMap(aLocationMap);

    ucimap a2Manifest = {
        {"bundle.symbolic_name", std::string("TestBundleA2")},
        {    "bundle.activator",                        true}
    };
    ucimap a2LocationMap = {
        {"TestBundleA2", cppms::AnyMap(a2Manifest)}
    };
    manifests[libName("TestBundleA2")] = cppms::AnyMap(a2LocationMap);

    // Now use the new API to install bundles with the path and a manifest stored in an AnyMap.
    auto bundleRoot = cppms::testing::LIB_PATH + util::DIR_SEP;
    for (auto const& m : manifests)
    {
        auto const& libPath = m.first;
        auto const& fullLibPath = bundleRoot + libPath;
        auto const& bundles = ctx.InstallBundles(fullLibPath, cppms::any_cast<cppms::AnyMap>((m.second)));
        ASSERT_EQ(1, bundles.size());
        for (auto b : bundles)
        {
            ASSERT_NO_THROW(b.Start());
        }
    }
}

TEST_F(BundleManifestTest, IgnoreSecondManifestInstall)
{
    namespace cppms = cppmicroservices;
    using cppms::any_cast;
    using cppms::any_map;
    using cppms::AnyMap;
    using cimap = AnyMap::unordered_any_cimap;

    auto ctx = framework.GetBundleContext();

    AnyMap manifests(any_map::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    cimap firstTimeManifest = {
        {                "test",                       true},
        {"bundle.symbolic_name", std::string("TestBundleA")},
        {    "bundle.activator",                       true}
    };
    cimap secondTimeManifest = {
        {                "test",                      false},
        {"bundle.symbolic_name", std::string("TestBundleA")},
        {    "bundle.activator",                       true}
    };
    manifests["TestBundleA"] = AnyMap(firstTimeManifest);
    auto const libPath = fullLibPath("TestBundleA");
    auto const& firstBundles = ctx.InstallBundles(libPath, manifests);
    auto const& firstHeaders = firstBundles[0].GetHeaders();
    ASSERT_TRUE(any_cast<bool>(firstHeaders.at("test")));

    // on second installation the manifest should be ignored, so our test value should remain true.
    manifests["TestBundleA"] = AnyMap(secondTimeManifest);
    auto const& secondBundles = ctx.InstallBundles(libPath, manifests);
    auto const& secondHeaders = secondBundles[0].GetHeaders();
    // should still be true after install.
    ASSERT_TRUE(any_cast<bool>(secondHeaders.at("test")));
}

TEST_F(BundleManifestTest, DirectManifestInstallAndStartMultiStatic)
{
    namespace cppms = cppmicroservices;
    namespace sc = std::chrono;
    using ucimap = cppms::AnyMap::unordered_any_cimap;

    auto ctx = framework.GetBundleContext();

    cppms::AnyMap manifests(cppms::any_map::UNORDERED_MAP_CASEINSENSITIVE_KEYS);

    ucimap bManifest = {
        {"bundle.symbolic_name", std::string("TestBundleB")},
        {    "bundle.activator",                       true}
    };
    ucimap b2Manifest = {
        {"bundle.symbolic_name", std::string("TestBundleImportedByB")},
        {    "bundle.activator",                                 true}
    };
    ucimap bLocationMap = {
        {          "TestBundleB",  cppms::AnyMap(bManifest)},
        {"TestBundleImportedByB", cppms::AnyMap(b2Manifest)}
    };
    // Now use the new API to install bundles with the path and a manifest stored in an AnyMap.
    auto bundleRoot = cppms::testing::LIB_PATH + util::DIR_SEP;
    auto const& libPath = libName("TestBundleB");
    auto const& fullLibPath = bundleRoot + libPath;
    auto const& bundles = ctx.InstallBundles(fullLibPath, cppms::AnyMap(bLocationMap));
    ASSERT_EQ(2, bundles.size());
    for (auto b : bundles)
    {
        ASSERT_NO_THROW(b.Start());
    }
}
#endif

US_MSVC_POP_WARNING
