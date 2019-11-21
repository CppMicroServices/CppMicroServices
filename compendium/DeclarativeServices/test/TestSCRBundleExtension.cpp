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

#include <chrono>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/BundleContext.h>
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "../src/SCRBundleExtension.hpp"
#include "Mocks.hpp"
#include "../src/metadata/Util.hpp"

#define str(s) #s
#define xstr(s) str(s)

using cppmicroservices::Any;
using cppmicroservices::service::component::ComponentConstants::SERVICE_COMPONENT;

namespace cppmicroservices{
namespace scrimpl {

using cppmicroservices::AnyMap;
// The fixture for testing class SCRActivator.
class SCRBundleExtensionTest
  : public ::testing::Test
{
protected:
  SCRBundleExtensionTest() : framework(cppmicroservices::FrameworkFactory().NewFramework())
  { }
  ~SCRBundleExtensionTest() = default;

  void SetUp() override {
    framework.Start();
  }

  void TearDown() override {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Framework& GetFramework() { return framework; }
private:
  cppmicroservices::Framework framework;
};

TEST_F(SCRBundleExtensionTest, CtorInvalidArgs)
{
  cppmicroservices::AnyMap headers(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto fakeLogger = std::make_shared<FakeLogger>();
  EXPECT_THROW({
      SCRBundleExtension bundleExt(BundleContext(),
                                   headers,
                                   mockRegistry,
                                   fakeLogger);
    }, std::invalid_argument);
  EXPECT_THROW({
      SCRBundleExtension bundleExt(GetFramework().GetBundleContext(),
                                   headers,
                                   nullptr,
                                   fakeLogger);
    }, std::invalid_argument);
  EXPECT_THROW({
      SCRBundleExtension bundleExt(GetFramework().GetBundleContext(),
                                   headers,
                                   mockRegistry,
                                   nullptr);
    }, std::invalid_argument);
  EXPECT_THROW({
      SCRBundleExtension bundleExt(GetFramework().GetBundleContext(),
                                   headers,
                                   mockRegistry,
                                   fakeLogger);
    }, std::invalid_argument);
}

TEST_F(SCRBundleExtensionTest, CtorWithValidArgs)
{
  auto bundles = GetFramework().GetBundleContext().GetBundles();
  auto thisBundleItr = std::find_if(bundles.begin(), bundles.end(), [](const cppmicroservices::Bundle& bundle){
                                                                      return (bundle.GetSymbolicName() == xstr(US_BUNDLE_NAME));
                                                                    });
  auto thisBundle = thisBundleItr != bundles.end() ? *thisBundleItr : cppmicroservices::Bundle();
  ASSERT_TRUE(static_cast<bool>(thisBundle));
  auto const& scr = ref_any_cast<cppmicroservices::AnyMap>(thisBundle.GetHeaders().at("scr_test_0"));

  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  EXPECT_CALL(*mockRegistry, AddComponentManager(testing::_))
    .Times(2)
    .WillOnce(testing::Throw(std::runtime_error("Failed to add component")))
    .WillOnce(testing::Return(true));
  EXPECT_CALL(*mockRegistry, RemoveComponentManager(testing::_))
    .Times(1);
  auto fakeLogger = std::make_shared<FakeLogger>();
  EXPECT_NO_THROW({
      SCRBundleExtension bundleExt(GetFramework().GetBundleContext(),
                                   scr,
                                   mockRegistry,
                                   fakeLogger);
      EXPECT_EQ(bundleExt.managers.size(), 0u);
    });
  EXPECT_NO_THROW({
      SCRBundleExtension bundleExt(GetFramework().GetBundleContext(),
                                   scr,
                                   mockRegistry,
                                   fakeLogger);
      EXPECT_EQ(bundleExt.managers.size(), 1u);
    });
}
}
}
