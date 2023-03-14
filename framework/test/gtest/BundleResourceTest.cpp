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

#include "cppmicroservices/BundleResource.h"
#include "TestUtils.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleResourceStream.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "gtest/gtest.h"
#include <unordered_set>

using namespace cppmicroservices;

struct ResourceComparator
{
    bool
    operator()(BundleResource const& mr1, BundleResource const& mr2) const
    {
        return mr1 < mr2;
    }
};

class BundleResourceTest : public ::testing::Test
{
  protected:
    Bundle testBundle;
    Bundle executableBundle;
    Framework framework;
    BundleContext context;

  public:
    BundleResourceTest() : framework(FrameworkFactory().NewFramework()) {}
    ~BundleResourceTest() override = default;

    void
    SetUp() override
    {
        framework.Start();
        context = framework.GetBundleContext();
        ASSERT_TRUE(context);
        testBundle = cppmicroservices::testing::InstallLib(context, "TestBundleR");
        executableBundle = cppmicroservices::testing::GetBundle("main", context);
        ASSERT_TRUE(executableBundle);
    }

    void
    TearDown() override
    {
        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }
};

void
checkResourceInfo(BundleResource const& res,
                  std::string const& path,
                  std::string const& baseName,
                  std::string const& completeBaseName,
                  std::string const& suffix,
                  std::string const& completeSuffix,
                  int size,
                  bool children = false)
{
    ASSERT_TRUE(res.IsValid());
    ASSERT_EQ(res.GetBaseName(), baseName);
    ASSERT_NE(res.GetChildren().empty(), children);
    ASSERT_EQ(res.GetCompleteBaseName(), completeBaseName);
    ASSERT_EQ(res.GetName(), completeBaseName + "." + suffix);
    ASSERT_EQ(res.GetResourcePath(), path + completeBaseName + "." + suffix);
    ASSERT_EQ(res.GetPath(), path);
    ASSERT_EQ(res.GetSize(), size);
    ASSERT_EQ(res.GetSuffix(), suffix);
    ASSERT_EQ(res.GetCompleteSuffix(), completeSuffix);
}

TEST(BundleResourceTestNoBundleInstall, operatorEqualTo)
{
    BundleResource a;
    BundleResource b;
    ASSERT_TRUE(a == b);
}

TEST(BundleResourceTestNoBundleInstall, getChildResourcesFromInvalidBundle)
{
    BundleResource result;
    ASSERT_EQ(result.GetChildResources().size(), static_cast<unsigned int>(0));
}

TEST_F(BundleResourceTest, getChildResources)
{
    BundleResource resource = testBundle.GetResource("icons/");

    // Confirm that GetChildResources() returns the correct number
    ASSERT_EQ(resource.GetChildResources().size(), static_cast<unsigned int>(3));
}

TEST_F(BundleResourceTest, testInvalidResource)
{
    BundleResource res = testBundle.GetResource("invalid");
    ASSERT_FALSE(res.IsValid());
    ASSERT_TRUE(res.GetName().empty());
    ASSERT_TRUE(res.GetPath().empty());
    ASSERT_TRUE(res.GetResourcePath().empty());
    ASSERT_TRUE(res.GetBaseName().empty());
    ASSERT_TRUE(res.GetCompleteBaseName().empty());
    ASSERT_TRUE(res.GetSuffix().empty());

    ASSERT_TRUE(res.GetChildren().empty());
    ASSERT_EQ(res.GetSize(), 0);
    ASSERT_EQ(res.GetCompressedSize(), 0);
    ASSERT_EQ(res.GetLastModified(), 0);
    ASSERT_EQ(res.GetCrc32(), 0);

    BundleResourceStream rs(res);
    ASSERT_TRUE(rs.good());
    rs.ignore();
    ASSERT_FALSE(rs.good());
    ASSERT_TRUE(rs.eof());
}

