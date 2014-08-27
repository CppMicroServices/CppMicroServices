/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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

#include <usModuleContext.h>
#include <usModuleEvent.h>
#include <usGetModuleContext.h>
#include <usModuleRegistry.h>
#include <usModule.h>
#include <usModuleSettings.h>
#include <usSharedLibrary.h>

#include <usTestingConfig.h>

#include "usTestUtilModuleListener.h"
#include "usTestingMacros.h"

#include <cassert>

US_USE_NAMESPACE

namespace {

#ifdef US_PLATFORM_WINDOWS
  static const std::string LIB_PATH = US_RUNTIME_OUTPUT_DIRECTORY;
#else
  static const std::string LIB_PATH = US_LIBRARY_OUTPUT_DIRECTORY;
#endif

void testDefaultAutoLoadPath(bool autoLoadEnabled)
{
  ModuleContext* mc = GetModuleContext();
  assert(mc);
  TestModuleListener listener;

  ModuleListenerRegistrationHelper<TestModuleListener> listenerReg(mc, &listener, &TestModuleListener::ModuleChanged);

  SharedLibrary libAL(LIB_PATH, "TestModuleAL");

  try
  {
    libAL.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

  Module* moduleAL = ModuleRegistry::GetModule("TestModuleAL");
  US_TEST_CONDITION_REQUIRED(moduleAL != NULL, "Test for existing module TestModuleAL")

  US_TEST_CONDITION(moduleAL->GetName() == "TestModuleAL", "Test module name")

  // check the listeners for events
  std::vector<ModuleEvent> pEvts;
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL));

  Any loadedModules = moduleAL->GetProperty(Module::PROP_AUTOLOADED_MODULES());
  Module* moduleAL_1 = ModuleRegistry::GetModule("TestModuleAL_1");
  if (autoLoadEnabled)
  {
    US_TEST_CONDITION_REQUIRED(moduleAL_1 != NULL, "Test for existing auto-loaded module TestModuleAL_1")
    US_TEST_CONDITION(moduleAL_1->GetName() == "TestModuleAL_1", "Test module name")
    US_TEST_CONDITION_REQUIRED(!loadedModules.Empty(), "Test for PROP_AUTOLOADED_MODULES property")
    US_TEST_CONDITION_REQUIRED(loadedModules.Type() == typeid(std::vector<std::string>), "Test for PROP_AUTOLOADED_MODULES property type")
    std::vector<std::string> loadedModulesVec = any_cast<std::vector<std::string> >(loadedModules);
    US_TEST_CONDITION_REQUIRED(loadedModulesVec.size() == 1, "Test for PROP_AUTOLOADED_MODULES vector size")
    US_TEST_CONDITION_REQUIRED(loadedModulesVec[0] == moduleAL_1->GetLocation(), "Test for PROP_AUTOLOADED_MODULES vector content")

    pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL_1));
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleAL_1));
  }
  else
  {
    US_TEST_CONDITION_REQUIRED(moduleAL_1 == NULL, "Test for non-existing auto-loaded module TestModuleAL_1")
    US_TEST_CONDITION_REQUIRED(loadedModules.Empty(), "Test for empty PROP_AUTOLOADED_MODULES property")
  }

  pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleAL));

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Test for unexpected events");

  mc->RemoveModuleListener(&listener, &TestModuleListener::ModuleChanged);

  libAL.Unload();
}

void testCustomAutoLoadPath()
{
  ModuleContext* mc = GetModuleContext();
  assert(mc);
  TestModuleListener listener;

  try
  {
    mc->AddModuleListener(&listener, &TestModuleListener::ModuleChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_OUTPUT( << "module listener registration failed " << ise.what() );
    throw;
  }

  SharedLibrary libAL2(LIB_PATH, "TestModuleAL2");

  try
  {
    libAL2.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

  Module* moduleAL2 = ModuleRegistry::GetModule("TestModuleAL2");
  US_TEST_CONDITION_REQUIRED(moduleAL2 != NULL, "Test for existing module TestModuleAL2")

  US_TEST_CONDITION(moduleAL2->GetName() == "TestModuleAL2", "Test module name")

  // check the listeners for events
  std::vector<ModuleEvent> pEvts;
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL2));

  Module* moduleAL2_1 = ModuleRegistry::GetModule("TestModuleAL2_1");
#ifdef US_ENABLE_AUTOLOADING_SUPPORT
  US_TEST_CONDITION_REQUIRED(moduleAL2_1 != NULL, "Test for existing auto-loaded module TestModuleAL2_1")
  US_TEST_CONDITION(moduleAL2_1->GetName() == "TestModuleAL2_1", "Test module name")

  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL2_1));
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleAL2_1));
#else
  US_TEST_CONDITION_REQUIRED(moduleAL2_1 == NULL, "Test for non-existing aut-loaded module TestModuleAL2_1")
#endif

  pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleAL2));

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Test for unexpected events");

  mc->RemoveModuleListener(&listener, &TestModuleListener::ModuleChanged);
}

} // end unnamed namespace


int usModuleAutoLoadTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ModuleLoaderTest");

  ModuleSettings::SetAutoLoadingEnabled(false);
  testDefaultAutoLoadPath(false);
  ModuleSettings::SetAutoLoadingEnabled(true);

  testDefaultAutoLoadPath(true);

  testCustomAutoLoadPath();

  US_TEST_END()
}
