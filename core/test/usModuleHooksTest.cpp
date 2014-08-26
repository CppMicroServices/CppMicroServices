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

#include <usModule.h>
#include <usModuleEvent.h>
#include <usModuleFindHook.h>
#include <usModuleEventHook.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usSharedLibrary.h>

#include "usTestingMacros.h"
#include "usTestingConfig.h"

US_USE_NAMESPACE

namespace {

#ifdef US_PLATFORM_WINDOWS
  static const std::string LIB_PATH = US_RUNTIME_OUTPUT_DIRECTORY;
#else
  static const std::string LIB_PATH = US_LIBRARY_OUTPUT_DIRECTORY;
#endif

class TestModuleListener
{
public:

  void ModuleChanged(const ModuleEvent moduleEvent)
  {
    this->events.push_back(moduleEvent);
  }

  std::vector<ModuleEvent> events;
};

class TestModuleFindHook : public ModuleFindHook
{
public:

  void Find(const ModuleContext* /*context*/, ShrinkableVector<Module*>& modules)
  {
    for (ShrinkableVector<Module*>::iterator i = modules.begin();
         i != modules.end();)
    {
      if ((*i)->GetName() == "TestModuleA")
      {
        i = modules.erase(i);
      }
      else
      {
        ++i;
      }
    }
  }
};

class TestModuleEventHook : public ModuleEventHook
{
public:

  void Event(const ModuleEvent& event, ShrinkableVector<ModuleContext*>& contexts)
  {
    if (event.GetType() == ModuleEvent::LOADING || event.GetType() == ModuleEvent::UNLOADING)
    {
      contexts.erase(std::remove(contexts.begin(), contexts.end(), GetModuleContext()), contexts.end());
    }
  }
};

void TestFindHook()
{
  SharedLibrary libA(LIB_PATH, "TestModuleA");

#ifdef US_BUILD_SHARED_LIBS
  try
  {
    libA.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }
#endif

  Module* moduleA = GetModuleContext()->GetModule("TestModuleA");
  US_TEST_CONDITION_REQUIRED(moduleA != 0, "Test for existing module TestModuleA")

  US_TEST_CONDITION(moduleA->GetName() == "TestModuleA", "Test module name")

  US_TEST_CONDITION(moduleA->IsLoaded() == true, "Test if loaded correctly");

  long moduleAId = moduleA->GetModuleId();
  US_TEST_CONDITION_REQUIRED(moduleAId > 0, "Test for valid module id")

  US_TEST_CONDITION_REQUIRED(GetModuleContext()->GetModule(moduleAId) != NULL, "Test for non-filtered GetModule(long) result")

  TestModuleFindHook findHook;
  ServiceRegistration<ModuleFindHook> findHookReg = GetModuleContext()->RegisterService<ModuleFindHook>(&findHook);

  US_TEST_CONDITION_REQUIRED(GetModuleContext()->GetModule(moduleAId) == NULL, "Test for filtered GetModule(long) result")

  std::vector<Module*> modules = GetModuleContext()->GetModules();
  for (std::vector<Module*>::iterator i = modules.begin();
       i != modules.end(); ++i)
  {
    if((*i)->GetName() == "TestModuleA")
    {
      US_TEST_FAILED_MSG(<< "TestModuleA not filtered from GetModules()")
    }
  }

  findHookReg.Unregister();

  libA.Unload();
}

#ifdef US_BUILD_SHARED_LIBS
void TestEventHook()
{
  TestModuleListener moduleListener;
  GetModuleContext()->AddModuleListener(&moduleListener, &TestModuleListener::ModuleChanged);

  SharedLibrary libA(LIB_PATH, "TestModuleA");
  try
  {
    libA.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }
  US_TEST_CONDITION_REQUIRED(moduleListener.events.size() == 2, "Test for received load module events")

  libA.Unload();
  US_TEST_CONDITION_REQUIRED(moduleListener.events.size() == 4, "Test for received unload module events")

  TestModuleEventHook eventHook;
  ServiceRegistration<ModuleEventHook> eventHookReg = GetModuleContext()->RegisterService<ModuleEventHook>(&eventHook);

  moduleListener.events.clear();
  try
  {
    libA.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

  US_TEST_CONDITION_REQUIRED(moduleListener.events.size() == 1, "Test for filtered load module events")
  US_TEST_CONDITION_REQUIRED(moduleListener.events[0].GetType() == ModuleEvent::LOADED, "Test for LOADED event")

  libA.Unload();
  US_TEST_CONDITION_REQUIRED(moduleListener.events.size() == 2, "Test for filtered unload module events")
  US_TEST_CONDITION_REQUIRED(moduleListener.events[1].GetType() == ModuleEvent::UNLOADED, "Test for UNLOADED event")

  eventHookReg.Unregister();
  GetModuleContext()->RemoveModuleListener(&moduleListener, &TestModuleListener::ModuleChanged);
}
#endif

} // end unnamed namespace

int usModuleHooksTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ModuleHooksTest");

  TestFindHook();

#ifdef US_BUILD_SHARED_LIBS
  TestEventHook();
#endif

  US_TEST_END()
}