TEST_F(BundleResourceTest, testResourceTree)
{
    BundleResource res = testBundle.GetResource("");

    // Check root file path
    ASSERT_EQ(res.GetResourcePath(), "/");
    // Check type
    ASSERT_TRUE(res.IsDir());

    std::vector<std::string> children = res.GetChildren();
    std::sort(children.begin(), children.end());
    // Check child count
    ASSERT_EQ(children.size(), 6);
    // Check child name
    ASSERT_EQ(children[0], "foo.ptxt");
    ASSERT_EQ(children[1], "foo2.ptxt");
    ASSERT_EQ(children[2], "icons/");
    ASSERT_EQ(children[3], "manifest.json");
    ASSERT_EQ(children[4], "special_chars.dummy.ptxt");
    ASSERT_EQ(children[5], "test.xml");

    // Check not existant path
    ASSERT_TRUE(testBundle.FindResources("!$noexist=?", std::string(), "true").empty());

    BundleResource readme = testBundle.GetResource("/icons/readme.txt");
    // Check file resource
    ASSERT_TRUE(readme.IsFile() && readme.GetChildren().empty());

    BundleResource icons = testBundle.GetResource("icons/");
    // Check directory resource
    ASSERT_TRUE(icons.IsDir() && !icons.IsFile() && !icons.GetChildren().empty());

    children = icons.GetChildren();
    // Check icons child count
    ASSERT_EQ(children.size(), 3);
    std::sort(children.begin(), children.end());
    // Check child name
    ASSERT_EQ(children[0], "compressable.bmp");
    ASSERT_EQ(children[1], "cppmicroservices.png");
    ASSERT_EQ(children[2], "readme.txt");

    ResourceComparator resourceComparator;

    // find all .txt files
    std::vector<BundleResource> nodes = testBundle.FindResources("", "*.txt", false);
    std::sort(nodes.begin(), nodes.end(), resourceComparator);
    ASSERT_EQ(nodes.size(), 0);

    nodes = testBundle.FindResources("", "*.txt", true);
    std::sort(nodes.begin(), nodes.end(), resourceComparator);
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0].GetResourcePath(), "/icons/readme.txt");

    // find all .ptxt files
    nodes = testBundle.FindResources("", "*.ptxt", false);
    std::sort(nodes.begin(), nodes.end(), resourceComparator);
    ASSERT_EQ(nodes.size(), 3);
    // Check child name
    ASSERT_EQ(nodes[0].GetResourcePath(), "/foo.ptxt");
    ASSERT_EQ(nodes[1].GetResourcePath(), "/foo2.ptxt");
    ASSERT_EQ(nodes[2].GetResourcePath(), "/special_chars.dummy.ptxt");

    // find all resources
    nodes = testBundle.FindResources("", "", true);
    ASSERT_EQ(nodes.size(), 9);
    nodes = testBundle.FindResources("", "**", true);
    ASSERT_EQ(nodes.size(), 9);

    // test pattern matching
    nodes.clear();
    nodes = testBundle.FindResources("/icons", "*micro*.png", false);
    // Check file pattern matches
    ASSERT_EQ(nodes.size(), 1);
    // Check file pattern matches
    ASSERT_EQ(nodes[0].GetResourcePath(), "/icons/cppmicroservices.png");

    nodes.clear();
    nodes = testBundle.FindResources("", "*.txt", true);
    // Check recursive pattern matches
    ASSERT_EQ(nodes.size(), 1);
}

TEST_F(BundleResourceTest, testResourceOperators)
{
    BundleResource invalid = testBundle.GetResource("invalid");
    BundleResource foo = testBundle.GetResource("foo.ptxt");
    // Check valid resource
    ASSERT_TRUE(foo.IsValid() && foo);
    BundleResource foo2(foo);
    // Check equality operator
    ASSERT_EQ(foo, foo);
    // Check copy constructor and equality operator
    ASSERT_EQ(foo, foo2);
    // Check inequality with invalid resource
    ASSERT_NE(foo, invalid);

    BundleResource xml = testBundle.GetResource("/test.xml");
    // Check valid resource
    ASSERT_TRUE(xml.IsValid() && xml);
    // Check inequality
    ASSERT_NE(foo, xml);
    ASSERT_TRUE(foo < xml);

    // check operator< by using a set
    std::set<BundleResource> resources;
    resources.insert(foo);
    resources.insert(foo);
    resources.insert(xml);
    ASSERT_EQ(resources.size(), 2);

    // check hash function specialization
    std::unordered_set<BundleResource> resources2;
    resources2.insert(foo);
    resources2.insert(foo);
    resources2.insert(xml);
    // Check operator< with unordered set
    ASSERT_EQ(resources2.size(), 2);

    // check operator<<
    std::ostringstream oss;
    oss << foo;
    ASSERT_EQ(oss.str(), foo.GetResourcePath());
}

