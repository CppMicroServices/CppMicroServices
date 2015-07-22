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
#include <usServiceEvent.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModuleRegistry.h>
#include <usModuleActivator.h>

#include "usTestUtilModuleListener.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

US_USE_NAMESPACE

namespace {

// Load libTestModuleB and check that it exists and that the service it registers exists,
// also check that the expected events occur
void frame020a(ModuleContext* mc, TestModuleListener& listener)
{
  try
  {
#if defined (US_BUILD_SHARED_LIBS)
    Module* module = mc->InstallBundle(LIB_PATH + DIR_SEP + LIB_PREFIX + "TestModuleB" + LIB_EXT + "/TestModuleB");
#else
    Module* module = mc->InstallBundle(BIN_PATH + DIR_SEP + "usCoreTestDriver" + EXE_EXT + "/TestModuleB");
#endif
    US_TEST_CONDITION_REQUIRED(module != NULL, "Test installation of module TestModuleB")
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Install bundle exception: " << e.what() << " + in frame020a:FAIL")
  }

  Module* moduleB = mc->GetModule("TestModuleB");
  US_TEST_CONDITION_REQUIRED(moduleB != 0, "Test for existing module TestModuleB")

  try
  {
#if defined (US_BUILD_SHARED_LIBS)
    Module* module = mc->InstallBundle(LIB_PATH + DIR_SEP + LIB_PREFIX + "TestModuleB" + LIB_EXT + "/TestModuleImportedByB");
#else
    Module* module = mc->InstallBundle(BIN_PATH + DIR_SEP + "usCoreTestDriver" + EXE_EXT + "/TestModuleImportedByB");
#endif
    US_TEST_CONDITION_REQUIRED(module != NULL, "Test installation of module TestModuleImportedByB")
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Install bundle exception: " << e.what() << " + in frame020a:FAIL")
  }

  Module* moduleImportedByB = mc->GetModule("TestModuleImportedByB");
  US_TEST_CONDITION_REQUIRED(moduleImportedByB != 0, "Test for existing module TestModuleImportedByB")

  US_TEST_CONDITION(moduleB->GetName() == "TestModuleB", "Test module name")
  US_TEST_CONDITION(moduleImportedByB->GetName() == "TestModuleImportedByB", "Test module name")

  moduleB->Start();
  moduleImportedByB->Start();
  // Check if libB registered the expected service
  try
  {
    std::vector<ServiceReferenceU> refs = mc->GetServiceReferences("us::TestModuleBService");
    US_TEST_CONDITION_REQUIRED(refs.size() == 2, "Test that both the service from the shared and imported library are regsitered");

    InterfaceMap o1 = mc->GetService(refs.front());
    US_TEST_CONDITION(!o1.empty(), "Test if first service object found");

    InterfaceMap o2 = mc->GetService(refs.back());
    US_TEST_CONDITION(!o2.empty(), "Test if second service object found");

    try
    {
      US_TEST_CONDITION(mc->UngetService(refs.front()), "Test if Service UnGet for first service returns true");
      US_TEST_CONDITION(mc->UngetService(refs.back()), "Test if Service UnGet for second service returns true");
    }
    catch (const std::logic_error le)
    {
      US_TEST_FAILED_MSG(<< "UnGetService exception: " << le.what())
    }

    // check the listeners for events
    std::vector<ModuleEvent> pEvts;
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleB));
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleB));
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleImportedByB));
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleImportedByB));

    std::vector<ServiceEvent> seEvts;
    seEvts.push_back(ServiceEvent(ServiceEvent::REGISTERED, refs.back()));
    seEvts.push_back(ServiceEvent(ServiceEvent::REGISTERED, refs.front()));

    US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");

  }
  catch (const ServiceException& /*se*/)
  {
    US_TEST_FAILED_MSG(<< "test module, expected service not found");
  }

  US_TEST_CONDITION(moduleB->IsLoaded() == true, "Test if loaded correctly");
}


// Unload libB and check for correct events
void frame030b(ModuleContext* mc, TestModuleListener& listener)
{
  Module* moduleB = mc->GetModule("TestModuleB");
  US_TEST_CONDITION_REQUIRED(moduleB != 0, "Test for non-null module")

  Module* moduleImportedByB = mc->GetModule("TestModuleImportedByB");
  US_TEST_CONDITION_REQUIRED(moduleImportedByB != 0, "Test for non-null module")

  std::vector<ServiceReferenceU> refs
      = mc->GetServiceReferences("us::TestModuleBService");
  US_TEST_CONDITION(refs.front(), "Test for first valid service reference")
  US_TEST_CONDITION(refs.back(), "Test for second valid service reference")

  try
  {
    moduleB->Stop();
    US_TEST_CONDITION(moduleB->IsLoaded() == false, "Test for unloaded state")
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Stop module exception: " << e.what())
  }

  try
  {
    moduleImportedByB->Stop();
    US_TEST_CONDITION(moduleImportedByB->IsLoaded() == false, "Test for unloaded state")
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Stop module exception: " << e.what())
  }

  std::vector<ModuleEvent> pEvts;
  pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADING, moduleB));
  pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADED, moduleB));
  pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADING, moduleImportedByB));
  pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADED, moduleImportedByB));

  std::vector<ServiceEvent> seEvts;
  seEvts.push_back(ServiceEvent(ServiceEvent::UNREGISTERING, refs.back()));
  seEvts.push_back(ServiceEvent(ServiceEvent::UNREGISTERING, refs.front()));

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");
}

} // end unnamed namespace

int usStaticModuleTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("StaticModuleTest");

  FrameworkFactory factory;
  Framework* framework = factory.newFramework(std::map<std::string, std::string>());
  framework->init();
  framework->Start();

  ModuleContext* mc = framework->GetModuleContext();

  { // scope the use of the listener so its destructor is
    // called before we destroy the framework's bundle context.
    // The TestModuleListener needs to remove its listeners while
    // the framework is still active.
    TestModuleListener listener;

    ModuleListenerRegistrationHelper<TestModuleListener> ml(mc, &listener, &TestModuleListener::ModuleChanged);
    ServiceListenerRegistrationHelper<TestModuleListener> sl(mc, &listener, &TestModuleListener::ServiceChanged);

    frame020a(mc, listener);
    frame030b(mc, listener);
  }

  delete framework;

  US_TEST_END()
}
