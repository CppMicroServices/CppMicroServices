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

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleResource.h"
#include "cppmicroservices/BundleResourceStream.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "TestUtils.h"

#include "gtest/gtest.h"

using namespace cppmicroservices;

std::string
GetResourceContent(BundleResource const& resource)
{
    std::string line;
    BundleResourceStream rs(resource);
    std::getline(rs, line);
    return line;
}

struct ResourceComparator
{
    bool
    operator()(BundleResource const& mr1, BundleResource const& mr2) const
    {
        return mr1 < mr2;
    }
};

class StaticBundleResourceTest : public ::testing::Test
{
  protected:
    Bundle testBundle;
    Bundle importedBundle;
    Framework framework;

  public:
    StaticBundleResourceTest() : framework(FrameworkFactory().NewFramework()) {};

    ~StaticBundleResourceTest() override = default;

    void
    SetUp() override
    {
        framework.Start();
        ASSERT_TRUE(framework.GetBundleContext());
        testBundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleB");
        ASSERT_TRUE(testBundle);
        importedBundle = cppmicroservices::testing::GetBundle("TestBundleImportedByB", framework.GetBundleContext());
        ASSERT_TRUE(importedBundle);
    }

    void
    TearDown() override
    {
        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }
};

TEST_F(StaticBundleResourceTest, testResourceOperators)
{
    std::vector<BundleResource> resources = testBundle.FindResources("", "res.ptxt", false);
    // Check resource count
    ASSERT_EQ(resources.size(), 1);
}

TEST_F(StaticBundleResourceTest, testResourcesWithStaticImport)
{
    BundleResource resource = testBundle.GetResource("res.ptxt");
    // Check valid res.txt resource
    ASSERT_TRUE(resource.IsValid());
    std::string line = GetResourceContent(resource);
    // Check dynamic resource content
    ASSERT_EQ(line, "dynamic resource");

    resource = testBundle.GetResource("dynamic.ptxt");
    // Check valid dynamic.txt resource
    ASSERT_TRUE(resource.IsValid());
    line = GetResourceContent(resource);
    // Check dynamic resource content
    ASSERT_EQ(line, "dynamic");

    resource = testBundle.GetResource("static.ptxt");
    // Check in-valid static.txt resource
    ASSERT_FALSE(resource.IsValid());

    resource = importedBundle.GetResource("static.ptxt");
    // Check valid static.txt resource
    ASSERT_TRUE(resource.IsValid());
    line = GetResourceContent(resource);
    // Check static resource content
    ASSERT_EQ(line, "static");

    std::vector<BundleResource> resources = testBundle.FindResources("", "*.ptxt", false);
    std::stable_sort(resources.begin(), resources.end(), ResourceComparator());
    std::vector<BundleResource> importedResources = importedBundle.FindResources("", "*.ptxt", false);
    std::stable_sort(importedResources.begin(), importedResources.end(), ResourceComparator());

    // Check resource count
    ASSERT_EQ(resources.size(), 2);
    ASSERT_EQ(importedResources.size(), 2);
    line = GetResourceContent(resources[0]);
    // Check dynamic.txt resource content
    ASSERT_EQ(line, "dynamic");
    line = GetResourceContent(resources[1]);
    // Check res.txt (from importing bundle) resource content
    ASSERT_EQ(line, "dynamic resource");
    line = GetResourceContent(importedResources[0]);
    ASSERT_EQ(line, "static resource");
    line = GetResourceContent(importedResources[1]);
    // Check static.txt (from importing bundle) resource content
    ASSERT_EQ(line, "static");
}

TEST_F(StaticBundleResourceTest, testResources)
{
    BundleResource resource = importedBundle.GetResource("static.ptxt");
    testBundle.Start();
    // Check valid static.txt resource
    ASSERT_TRUE(resource.IsValid());

    testBundle.Stop();
    // Check still valid static.txt resource
    ASSERT_TRUE(resource.IsValid());
}
