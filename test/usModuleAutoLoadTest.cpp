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

#include <usTestingConfig.h>

#include "usTestUtilSharedLibrary.h"
#include "usTestUtilModuleListener.h"
#include "usTestingMacros.h"

#include <assert.h>

US_USE_NAMESPACE

namespace {

void testDefaultAutoLoadPath(bool autoLoadEnabled)
{
  ModuleContext* mc = GetModuleContext();
  assert(mc);
  TestModuleListener listener(mc);

  try
  {
    mc->AddModuleListener(&listener, &TestModuleListener::ModuleChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_OUTPUT( << "module listener registration failed " << ise.what() );
    throw;
  }

  SharedLibraryHandle libAL("TestModuleAL");

  try
  {
    libAL.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

  Module* moduleAL = ModuleRegistry::GetModule("TestModuleAL Module");
  US_TEST_CONDITION_REQUIRED(moduleAL != NULL, "Test for existing module TestModuleAL")

  US_TEST_CONDITION(moduleAL->GetName() == "TestModuleAL Module", "Test module name")

  // check the listeners for events
  std::vector<ModuleEvent> pEvts;
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL));

  Module* moduleAL_1 = ModuleRegistry::GetModule("TestModuleAL_1 Module");
  if (autoLoadEnabled)
  {
    US_TEST_CONDITION_REQUIRED(moduleAL_1 != NULL, "Test for existing auto-loaded module TestModuleAL_1")
    US_TEST_CONDITION(moduleAL_1->GetName() == "TestModuleAL_1 Module", "Test module name")

    pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL_1));
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleAL_1));
  }
  else
  {
    US_TEST_CONDITION_REQUIRED(moduleAL_1 == NULL, "Test for non-existing auto-loaded module TestModuleAL_1")
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
  TestModuleListener listener(mc);

  try
  {
    mc->AddModuleListener(&listener, &TestModuleListener::ModuleChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_OUTPUT( << "module listener registration failed " << ise.what() );
    throw;
  }

  SharedLibraryHandle libAL2("TestModuleAL2");

  try
  {
    libAL2.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

  Module* moduleAL2 = ModuleRegistry::GetModule("TestModuleAL2 Module");
  US_TEST_CONDITION_REQUIRED(moduleAL2 != NULL, "Test for existing module TestModuleAL2")

  US_TEST_CONDITION(moduleAL2->GetName() == "TestModuleAL2 Module", "Test module name")

  // check the listeners for events
  std::vector<ModuleEvent> pEvts;
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL2));

  Module* moduleAL2_1 = ModuleRegistry::GetModule("TestModuleAL2_1 Module");
#ifdef US_ENABLE_AUTOLOADING_SUPPORT
  US_TEST_CONDITION_REQUIRED(moduleAL2_1 != NULL, "Test for existing auto-loaded module TestModuleAL2_1")
  US_TEST_CONDITION(moduleAL2_1->GetName() == "TestModuleAL2_1 Module", "Test module name")

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
