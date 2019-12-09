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
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleResource.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "gtest/gtest.h"

using namespace cppmicroservices;

class BundleResourceTest : public ::testing::Test
{
protected:
  Bundle    bundleR;
  Framework f;

public:
  BundleResourceTest() : f(FrameworkFactory().NewFramework()) {}
  ~BundleResourceTest() override = default;

  void SetUp() override
  {
    f.Start();
    auto fCtx = f.GetBundleContext();
    bundleR = cppmicroservices::testing::InstallLib(fCtx, "TestBundleR");
  }

  void TearDown() override
  {
    f.Stop();
    f.WaitForStop(std::chrono::milliseconds::zero());
  }
};

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
  BundleResource resource = bundleR.GetResource("icons/");

  // Confirm that GetChildResources() returns the correct number
  ASSERT_EQ(resource.GetChildResources().size(), static_cast<unsigned int>(3));
}
