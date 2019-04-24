﻿/*=============================================================================

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

using namespace cppmicroservices;

TEST(BundleManifestTest, UnicodeProperty)
{
  // 1. building static libraries (test bundle is included in the executable)
  // 2. using MINGW evironment (MinGW linker fails to link DLL with unicode path)
  // 3. using a compiler with no support for C++11 unicode string literals
#if !defined(US_BUILD_SHARED_LIBS) || defined(__MINGW32__) ||                  \
  !defined(US_CXX_UNICODE_LITERALS)
  SUCCEED() << "Skipping test point for unicode path";
#else
  auto framework = FrameworkFactory().NewFramework();
  framework.Start();
  std::string path_utf8 = testing::LIB_PATH + cppmicroservices::util::DIR_SEP +
                          u8"くいりのまちとこしくそ" +
                          cppmicroservices::util::DIR_SEP + US_LIB_PREFIX +
                          "TestBundleU" + US_LIB_EXT;
  auto bundles = bc.InstallBundles(path_utf8);
  ASSERT_EQ(bundles.size(), 1) << "Failed to install bundle using a unicode path";
  auto bundle = bundles.at(0);
  std::string expectedValue = u8"电脑 くいりのまちとこしくそ";
  std::string actualValue = bundle.GetHeaders().at("unicode.sample").ToString();
  ASSERT_STREQ(expectedValue,actualValue) <<
                    "Unicode data from manifest.json doesn't match expected value.";
  framework.Stop();
  framework.WaitForStop(std::chrono::milliseconds::zero());
#endif
}

TEST(BundleManifestTest, InstallBundleWithDeepManifest)
{
  auto framework = FrameworkFactory().NewFramework();
  framework.Start();
  auto bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleWithDeepManifest");
  auto headers = bundle.GetHeaders();
  ASSERT_THAT(headers.at(Constants::BUNDLE_SYMBOLICNAME).ToString(),
                      ::testing::StrEq("TestBundleWithDeepManifest")) << "Bundle symblic name doesn't match.";
  framework.Stop();
  framework.WaitForStop(std::chrono::milliseconds::zero());
}

TEST(BundleManifestTest, ParseManifest)
{
  auto framework = FrameworkFactory().NewFramework();
  framework.Start();

  auto bundleM =
    cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleM");
  ASSERT_TRUE(bundleM) << "Failed to install TestBundleM";

  auto headers = bundleM.GetHeaders();

  EXPECT_THAT(headers.at(Constants::BUNDLE_SYMBOLICNAME).ToString(),
               ::testing::StrEq("TestBundleM"));
  EXPECT_THAT(bundleM.GetSymbolicName(), ::testing::StrEq("TestBundleM"));
  EXPECT_THAT(headers.at(Constants::BUNDLE_DESCRIPTION).ToString(),
               ::testing::StrEq("My Bundle description"));
  EXPECT_THAT(headers.at(Constants::BUNDLE_VERSION).ToString(), ::testing::StrEq("1.0.0"));
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

  framework.Stop();
  framework.WaitForStop(std::chrono::milliseconds::zero());
}
