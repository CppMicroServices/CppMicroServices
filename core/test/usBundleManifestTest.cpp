/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

#include <usFrameworkFactory.h>
#include <usFramework.h>
#include <usGetBundleContext.h>
#include <usBundle.h>
#include <usBundleEvent.h>
#include <usServiceEvent.h>
#include <usBundleContext.h>
#include <usBundleActivator.h>

#include "usTestUtils.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

using namespace us;

namespace {

} // end unnamed namespace

int usBundleManifestTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("BundleManifestTest");

  FrameworkFactory factory;
  auto framework = factory.NewFramework();
  framework->Start();

  InstallTestBundle(framework->GetBundleContext(), "TestBundleM");

  auto bundleM = framework->GetBundleContext()->GetBundle("TestBundleM");
  US_TEST_CONDITION_REQUIRED(bundleM != nullptr, "Test for existing bundle TestBundleM")

  US_TEST_CONDITION(bundleM->GetProperty(Bundle::PROP_NAME).ToString() == "TestBundleM", "Bundle name")
  US_TEST_CONDITION(bundleM->GetName() == "TestBundleM", "Bundle name 2")
  US_TEST_CONDITION(bundleM->GetProperty(Bundle::PROP_DESCRIPTION).ToString() == "My Bundle description", "Bundle description")
  US_TEST_CONDITION(bundleM->GetLocation() == bundleM->GetProperty(Bundle::PROP_LOCATION).ToString(), "Bundle location")
  US_TEST_CONDITION(bundleM->GetProperty(Bundle::PROP_VERSION).ToString() == "1.0.0", "Bundle version")
  US_TEST_CONDITION(bundleM->GetVersion() == BundleVersion(1,0,0), "Bundle version 2")

  Any anyVector = bundleM->GetProperty("vector");
  US_TEST_CONDITION_REQUIRED(anyVector.Type() == typeid(std::vector<Any>), "vector type")
  std::vector<Any>& vec = ref_any_cast<std::vector<Any> >(anyVector);
  US_TEST_CONDITION_REQUIRED(vec.size() == 3, "vector size")
  US_TEST_CONDITION_REQUIRED(vec[0].Type() == typeid(std::string), "vector 0 type")
  US_TEST_CONDITION_REQUIRED(vec[0].ToString() == "first", "vector 0 value")
  US_TEST_CONDITION_REQUIRED(vec[1].Type() == typeid(int), "vector 1 type")
  US_TEST_CONDITION_REQUIRED(any_cast<int>(vec[1]) == 2, "vector 1 value")

  Any anyMap = bundleM->GetProperty("map");
  US_TEST_CONDITION_REQUIRED(anyMap.Type() == typeid(std::map<std::string,Any>), "map type")
  std::map<std::string, Any>& m = ref_any_cast<std::map<std::string, Any> >(anyMap);
  US_TEST_CONDITION_REQUIRED(m.size() == 3, "map size")
  US_TEST_CONDITION_REQUIRED(m["string"].Type() == typeid(std::string), "map 0 type")
  US_TEST_CONDITION_REQUIRED(m["string"].ToString() == "hi", "map 0 value")
  US_TEST_CONDITION_REQUIRED(m["number"].Type() == typeid(int), "map 1 type")
  US_TEST_CONDITION_REQUIRED(any_cast<int>(m["number"]) == 4, "map 1 value")
  US_TEST_CONDITION_REQUIRED(m["list"].Type() == typeid(std::vector<Any>), "map 2 type")
  US_TEST_CONDITION_REQUIRED(any_cast<std::vector<Any> >(m["list"]).size() == 2, "map 2 value size")

  US_TEST_END()
}
