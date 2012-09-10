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

#include <usTestingConfig.h>

#include "usTestUtilSharedLibrary.cpp"
#include "usTestingMacros.h"

#include <assert.h>

US_USE_NAMESPACE

extern ModuleActivator* _us_module_activator_instance_TestModuleAL();
extern ModuleActivator* _us_module_activator_instance_TestModuleAL2();

namespace {

class TestModuleAutoLoadListener {

public:

  TestModuleAutoLoadListener(ModuleContext* mc) : mc(mc), moduleEvents()
  {}

  void ModuleChanged(const ModuleEvent event)
  {
    moduleEvents.push_back(event);
    US_DEBUG << "ModuleEvent:" << event;
  }

  ModuleEvent GetModuleEvent() const
  {
    if (moduleEvents.empty())
    {
      return ModuleEvent();
    }
    return moduleEvents.back();
  }

  bool CheckListenerEvents(
      bool pexp, ModuleEvent::Type ptype,
      Module* moduleX)
  {
    std::vector<ModuleEvent> pEvts;

    if (pexp) pEvts.push_back(ModuleEvent(ptype, moduleX));

    return CheckListenerEvents(pEvts);
  }

  bool CheckListenerEvents(
      const std::vector<ModuleEvent>& pEvts)
  {
    bool listenState = true; // assume everything will work

    if (pEvts.size() != moduleEvents.size())
    {
      listenState = false;
      US_DEBUG << "*** Module event mismatch: expected "
          << pEvts.size() << " event(s), found "
          << moduleEvents.size() << " event(s).";

      const std::size_t max = pEvts.size() > moduleEvents.size() ? pEvts.size() : moduleEvents.size();
      for (std::size_t i = 0; i < max; ++i)
      {
        const ModuleEvent& pE = i < pEvts.size() ? pEvts[i] : ModuleEvent();
        const ModuleEvent& pR = i < moduleEvents.size() ? moduleEvents[i] : ModuleEvent();
        US_DEBUG << "    " << pE << " - " << pR;
      }
    }
    else
    {
      for (std::size_t i = 0; i < pEvts.size(); ++i)
      {
        const ModuleEvent& pE = pEvts[i];
        const ModuleEvent& pR = moduleEvents[i];
        if (pE.GetType() != pR.GetType()
          || pE.GetModule() != pR.GetModule())
        {
          listenState = false;
          US_DEBUG << "*** Wrong module event: " << pR << " expected " << pE;
        }
      }
    }

    moduleEvents.clear();
    return listenState;
  }

private:

  ModuleContext* const mc;

  std::vector<ModuleEvent> moduleEvents;
};

void testDefaultAutLoadPath()
{
  ModuleContext* mc = GetModuleContext();
  assert(mc);
  TestModuleAutoLoadListener listener(mc);

  try
  {
    mc->AddModuleListener(&listener, &TestModuleAutoLoadListener::ModuleChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_OUTPUT( << "module listener registration failed " << ise.what() );
    throw;
  }

  SharedLibraryHandle libAL("TestModuleAL"
                           #ifndef US_BUILD_SHARED_LIBS
                           , _us_module_activator_instance_TestModuleAL
                           #endif
                           );

  try
  {
    libAL.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

#ifdef US_BUILD_SHARED_LIBS
  Module* moduleAL = ModuleRegistry::GetModule("TestModuleAL Module");
  US_TEST_CONDITION_REQUIRED(moduleAL != 0, "Test for existing module TestModuleAL")

  US_TEST_CONDITION(moduleAL->GetName() == "TestModuleAL Module", "Test module name")
#endif

  // check the listeners for events
  std::vector<ModuleEvent> pEvts;
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL));

  Module* moduleAL_1 = ModuleRegistry::GetModule("TestModuleAL_1 Module");
#ifdef US_ENABLE_AUTOLOADING_SUPPORT
  US_TEST_CONDITION_REQUIRED(moduleAL_1 != 0, "Test for existing auto-loaded module TestModuleAL_1")
  US_TEST_CONDITION(moduleAL_1->GetName() == "TestModuleAL_1 Module", "Test module name")

  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL_1));
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleAL_1));
#else
  US_TEST_CONDITION_REQUIRED(moduleAL_1 == NULL, "Test for non-existing aut-loaded module TestModuleAL_1")
#endif

  pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleAL));

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Test for unexpected events");

  mc->RemoveModuleListener(&listener, &TestModuleAutoLoadListener::ModuleChanged);
}

void testCustomAutoLoadPath()
{
  ModuleContext* mc = GetModuleContext();
  assert(mc);
  TestModuleAutoLoadListener listener(mc);

  try
  {
    mc->AddModuleListener(&listener, &TestModuleAutoLoadListener::ModuleChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_OUTPUT( << "module listener registration failed " << ise.what() );
    throw;
  }

  SharedLibraryHandle libAL2("TestModuleAL2"
                             #ifndef US_BUILD_SHARED_LIBS
                             , _us_module_activator_instance_TestModuleAL2
                             #endif
                             );

  try
  {
    libAL2.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

#ifdef US_BUILD_SHARED_LIBS
  Module* moduleAL2 = ModuleRegistry::GetModule("TestModuleAL2 Module");
  US_TEST_CONDITION_REQUIRED(moduleAL2 != 0, "Test for existing module TestModuleAL2")

  US_TEST_CONDITION(moduleAL2->GetName() == "TestModuleAL2 Module", "Test module name")
#endif

  // check the listeners for events
  std::vector<ModuleEvent> pEvts;
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL2));

  Module* moduleAL2_1 = ModuleRegistry::GetModule("TestModuleAL2_1 Module");
#ifdef US_ENABLE_AUTOLOADING_SUPPORT
  US_TEST_CONDITION_REQUIRED(moduleAL2_1 != 0, "Test for existing auto-loaded module TestModuleAL2_1")
  US_TEST_CONDITION(moduleAL2_1->GetName() == "TestModuleAL2_1 Module", "Test module name")

  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleAL2_1));
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleAL2_1));
#else
  US_TEST_CONDITION_REQUIRED(moduleAL2_1 == NULL, "Test for non-existing aut-loaded module TestModuleAL2_1")
#endif

  pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleAL2));

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Test for unexpected events");

  mc->RemoveModuleListener(&listener, &TestModuleAutoLoadListener::ModuleChanged);
}

} // end unnamed namespace


int usModuleAutoLoadTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ModuleLoaderTest");

  testDefaultAutLoadPath();

  testCustomAutoLoadPath();

  US_TEST_END()
}
