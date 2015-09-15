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
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleEvent.h>
#include <usServiceEvent.h>
#include <usModuleContext.h>
#include <usModuleActivator.h>

#include "usTestUtils.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

US_USE_NAMESPACE

namespace {

} // end unnamed namespace

int usModuleManifestTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ModuleManifestTest");

  FrameworkFactory factory;
  Framework* framework = factory.NewFramework(std::map<std::string, std::string>());
  framework->Start();

  InstallTestBundle(framework->GetModuleContext(), "TestModuleM");

  Module* moduleM = framework->GetModuleContext()->GetModule("TestModuleM");
  US_TEST_CONDITION_REQUIRED(moduleM != 0, "Test for existing module TestModuleM")

  US_TEST_CONDITION(moduleM->GetProperty(Module::PROP_NAME()).ToString() == "TestModuleM", "Module name")
  US_TEST_CONDITION(moduleM->GetName() == "TestModuleM", "Module name 2")
  US_TEST_CONDITION(moduleM->GetProperty(Module::PROP_DESCRIPTION()).ToString() == "My Module description", "Module description")
  US_TEST_CONDITION(moduleM->GetLocation() == moduleM->GetProperty(Module::PROP_LOCATION()).ToString(), "Module location")
  US_TEST_CONDITION(moduleM->GetProperty(Module::PROP_VERSION()).ToString() == "1.0.0", "Module version")
  US_TEST_CONDITION(moduleM->GetVersion() == ModuleVersion(1,0,0), "Module version 2")

  Any anyVector = moduleM->GetProperty("vector");
  US_TEST_CONDITION_REQUIRED(anyVector.Type() == typeid(std::vector<Any>), "vector type")
  std::vector<Any>& vec = ref_any_cast<std::vector<Any> >(anyVector);
  US_TEST_CONDITION_REQUIRED(vec.size() == 3, "vector size")
  US_TEST_CONDITION_REQUIRED(vec[0].Type() == typeid(std::string), "vector 0 type")
  US_TEST_CONDITION_REQUIRED(vec[0].ToString() == "first", "vector 0 value")
  US_TEST_CONDITION_REQUIRED(vec[1].Type() == typeid(int), "vector 1 type")
  US_TEST_CONDITION_REQUIRED(any_cast<int>(vec[1]) == 2, "vector 1 value")

  Any anyMap = moduleM->GetProperty("map");
  US_TEST_CONDITION_REQUIRED(anyMap.Type() == typeid(std::map<std::string,Any>), "map type")
  std::map<std::string, Any>& m = ref_any_cast<std::map<std::string, Any> >(anyMap);
  US_TEST_CONDITION_REQUIRED(m.size() == 3, "map size")
  US_TEST_CONDITION_REQUIRED(m["string"].Type() == typeid(std::string), "map 0 type")
  US_TEST_CONDITION_REQUIRED(m["string"].ToString() == "hi", "map 0 value")
  US_TEST_CONDITION_REQUIRED(m["number"].Type() == typeid(int), "map 1 type")
  US_TEST_CONDITION_REQUIRED(any_cast<int>(m["number"]) == 4, "map 1 value")
  US_TEST_CONDITION_REQUIRED(m["list"].Type() == typeid(std::vector<Any>), "map 2 type")
  US_TEST_CONDITION_REQUIRED(any_cast<std::vector<Any> >(m["list"]).size() == 2, "map 2 value size")

  delete framework;

  US_TEST_END()
}
