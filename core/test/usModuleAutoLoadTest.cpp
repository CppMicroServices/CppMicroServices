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
#include <usModuleContext.h>
#include <usModuleEvent.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleSettings.h>

#include <usTestingConfig.h>
#include "usTestUtils.h"
#include "usTestUtilModuleListener.h"
#include "usTestingMacros.h"

#include <cassert>

using namespace us;

namespace {

void testDefaultAutoLoadPath(bool autoLoadEnabled, Framework* framework)
{
  ModuleContext* mc = framework->GetModuleContext();
  assert(mc);
  TestModuleListener listener;

  ModuleListenerRegistrationHelper<TestModuleListener> listenerReg(mc, &listener, &TestModuleListener::ModuleChanged);

  InstallTestBundle(mc, "TestModuleAL");

  Module* moduleAL = mc->GetModule("TestModuleAL");
  US_TEST_CONDITION_REQUIRED(moduleAL != NULL, "Test for existing module TestModuleAL")

  US_TEST_CONDITION(moduleAL->GetName() == "TestModuleAL", "Test module name")

  moduleAL->Start();

  Any loadedModules = moduleAL->GetProperty(Module::PROP_AUTOLOADED_MODULES());
  Module* moduleAL_1 = mc->GetModule("TestModuleAL_1");

  // check the listeners for events
  std::vector<ModuleEvent> pEvts;
  if (autoLoadEnabled)
  {
      pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, moduleAL_1));
  }
  pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, moduleAL));
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL));
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleAL));

  if (autoLoadEnabled)
  {
    US_TEST_CONDITION_REQUIRED(moduleAL_1 != NULL, "Test for existing auto-loaded module TestModuleAL_1")
    US_TEST_CONDITION(moduleAL_1->GetName() == "TestModuleAL_1", "Test module name")
    US_TEST_CONDITION_REQUIRED(!loadedModules.Empty(), "Test for PROP_AUTOLOADED_MODULES property")
    US_TEST_CONDITION_REQUIRED(loadedModules.Type() == typeid(std::vector<std::string>), "Test for PROP_AUTOLOADED_MODULES property type")
    std::vector<std::string> loadedModulesVec = any_cast<std::vector<std::string> >(loadedModules);
    US_TEST_CONDITION_REQUIRED(loadedModulesVec.size() == 1, "Test for PROP_AUTOLOADED_MODULES vector size")
    US_TEST_CONDITION_REQUIRED(loadedModulesVec[0] == moduleAL_1->GetName(), "Test for PROP_AUTOLOADED_MODULES vector content")

    moduleAL_1->Start();

    pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL_1));
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleAL_1));

  }
  else
  {
    US_TEST_CONDITION_REQUIRED(moduleAL_1 == NULL, "Test for non-existing auto-loaded module TestModuleAL_1")
    US_TEST_CONDITION_REQUIRED(loadedModules.Empty(), "Test for empty PROP_AUTOLOADED_MODULES property")
  }

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Test for unexpected events");

  mc->RemoveModuleListener(&listener, &TestModuleListener::ModuleChanged);

  moduleAL->Stop();
}

void testCustomAutoLoadPath(Framework* framework)
{
  ModuleContext* mc = framework->GetModuleContext();
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

  InstallTestBundle(mc, "TestModuleAL2");

  Module* moduleAL2 = mc->GetModule("TestModuleAL2");
  US_TEST_CONDITION_REQUIRED(moduleAL2 != NULL, "Test for existing module TestModuleAL2")

  US_TEST_CONDITION(moduleAL2->GetName() == "TestModuleAL2", "Test module name")

  moduleAL2->Start();

  Module* moduleAL2_1 = mc->GetModule("TestModuleAL2_1");

  // check the listeners for events
  std::vector<ModuleEvent> pEvts;
#ifdef US_ENABLE_AUTOLOADING_SUPPORT
  pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, moduleAL2_1));
#endif
  pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, moduleAL2));
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL2));
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleAL2));

#ifdef US_ENABLE_AUTOLOADING_SUPPORT
  US_TEST_CONDITION_REQUIRED(moduleAL2_1 != NULL, "Test for existing auto-loaded module TestModuleAL2_1")
  US_TEST_CONDITION(moduleAL2_1->GetName() == "TestModuleAL2_1", "Test module name")

  moduleAL2_1->Start();
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL2_1));
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleAL2_1));

#else
  US_TEST_CONDITION_REQUIRED(moduleAL2_1 == NULL, "Test for non-existing aut-loaded module TestModuleAL2_1")
#endif

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Test for unexpected events");

  mc->RemoveModuleListener(&listener, &TestModuleListener::ModuleChanged);

  moduleAL2->Stop();
}

} // end unnamed namespace


int usModuleAutoLoadTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ModuleLoaderTest");

  FrameworkFactory factory;
  Framework* framework = factory.NewFramework(std::map<std::string, std::string>());
  framework->Start();

  framework->SetAutoLoadingEnabled(false);
  testDefaultAutoLoadPath(false, framework);
  
  framework->Stop();
  delete framework;

  framework = factory.NewFramework(std::map<std::string, std::string>());
  framework->Start();

  framework->SetAutoLoadingEnabled(true);

  testDefaultAutoLoadPath(true, framework);
  testCustomAutoLoadPath(framework);

  framework->Stop();
  delete framework;

  US_TEST_END()
}
