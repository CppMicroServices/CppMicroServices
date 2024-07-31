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

#include "Mocks.h"
#include "MockUtils.h"

#include "../../src/bundle/BundleManifest.h"
#include "cppmicroservices/BundleResourceStream.h"

#include <iostream>

US_MSVC_PUSH_DISABLE_WARNING(4996)

using namespace cppmicroservices;
using ::testing::_;
using ::testing::Eq;
using ::testing::Return;
using ::testing::AtLeast;

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
    BundleManifestTest()
        : bundleStorage(new MockBundleStorageMemory())
        , mockEnv(MockedEnvironment(bundleStorage))
        , framework(mockEnv.framework) {}
    virtual ~BundleManifestTest()
    {}

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

    cppmicroservices::MockBundleStorageMemory* bundleStorage;
    cppmicroservices::MockedEnvironment mockEnv;
    cppmicroservices::Framework& framework;
};

TEST_F(BundleManifestTest, UnicodeProperty)
{
    std::string bundleName = "TestBundleU";
    AnyMap manifest = AnyMap({
        { "bundle.activator", Any(true) },
        { "bundle.symbolic_name", Any(std::string(bundleName)) },
        { "unicode.sample", Any(std::string(u8"电脑 くいりのまちとこしくそ")) }
    });

    std::vector<std::string> files = {"TestBundleU"};
    auto resCont = std::make_shared<MockBundleResourceContainer>();
    ON_CALL(*resCont, GetTopLevelDirs())
        .WillByDefault(Return(files));
    EXPECT_CALL(*resCont, GetTopLevelDirs()).Times(1);

    ON_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(bundleName), _))
        .WillByDefault(Return(std::make_shared<MockBundleArchive>(
            bundleStorage,
            resCont,
            "MockTestBundleU", bundleName, 1,
            manifest
        )));
    EXPECT_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(bundleName), _)).Times(1);

    auto bundles = mockEnv.Install(bundleName, manifest, resCont);
    ASSERT_EQ(1, bundles.size()) << "Mock bundle failed to install correctly.";
    auto bundle = bundles.at(0);
    std::string expectedValue = u8"电脑 くいりのまちとこしくそ";
    std::string actualValue = bundle.GetHeaders().at("unicode.sample").ToString();
    ASSERT_STREQ(expectedValue.c_str(), actualValue.c_str()) << "Unicode data from manifest.json doesn't match expected value.";
}

#define MAKE_DEEP(x) AnyMap({ \
        { "relativelylongkeyname_element", Any(true) }, \
        { "relativelylongkeyname_map", x } \
    })
#define MAKE_DEEP_2(x) MAKE_DEEP(MAKE_DEEP(x))
#define MAKE_DEEP_4(x) MAKE_DEEP_2(MAKE_DEEP_2(x))
#define MAKE_DEEP_8(x) MAKE_DEEP_4(MAKE_DEEP_4(x))
#define MAKE_DEEP_16(x) MAKE_DEEP_8(MAKE_DEEP_8(x))
TEST_F(BundleManifestTest, InstallBundleWithDeepManifest)
{
    std::string bundleName = "TestBundleWithDeepManifest";
    AnyMap manifest = AnyMap({
        { "Test_AtCompoundKey", MAKE_DEEP_16(AnyMap()) },
        { "bundle.activator", Any(true) },
        { "bundle.symbolic_name", Any(std::string(bundleName)) },
        { "bundle.vendor", Any(std::string("The Company, Inc.")) }
    });

    std::vector<std::string> files = {"TestBundleWithDeepManifest"};
    auto resCont = std::make_shared<MockBundleResourceContainer>();
    ON_CALL(*resCont, GetTopLevelDirs())
        .WillByDefault(Return(files));
    EXPECT_CALL(*resCont, GetTopLevelDirs()).Times(1);

    ON_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(bundleName), _))
        .WillByDefault(Return(std::make_shared<MockBundleArchive>(
            bundleStorage,
            resCont,
            "MockTestBundleWithDeepManifest", bundleName, 1,
            manifest
        )));
    EXPECT_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(bundleName), _)).Times(1);

    auto bundles = mockEnv.Install(bundleName, manifest, resCont);
    ASSERT_EQ(1, bundles.size()) << "Mock bundle failed to install correctly.";
    auto bundle = bundles.at(0);
    auto const& headers = bundle.GetHeaders();
    ASSERT_THAT(headers.at(Constants::BUNDLE_SYMBOLICNAME).ToString(), ::testing::StrEq("TestBundleWithDeepManifest"))
        << "Bundle symbolic name doesn't match.";

    // The same key/value must continue to exist in the deprecated properties map.
    auto deprecatedProperties = bundle.GetProperties();
    ASSERT_TRUE(compare_deprecated_properties(headers, deprecatedProperties)) << "Deprecated properties mismatch";
}

