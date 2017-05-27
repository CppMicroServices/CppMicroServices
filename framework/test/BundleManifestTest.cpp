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

#include "cppmicroservices/Any.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "TestingConfig.h"
#include "TestingMacros.h"
#include "TestUtils.h"

using namespace cppmicroservices;

void TestUnicodeProperty(BundleContext bc)
{
#if defined(US_BUILD_SHARED_LIBS) && US_CXX_UNICODE_LITERALS
	std::string path_utf8 = testing::LIB_PATH + testing::DIR_SEP + u8"くいりのまちとこしくそ" + testing::DIR_SEP + US_LIB_PREFIX + "TestBundleU" + US_LIB_EXT;
	auto bundles = bc.InstallBundles(path_utf8);
	US_TEST_CONDITION(bundles.size() == 1, "Install bundle from unicode path");
	auto bundle = bundles.at(0);
	std::string expectedValue = u8"电脑 くいりのまちとこしくそ";
	std::string actualValue = bundle.GetHeaders().at("unicode.sample").ToString();
	US_TEST_CONDITION(expectedValue == actualValue, "Check unicode data from manifest.json");
	bundle.Stop();
#endif
}

int BundleManifestTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("BundleManifestTest");

  FrameworkFactory factory;
  auto framework = factory.NewFramework();
  framework.Start();

  auto bundleM = testing::InstallLib(framework.GetBundleContext(), "TestBundleM");
  US_TEST_CONDITION_REQUIRED(bundleM, "Test for existing bundle TestBundleM")

  auto headers = bundleM.GetHeaders();

  US_TEST_CONDITION(headers.at(Constants::BUNDLE_SYMBOLICNAME).ToString() == "TestBundleM", "Bundle name")
  US_TEST_CONDITION(bundleM.GetSymbolicName() == "TestBundleM", "Bundle name 2")
  US_TEST_CONDITION(headers.at(Constants::BUNDLE_DESCRIPTION).ToString() == "My Bundle description", "Bundle description")
  US_TEST_CONDITION(headers.at(Constants::BUNDLE_VERSION).ToString() == "1.0.0", "Bundle version")
  US_TEST_CONDITION(bundleM.GetVersion() == BundleVersion(1,0,0), "Bundle version 2")

  Any anyVector = headers.at("vector");
  US_TEST_CONDITION_REQUIRED(anyVector.Type() == typeid(std::vector<Any>), "vector type")
  std::vector<Any>& vec = ref_any_cast<std::vector<Any> >(anyVector);
  US_TEST_CONDITION_REQUIRED(vec.size() == 3, "vector size")
  US_TEST_CONDITION_REQUIRED(vec[0].Type() == typeid(std::string), "vector 0 type")
  US_TEST_CONDITION_REQUIRED(vec[0].ToString() == "first", "vector 0 value")
  US_TEST_CONDITION_REQUIRED(vec[1].Type() == typeid(int), "vector 1 type")
  US_TEST_CONDITION_REQUIRED(any_cast<int>(vec[1]) == 2, "vector 1 value")

  Any anyMap = headers.at("map");
  US_TEST_CONDITION_REQUIRED(anyMap.Type() == typeid(AnyMap), "map type")
  AnyMap& m = ref_any_cast<AnyMap>(anyMap);
  US_TEST_CONDITION_REQUIRED(m.size() == 3, "map size")
  US_TEST_CONDITION_REQUIRED(m["string"].Type() == typeid(std::string), "map 0 type")
  US_TEST_CONDITION_REQUIRED(m["string"].ToString() == "hi", "map 0 value")
  US_TEST_CONDITION_REQUIRED(m["number"].Type() == typeid(int), "map 1 type")
  US_TEST_CONDITION_REQUIRED(any_cast<int>(m["number"]) == 4, "map 1 value")
  US_TEST_CONDITION_REQUIRED(m["list"].Type() == typeid(std::vector<Any>), "map 2 type")
  US_TEST_CONDITION_REQUIRED(any_cast<std::vector<Any> >(m["list"]).size() == 2, "map 2 value size")

  TestUnicodeProperty(framework.GetBundleContext());

  framework.Stop();
  framework.WaitForStop(std::chrono::milliseconds(0));

  US_TEST_END()
}