TEST_F(BundleResourceTest, testTextResource)
{
    BundleResource res = testBundle.GetResource("foo.ptxt");
    checkResourceInfo(res, "/", "foo", "foo", "ptxt", "ptxt", 13, false);

#ifdef US_PLATFORM_WINDOWS
    const std::streampos ssize(13);
    const std::string fileData = "foo and\nbar\n\n";
#else
    const std::streampos ssize(12);
    const std::string fileData = "foo and\nbar\n";
#endif

    BundleResourceStream rs(res);

    rs.seekg(0, std::ios::end);
    // Check Stream content length
    ASSERT_EQ(rs.tellg(), ssize);
    rs.seekg(0, std::ios::beg);

    std::string content;
    content.reserve(res.GetSize());
    char buffer[1024];
    while (rs.read(buffer, sizeof(buffer)))
    {
        content.append(buffer, sizeof(buffer));
    }
    content.append(buffer, static_cast<std::size_t>(rs.gcount()));

    ASSERT_TRUE(rs.eof());
    ASSERT_EQ(content, fileData);

    rs.clear();
    rs.seekg(0);

    ASSERT_EQ(rs.tellg(), std::streampos(0));
    ASSERT_TRUE(rs.good());

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(rs, line))
    {
        lines.push_back(line);
    }
    ASSERT_GT(static_cast<int>(lines.size()), 1);
    ASSERT_EQ(lines[0], "foo and");
    ASSERT_EQ(lines[1], "bar");
}

TEST_F(BundleResourceTest, testTextResourceAsBinary)
{
    BundleResource res = testBundle.GetResource("foo.ptxt");

    checkResourceInfo(res, "/", "foo", "foo", "ptxt", "ptxt", 13, false);

    /*
        Note: there is no ifdef for platform in this test case because tellg()
        reports the byte offset from the beginning of the file since it is being
        read as binary. This is not the case for when the file is read as text.

        The same is true for fileData; when read as binary data, linux will pick up
        the last newline character, but if read as text, it  won't. In this case, since
        the data is being read as binary data, no ifdef is necessary
      */

    const std::streampos ssize(13);
    const std::string fileData = "foo and\nbar\n\n";

    BundleResourceStream rs(res, std::ios_base::binary);

    rs.seekg(0, std::ios::end);
    // Check Stream content length
    ASSERT_EQ(rs.tellg(), ssize);
    rs.seekg(0, std::ios::beg);

    std::string content;
    content.reserve(res.GetSize());
    char buffer[1024];
    while (rs.read(buffer, sizeof(buffer)))
    {
        content.append(buffer, sizeof(buffer));
    }
    content.append(buffer, static_cast<std::size_t>(rs.gcount()));

    ASSERT_TRUE(rs.eof());
    ASSERT_EQ(content, fileData);
}

TEST_F(BundleResourceTest, testBinaryResource)
{
    BundleResource res = testBundle.GetResource("/icons/cppmicroservices.png");
    checkResourceInfo(res, "/icons/", "cppmicroservices", "cppmicroservices", "png", "png", 2424, false);

    BundleResourceStream rs(res, std::ios_base::binary);
    rs.seekg(0, std::ios_base::end);
    std::streampos resLength = rs.tellg();
    rs.seekg(0);

    std::ifstream png(US_FRAMEWORK_SOURCE_DIR "/test/bundles/libRWithResources/resources/icons/cppmicroservices.png",
                      std::ifstream::in | std::ifstream::binary);

    ASSERT_TRUE(png.is_open());

    png.seekg(0, std::ios_base::end);
    std::streampos pngLength = png.tellg();
    png.seekg(0);
    ASSERT_EQ(res.GetSize(), resLength);
    ASSERT_EQ(resLength, pngLength);

    char c1 = 0;
    char c2 = 0;
    bool isEqual = true;
    int count = 0;
    while (png.get(c1) && rs.get(c2))
    {
        ++count;
        if (c1 != c2)
        {
            isEqual = false;
            break;
        }
    }

    // Check if everything was read
    ASSERT_EQ(count, pngLength);
    ASSERT_TRUE(isEqual);
    ASSERT_TRUE(png.eof());
}