TEST_F(BundleManifestTest, ParseManifest)
{
    std::istringstream manifest = std::istringstream("{"
        "\"bundle.symbolic_name\": \"TestBundleM\","
        "\"bundle.description\": \"My Bundle description\","
        "\"bundle.version\": \"1.0.0\","
        "\"bundle.activator\" : true,"
        "\"number\": 5,"
        "\"double\": 1.1,"
        "\"vector\": ["
            "\"first\","
            "2,"
            "\"third\""
        "],"
        "\"map\": {"
            "\"string\": \"hi\","
            "\"number\": 4,"
            "\"list\": [ \"a\", \"b\" ]"
        "}"
    "}");

    BundleManifest bundleM;
    bundleM.Parse(manifest);

    auto const& headers = bundleM.GetHeaders();
    EXPECT_THAT(headers.at(Constants::BUNDLE_SYMBOLICNAME).ToString(), ::testing::StrEq("TestBundleM"));
    EXPECT_THAT(headers.at(Constants::BUNDLE_DESCRIPTION).ToString(), ::testing::StrEq("My Bundle description"));
    EXPECT_THAT(headers.at(Constants::BUNDLE_VERSION).ToString(), ::testing::StrEq("1.0.0"));

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

    struct TestBundleA : public cppmicroservices::TestBundleAService
    {
        virtual ~TestBundleA() {}
    };

    class TestBundleAActivator : public cppmicroservices::BundleActivator
    {
      public:
        TestBundleAActivator() {}
        ~TestBundleAActivator() {}

        void Start(BundleContext context)
        {
            s = std::make_shared<TestBundleA>();
            sr = context.RegisterService<TestBundleAService>(s);
        }

        void Stop(BundleContext)
        {
            sr.Unregister();
        }

      private:
        std::shared_ptr<TestBundleA> s;
        ServiceRegistration<TestBundleAService> sr;
    };

    struct TestBundleBActivator : public cppmicroservices::BundleActivator
    {
      public:
        void Start(BundleContext)
        {}

        void Stop(BundleContext)
        {}
    };

    struct TestBundleImportedByBActivator : public cppmicroservices::BundleActivator
    {
      public:
        void Start(BundleContext)
        {}

        void Stop(BundleContext)
        {}
    };

    template<typename T>
    BundleActivator* createActivator()
    {
        return new T();
    }
    void destroyActivator(BundleActivator* bundleActivator)
    {
        delete bundleActivator;
    }

    using map_type = std::map<std::string, BundleActivator*(*)()>;
    map_type activators{
        { "TestBundleA", &createActivator<TestBundleAActivator> },
        { "TestBundleA2", &createActivator<TestBundleAActivator> },
        { "TestBundleB", &createActivator<TestBundleBActivator> },
        { "TestBundleImportedByB", &createActivator<TestBundleImportedByBActivator> }
    };

} // namespace cppmicroservices

