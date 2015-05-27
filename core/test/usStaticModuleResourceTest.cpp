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

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModuleRegistry.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <usSharedLibrary.h>

#include "usTestingMacros.h"
#include "usTestingConfig.h"

#include <cassert>

#include <set>

US_USE_NAMESPACE

namespace {

std::string GetResourceContent(const ModuleResource& resource)
{
  std::string line;
  ModuleResourceStream rs(resource);
  std::getline(rs, line);
  return line;
}

struct ResourceComparator {
  bool operator()(const ModuleResource& mr1, const ModuleResource& mr2) const
  {
    return mr1 < mr2;
  }
};

void testResourceOperators(Module* module)
{
  std::vector<ModuleResource> resources = module->FindResources("", "res.txt", false);
  US_TEST_CONDITION_REQUIRED(resources.size() == 1, "Check resource count")
}

void testResourcesWithStaticImport(Framework* framework, Module* module)
{
  ModuleResource resource = module->GetResource("res.txt");
  US_TEST_CONDITION_REQUIRED(resource.IsValid(), "Check valid res.txt resource")
  std::string line = GetResourceContent(resource);
  US_TEST_CONDITION(line == "dynamic resource", "Check dynamic resource content")

  resource = module->GetResource("dynamic.txt");
  US_TEST_CONDITION_REQUIRED(resource.IsValid(), "Check valid dynamic.txt resource")
  line = GetResourceContent(resource);
  US_TEST_CONDITION(line == "dynamic", "Check dynamic resource content")

  resource = module->GetResource("static.txt");
  US_TEST_CONDITION_REQUIRED(!resource.IsValid(), "Check in-valid static.txt resource")

  Module* importedByBModule = framework->GetModuleContext()->GetModule("TestModuleImportedByB");
  US_TEST_CONDITION_REQUIRED(importedByBModule != NULL, "Check valid static module")
  resource = importedByBModule->GetResource("static.txt");
  US_TEST_CONDITION_REQUIRED(resource.IsValid(), "Check valid static.txt resource")
  line = GetResourceContent(resource);
  US_TEST_CONDITION(line == "static", "Check static resource content")

  std::vector<ModuleResource> resources = module->FindResources("", "*.txt", false);
  std::stable_sort(resources.begin(), resources.end(), ResourceComparator());
  std::vector<ModuleResource> importedResources = importedByBModule->FindResources("", "*.txt", false);
  std::stable_sort(importedResources.begin(), importedResources.end(), ResourceComparator());

  US_TEST_CONDITION(resources.size() == 2, "Check resource count")
  US_TEST_CONDITION(importedResources.size() == 2, "Check resource count")
  line = GetResourceContent(resources[0]);
  US_TEST_CONDITION(line == "dynamic", "Check dynamic.txt resource content")
  line = GetResourceContent(resources[1]);
  US_TEST_CONDITION(line == "dynamic resource", "Check res.txt (from importing module) resource content")
  line = GetResourceContent(importedResources[0]);
  US_TEST_CONDITION(line == "static resource", "Check res.txt (from imported module) resource content")
  line = GetResourceContent(importedResources[1]);
  US_TEST_CONDITION(line == "static", "Check static.txt (from importing module) resource content")
}

} // end unnamed namespace


int usStaticModuleResourceTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("StaticModuleResourceTest");

  FrameworkFactory factory;
  Framework* framework = factory.newFramework(std::map<std::string, std::string>());
  framework->init();
  framework->Start();

  assert(framework->GetModuleContext());


#ifdef US_PLATFORM_WINDOWS
  static const std::string LIB_PATH = US_RUNTIME_OUTPUT_DIRECTORY;
  static const std::string DIR_SEP = "\\";
  static const std::string LIB_PREFIX = "";
  static const std::string LIB_EXT = ".dll";
#else
#if defined US_PLATFORM_APPLE
  static const std::string LIB_EXT = ".dylib";
#else
  static const std::string LIB_EXT = ".so";
#endif
  static const std::string LIB_PATH = US_LIBRARY_OUTPUT_DIRECTORY;
  static const std::string LIB_PREFIX = "lib";
  static const std::string DIR_SEP = "/";
#endif

   try
  {
    Module* module = framework->GetModuleContext()->InstallBundle(LIB_PATH + DIR_SEP + "TestModuleB.dll/TestModuleB");
    US_TEST_CONDITION_REQUIRED(module != NULL, "Test installation of module TestModuleB")
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Install bundle exception: " << e.what())
  }

  try
  {
    Module* module = framework->GetModuleContext()->InstallBundle(LIB_PATH + DIR_SEP + "TestModuleB.dll/TestModuleImportedByB");
    US_TEST_CONDITION_REQUIRED(module != NULL, "Test installation of module TestModuleImportedByB")
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Install bundle exception: " << e.what())
  }

  Module* module = framework->GetModuleContext()->GetModule("TestModuleB");
  US_TEST_CONDITION_REQUIRED(module != NULL, "Test for existing module TestModuleB")
  US_TEST_CONDITION(module->GetName() == "TestModuleB", "Test module name")

  testResourceOperators(module);
  testResourcesWithStaticImport(framework, module);

  ModuleResource resource = framework->GetModuleContext()->GetModule("TestModuleImportedByB")->GetResource("static.txt");
  module->Start();
  US_TEST_CONDITION_REQUIRED(resource.IsValid(), "Check valid static.txt resource")

  module->Stop();
  US_TEST_CONDITION_REQUIRED(resource.IsValid() == true, "Check still valid static.txt resource")

  delete framework;

  US_TEST_END()
}
