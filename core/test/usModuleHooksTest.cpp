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
#include <usModule.h>
#include <usModuleEvent.h>
#include <usModuleFindHook.h>
#include <usModuleEventHook.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usSharedLibrary.h>

#include "usTestUtils.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

US_USE_NAMESPACE

namespace {

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
      contexts.clear();//erase(std::remove(contexts.begin(), contexts.end(), GetModuleContext()), contexts.end());
    }
  }
};

void TestFindHook(Framework* framework)
{
  InstallTestBundle(framework->GetModuleContext(), "TestModuleA");

  Module* moduleA = framework->GetModuleContext()->GetModule("TestModuleA");
  US_TEST_CONDITION_REQUIRED(moduleA != 0, "Test for existing module TestModuleA")

  US_TEST_CONDITION(moduleA->GetName() == "TestModuleA", "Test module name")

  moduleA->Start();

  US_TEST_CONDITION(moduleA->IsLoaded() == true, "Test if loaded correctly");

  long moduleAId = moduleA->GetModuleId();
  US_TEST_CONDITION_REQUIRED(moduleAId > 0, "Test for valid module id")

  US_TEST_CONDITION_REQUIRED(framework->GetModuleContext()->GetModule(moduleAId) != NULL, "Test for non-filtered GetModule(long) result")

  TestModuleFindHook findHook;
  ServiceRegistration<ModuleFindHook> findHookReg = framework->GetModuleContext()->RegisterService<ModuleFindHook>(&findHook);

  US_TEST_CONDITION_REQUIRED(framework->GetModuleContext()->GetModule(moduleAId) == NULL, "Test for filtered GetModule(long) result")

  std::vector<Module*> modules = framework->GetModuleContext()->GetModules();
  for (std::vector<Module*>::iterator i = modules.begin();
       i != modules.end(); ++i)
  {
    if((*i)->GetName() == "TestModuleA")
    {
      US_TEST_FAILED_MSG(<< "TestModuleA not filtered from GetModules()")
    }
  }

  findHookReg.Unregister();

  moduleA->Stop();
}

void TestEventHook(Framework* framework)
{
  TestModuleListener moduleListener;
  framework->GetModuleContext()->AddModuleListener(&moduleListener, &TestModuleListener::ModuleChanged);

  InstallTestBundle(framework->GetModuleContext(), "TestModuleA");

  Module* moduleA = framework->GetModuleContext()->GetModule("TestModuleA");
  moduleA->Start();
  US_TEST_CONDITION_REQUIRED(moduleListener.events.size() == 3, "Test for received load module events")

  moduleA->Stop();
  US_TEST_CONDITION_REQUIRED(moduleListener.events.size() == 5, "Test for received unload module events")

  TestModuleEventHook eventHook;
  ServiceRegistration<ModuleEventHook> eventHookReg = framework->GetModuleContext()->RegisterService<ModuleEventHook>(&eventHook);

  moduleListener.events.clear();
  
  moduleA->Start();
  US_TEST_CONDITION_REQUIRED(moduleListener.events.size() == 1, "Test for filtered load module events")
  US_TEST_CONDITION_REQUIRED(moduleListener.events[0].GetType() == ModuleEvent::LOADED, "Test for LOADED event")

  moduleA->Stop();
  US_TEST_CONDITION_REQUIRED(moduleListener.events.size() == 2, "Test for filtered unload module events")
  US_TEST_CONDITION_REQUIRED(moduleListener.events[1].GetType() == ModuleEvent::UNLOADED, "Test for UNLOADED event")

  eventHookReg.Unregister();
  framework->GetModuleContext()->RemoveModuleListener(&moduleListener, &TestModuleListener::ModuleChanged);
}

} // end unnamed namespace

int usModuleHooksTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ModuleHooksTest");

  FrameworkFactory factory;
  Framework* framework = factory.newFramework(std::map<std::string, std::string>());
  framework->init();
  framework->Start();

  TestFindHook(framework);
  TestEventHook(framework);

  delete framework;

  US_TEST_END()
}