TEST_F(BundleResourceTest, testCompressedResource)
{
    BundleResource res = testBundle.GetResource("/icons/compressable.bmp");
    checkResourceInfo(res, "/icons/", "compressable", "compressable", "bmp", "bmp", 300122, false);

    BundleResourceStream rs(res, std::ios_base::binary);
    rs.seekg(0, std::ios_base::end);
    std::streampos resLength = rs.tellg();
    rs.seekg(0);

    std::ifstream bmp(US_FRAMEWORK_SOURCE_DIR "/test/bundles/libRWithResources/resources/icons/compressable.bmp",
                      std::ifstream::in | std::ifstream::binary);

    ASSERT_TRUE(bmp.is_open());

    bmp.seekg(0, std::ios_base::end);
    std::streampos bmpLength = bmp.tellg();
    bmp.seekg(0);
    // Check resource size
    ASSERT_EQ(300122, resLength);
    ASSERT_EQ(resLength, bmpLength);

    char c1 = 0;
    char c2 = 0;
    bool isEqual = true;
    int count = 0;
    while (bmp.get(c1) && rs.get(c2))
    {
        ++count;
        if (c1 != c2)
        {
            isEqual = false;
            break;
        }
    }

    ASSERT_EQ(count, bmpLength);
    ASSERT_TRUE(isEqual);
    ASSERT_TRUE(bmp.eof());
}

TEST_F(BundleResourceTest, testResources)
{
    BundleResource foo = testBundle.GetResource("foo.ptxt");
    ASSERT_TRUE(foo.IsValid());

    // Check resourse count
    auto testBundleRL = cppmicroservices::testing::InstallLib(context, "TestBundleRL");
    ASSERT_EQ(testBundleRL.FindResources("", "*.txt", true).size(), 2);

    auto testBundleRA = cppmicroservices::testing::InstallLib(context, "TestBundleRA");
    ASSERT_EQ(testBundleRA.FindResources("", "*.txt", true).size(), 2);
}

TEST_F(BundleResourceTest, testResourceFromExecutable)
{
    BundleResource resource = executableBundle.GetResource("TestResource.ptxt");
    ASSERT_TRUE(resource.IsValid());

    std::string line;
    BundleResourceStream rs(resource);
    std::getline(rs, line);
    ASSERT_EQ(line, "meant to be compiled into the test driver");
}

// Note: Following test has broken encoding
TEST_F(BundleResourceTest, testSpecialCharacters)
{
    BundleResource res = testBundle.GetResource("special_chars.dummy.ptxt");
    checkResourceInfo(res, "/", "special_chars", "special_chars.dummy", "ptxt", "dummy.ptxt", 54, false);

#ifdef US_PLATFORM_WINDOWS
    const std::streampos ssize(54);
    const std::string fileData = "German Füße (feet)\nFrench garçon de café (waiter)\n";
#else
    const std::streampos ssize(53);
    const std::string fileData = "German Füße (feet)\nFrench garçon de café (waiter)";
#endif

    BundleResourceStream rs(res);

    rs.seekg(0, std::ios_base::end);
    // Check Stream content length
    ASSERT_EQ(rs.tellg(), ssize);
    rs.seekg(0, std::ios_base::beg);

    std::string content;
    content.reserve(res.GetSize());
    char buffer[1024];
    while (rs.read(buffer, sizeof(buffer)))
    {
        content.append(buffer, sizeof(buffer));
    }
    content.append(buffer, static_cast<std::size_t>(rs.gcount()));

    ASSERT_TRUE(rs.eof());
    ASSERT_EQ(content, fileData);
}

class BundleResourceDataOnlyTest : public ::testing::Test
{
  protected:
    Framework framework;
    BundleContext context;

  public:
    BundleResourceDataOnlyTest() : framework(FrameworkFactory().NewFramework()) {}
    ~BundleResourceDataOnlyTest() override = default;

    void
    SetUp() override
    {
        framework.Start();
        context = framework.GetBundleContext();
        ASSERT_TRUE(context);
    }

    void
    TearDown() override
    {
        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }
};

TEST_F(BundleResourceDataOnlyTest, TestResourceContainerGetsOpened)
{
    cppmicroservices::AnyMap fakeManifest(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    cppmicroservices::AnyMap manifestForTestBundleR(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    manifestForTestBundleR["bundle.symbolic_name"] = std::string("TestBundleR");
    fakeManifest["TestBundleR"] = manifestForTestBundleR;

    // Install bundle with fake manifest cache entry
    auto testBundle = cppmicroservices::testing::InstallLib(context, "TestBundleR", fakeManifest);
    ASSERT_TRUE(testBundle);

    // Try to get resource, should not throw and resources should be returend.
    auto resources = testBundle.FindResources("icons", "*", true);
    ASSERT_FALSE(resources.empty());
    ASSERT_EQ(resources.size(), 3);
}