#ifdef US_BUILD_SHARED_LIBS
TEST_F(BundleManifestTest, DirectManifestInstall)
{
    std::vector<std::string> files = {"TestBundleA"};
    cppmicroservices::AnyMap manifests(cppmicroservices::any_map::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    cppmicroservices::AnyMap::unordered_any_cimap testBundleAManifest = {
        {"bundle.symbolic_name", std::string("TestBundleA")},
        {    "bundle.activator",                       true}
    };
    manifests["TestBundleA"] = cppmicroservices::AnyMap(testBundleAManifest);

    auto resCont = std::make_shared<MockBundleResourceContainer>();
    ON_CALL(*resCont, GetTopLevelDirs())
        .WillByDefault(Return(files));
    EXPECT_CALL(*resCont, GetTopLevelDirs()).Times(1);

    ON_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(files[0]), _))
        .WillByDefault(Return(std::make_shared<MockBundleArchive>(
            bundleStorage,
            resCont,
            files[0], files[0], 1,
            testBundleAManifest
        )));
    EXPECT_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(files[0]), _)).Times(1);

    auto libPath = fullLibPath("TestBundleA");
    auto const& bundles = mockEnv.Install(libPath, manifests, resCont);

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
    std::vector<std::string> files = {"TestBundleA"};
    cppmicroservices::AnyMap manifests(cppmicroservices::any_map::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    cppmicroservices::AnyMap::unordered_any_cimap testBundleAManifest = {
  // We need to have at least one entry in the manifest to check and make sure it has what's
  // required. This is because if the manifest is empty, it is assumed that we are NOT injecting a
  // manifest and we go through the standard install by reading the manifest from the bundle
  // itself.
        {"foo", std::string("bar")}
    };
    manifests["TestBundleA"] = cppmicroservices::AnyMap(testBundleAManifest);

    auto libPath = fullLibPath("TestBundleA");

    auto resCont = std::make_shared<MockBundleResourceContainer>();
    ON_CALL(*resCont, GetTopLevelDirs())
        .WillByDefault(Return(files));
    EXPECT_CALL(*resCont, GetTopLevelDirs()).Times(1);

    ON_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(files[0]), _))
        .WillByDefault(Return(std::make_shared<MockBundleArchive>(
            bundleStorage,
            resCont,
            files[0], files[0], 1,
            testBundleAManifest
        )));
    EXPECT_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(files[0]), _)).Times(1);
    EXPECT_CALL(*bundleStorage, RemoveArchive(_)).Times(1);

    EXPECT_THROW({ mockEnv.Install(libPath, manifests, resCont); }, std::runtime_error);
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
    std::vector<std::string> files = {"TestBundleA", "TestBundleB"};
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

    auto resCont = std::make_shared<MockBundleResourceContainer>();
    ON_CALL(*resCont, GetTopLevelDirs())
        .WillByDefault(Return(files));
    EXPECT_CALL(*resCont, GetTopLevelDirs()).Times(1);

    ON_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(files[0]), _))
        .WillByDefault(Return(std::make_shared<MockBundleArchive>(
            bundleStorage,
            resCont,
            files[0], files[0], 1,
            testBundleAManifest
        )));
    EXPECT_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(files[0]), _)).Times(1);

    ON_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(files[1]), _))
        .WillByDefault(Return(std::make_shared<MockBundleArchive>(
            bundleStorage,
            resCont,
            files[1], files[1], 2,
            testBundleBManifest
        )));
    EXPECT_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(files[1]), _)).Times(1);

    auto libPath = fullLibPath("TestBundleA");
    auto const& bundles = mockEnv.Install(libPath, manifests, resCont);
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

    std::vector<std::string> files = {"TestBundleA"};
    cppmicroservices::AnyMap manifests(cppmicroservices::any_map::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    cppmicroservices::AnyMap::unordered_any_cimap testBundleAManifest = {
        {                   "A",                        1.5},
        {                   "B",        std::string("Test")},
        {"bundle.symbolic_name", std::string("TestBundleA")},
        {    "bundle.activator",                       true}
    };
    manifests["TestBundleA"] = cppmicroservices::AnyMap(testBundleAManifest);

    auto resCont = std::make_shared<MockBundleResourceContainer>();
    ON_CALL(*resCont, GetTopLevelDirs())
        .WillByDefault(Return(files));
    EXPECT_CALL(*resCont, GetTopLevelDirs()).Times(1);

    ON_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(files[0]), _))
        .WillByDefault(Return(std::make_shared<MockBundleArchive>(
            bundleStorage,
            resCont,
            files[0], files[0], 1,
            testBundleAManifest
        )));
    EXPECT_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(files[0]), _)).Times(1);

    // Inject mocked SharedLibrary to prevent framework bundle from being loaded
    MockSharedLibrary* sharedLib = new MockSharedLibrary();
    EXPECT_CALL(*sharedLib, Load(_)).Times(1);
    mockEnv.bundlePrivate->lib = sharedLib;

    auto location = fullLibPath("TestBundleA");
    auto const& bundles = mockEnv.Install(location, manifests, resCont);
    ASSERT_EQ(1, bundles.size());
    auto b = bundles[0];

    // Inject mocked SharedLibrary to prevent libA.so from being loaded
    auto priv = GetPrivate(b);
    delete priv->lib;
    priv->lib = sharedLib;

    std::string createActivatorFunc = US_STR(US_CREATE_ACTIVATOR_PREFIX) + files[0];
    std::string destroyActivatorFunc = US_STR(US_DESTROY_ACTIVATOR_PREFIX) + files[0];

    MockBundleUtils* bundleUtils = new MockBundleUtils();
    ON_CALL(*bundleUtils, GetSymbol(_, Eq(createActivatorFunc), _))
        .WillByDefault(Return(reinterpret_cast<void*>(cppmicroservices::activators["TestBundleA"])));
    ON_CALL(*bundleUtils, GetSymbol(_, Eq(destroyActivatorFunc), _))
        .WillByDefault(Return(reinterpret_cast<void*>(&destroyActivator)));
    EXPECT_CALL(*bundleUtils, GetSymbol(_, _, _)).Times(3);
    delete priv->bundleUtils;
    priv->bundleUtils = bundleUtils;

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

    delete bundleUtils;
    delete sharedLib;
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

    // Inject mocked SharedLibrary to prevent framework bundle from being loaded
    MockSharedLibrary* sharedLib = new MockSharedLibrary();
    EXPECT_CALL(*sharedLib, Load(_)).Times(2);
    mockEnv.bundlePrivate->lib = sharedLib;

    // Now use the new API to install bundles with the path and a manifest stored in an AnyMap.
    for (int i = 0; i < 2; i++)
    {
        std::string libPath = i == 0 ? "TestBundleA" : "TestBundleA2";
        AnyMap manifest = i == 0 ? aManifest : a2Manifest;

        std::vector<std::string> files = { libPath };
        auto resCont = std::make_shared<MockBundleResourceContainer>();
        ON_CALL(*resCont, GetTopLevelDirs())
            .WillByDefault(Return(files));
        EXPECT_CALL(*resCont, GetTopLevelDirs()).Times(1);

        EXPECT_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(libPath), _))
            .WillOnce(Return(std::make_shared<MockBundleArchive>(
                bundleStorage,
                resCont,
                libPath, libPath, i,
                manifest
            )));

        auto location = fullLibPath(libPath);
        auto const& bundles = mockEnv.Install(location, manifests, resCont);
        ASSERT_EQ(1, bundles.size());
        for (auto b : bundles)
        {
            // Inject mocked SharedLibrary to prevent *.so from being loaded
            auto priv = GetPrivate(b);
            delete priv->lib;
            priv->lib = sharedLib;

            std::string createActivatorFunc = US_STR(US_CREATE_ACTIVATOR_PREFIX) + libPath;
            std::string destroyActivatorFunc = US_STR(US_DESTROY_ACTIVATOR_PREFIX) + libPath;

            MockBundleUtils* bundleUtils = new MockBundleUtils();
            ON_CALL(*bundleUtils, GetSymbol(_, Eq(createActivatorFunc), _))
                .WillByDefault(Return(reinterpret_cast<void*>(cppmicroservices::activators[libPath])));
            ON_CALL(*bundleUtils, GetSymbol(_, Eq(destroyActivatorFunc), _))
                .WillByDefault(Return(reinterpret_cast<void*>(&destroyActivator)));
            EXPECT_CALL(*bundleUtils, GetSymbol(_, _, _)).Times(3);
            delete priv->bundleUtils;
            priv->bundleUtils = bundleUtils;

            ASSERT_NO_THROW(b.Start());
            delete bundleUtils;
        }
    }

    delete sharedLib;
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
    auto libPath = fullLibPath("TestBundleA");
    std::vector<std::string> files = { libPath };
    auto resCont = std::make_shared<MockBundleResourceContainer>();
    EXPECT_CALL(*resCont, GetTopLevelDirs())
        .WillRepeatedly(Return(files));

    EXPECT_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(libPath), _))
        .WillOnce(Return(std::make_shared<MockBundleArchive>(
            bundleStorage,
            resCont,
            libPath, libPath, 1,
            firstTimeManifest
        )))
        .WillOnce(Return(std::make_shared<MockBundleArchive>(
            bundleStorage,
            resCont,
            libPath, libPath, 2,
            secondTimeManifest
        )));
    EXPECT_CALL(*bundleStorage, RemoveArchive(_)).Times(AtLeast(1));

    auto const& firstBundles = mockEnv.Install(libPath, manifests, resCont);
    auto const& firstHeaders = firstBundles[0].GetHeaders();
    ASSERT_TRUE(any_cast<bool>(firstHeaders.at("test")));

    // on second installation the manifest should be ignored, so our test value should remain true.
    manifests["TestBundleA"] = AnyMap(secondTimeManifest);
    EXPECT_THROW({ mockEnv.Install(libPath, manifests, resCont); }, std::runtime_error);
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

    // Inject mocked SharedLibrary to prevent framework bundle from being loaded
    MockSharedLibrary* sharedLib = new MockSharedLibrary();
    EXPECT_CALL(*sharedLib, Load(_)).Times(2);
    mockEnv.bundlePrivate->lib = sharedLib;

    std::vector<std::string> files = { "TestBundleB", "TestBundleImportedByB" };
    auto resCont = std::make_shared<MockBundleResourceContainer>();
    EXPECT_CALL(*resCont, GetTopLevelDirs())
        .WillRepeatedly(Return(files));

    EXPECT_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq("TestBundleB"), _))
        .WillOnce(Return(std::make_shared<MockBundleArchive>(
            bundleStorage,
            resCont,
            "TestBundleB", "TestBundleB", 1,
            bManifest
        )));
    EXPECT_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq("TestBundleImportedByB"), _))
        .WillOnce(Return(std::make_shared<MockBundleArchive>(
            bundleStorage,
            resCont,
            "TestBundleImportedByB", "TestBundleImportedByB", 2,
            b2Manifest
        )));

    // Now use the new API to install bundles with the path and a manifest stored in an AnyMap.
    auto libPath = libName("TestBundleB");
    auto locs = cppms::AnyMap(bLocationMap);
    auto const& bundles = mockEnv.Install(libPath, locs, resCont);
    ASSERT_EQ(2, bundles.size());
    for (auto b : bundles)
    {
        auto symbolicName = b.GetProperty("bundle.symbolic_name").ToString();

        // Inject mocked SharedLibrary to prevent libA.so from being loaded
        auto priv = GetPrivate(b);
        delete priv->lib;
        priv->lib = sharedLib;

        std::string createActivatorFunc = US_STR(US_CREATE_ACTIVATOR_PREFIX) + symbolicName;
        std::string destroyActivatorFunc = US_STR(US_DESTROY_ACTIVATOR_PREFIX) + symbolicName;

        MockBundleUtils* bundleUtils = new MockBundleUtils();
        ON_CALL(*bundleUtils, GetSymbol(_, Eq(createActivatorFunc), _))
            .WillByDefault(Return(reinterpret_cast<void*>(activators[symbolicName])));
        ON_CALL(*bundleUtils, GetSymbol(_, Eq(destroyActivatorFunc), _))
            .WillByDefault(Return(reinterpret_cast<void*>(&destroyActivator)));
        EXPECT_CALL(*bundleUtils, GetSymbol(_, _, _)).Times(3);
        delete priv->bundleUtils;
        priv->bundleUtils = bundleUtils;

        ASSERT_NO_THROW(b.Start());
        delete bundleUtils;
    }

    delete sharedLib;
}
#endif

US_MSVC_POP_WARNING
