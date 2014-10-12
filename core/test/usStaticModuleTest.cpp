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
#include <usServiceEvent.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModuleRegistry.h>
#include <usModuleActivator.h>
#include <usSharedLibrary.h>

#include "usTestUtilModuleListener.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

US_USE_NAMESPACE

namespace {

#ifdef US_PLATFORM_WINDOWS
  static const std::string LIB_PATH = US_RUNTIME_OUTPUT_DIRECTORY;
#else
  static const std::string LIB_PATH = US_LIBRARY_OUTPUT_DIRECTORY;
#endif

// Load libTestModuleB and check that it exists and that the service it registers exists,
// also check that the expected events occur
void frame020a(ModuleContext* mc, TestModuleListener& listener,
#ifdef US_BUILD_SHARED_LIBS
               SharedLibrary& libB)
{
  try
  {
    libB.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }
#else
               SharedLibrary& /*libB*/)
{
#endif

  Module* moduleB = ModuleRegistry::GetModule("TestModuleB");
  US_TEST_CONDITION_REQUIRED(moduleB != 0, "Test for existing module TestModuleB")

  Module* moduleImportedByB = ModuleRegistry::GetModule("TestModuleImportedByB");
  US_TEST_CONDITION_REQUIRED(moduleImportedByB != 0, "Test for existing module TestModuleImportedByB")

  US_TEST_CONDITION(moduleB->GetName() == "TestModuleB", "Test module name")

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
#ifdef US_BUILD_SHARED_LIBS
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleB));
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleB));
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleImportedByB));
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleImportedByB));
#endif

    std::vector<ServiceEvent> seEvts;
#ifdef US_BUILD_SHARED_LIBS
    seEvts.push_back(ServiceEvent(ServiceEvent::REGISTERED, refs.back()));
    seEvts.push_back(ServiceEvent(ServiceEvent::REGISTERED, refs.front()));
#endif

    US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");

  }
  catch (const ServiceException& /*se*/)
  {
    US_TEST_FAILED_MSG(<< "test module, expected service not found");
  }

#ifdef US_BUILD_SHARED_LIBS
  US_TEST_CONDITION(moduleB->IsLoaded() == true, "Test if loaded correctly");
#endif
}


// Unload libB and check for correct events
void frame030b(ModuleContext* mc, TestModuleListener& listener, SharedLibrary& libB)
{
  Module* moduleB = ModuleRegistry::GetModule("TestModuleB");
  US_TEST_CONDITION_REQUIRED(moduleB != 0, "Test for non-null module")

  Module* moduleImportedByB = ModuleRegistry::GetModule("TestModuleImportedByB");
  US_TEST_CONDITION_REQUIRED(moduleImportedByB != 0, "Test for non-null module")

  std::vector<ServiceReferenceU> refs
      = mc->GetServiceReferences("us::TestModuleBService");
  US_TEST_CONDITION(refs.front(), "Test for first valid service reference")
  US_TEST_CONDITION(refs.back(), "Test for second valid service reference")

  try
  {
    libB.Unload();
#ifdef US_BUILD_SHARED_LIBS
    US_TEST_CONDITION(moduleB->IsLoaded() == false, "Test for unloaded state")
#endif
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "UnLoad module exception: " << e.what())
  }

  std::vector<ModuleEvent> pEvts;
#ifdef US_BUILD_SHARED_LIBS
  pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADING, moduleImportedByB));
  pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADED, moduleImportedByB));
  pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADING, moduleB));
  pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADED, moduleB));
#endif

  std::vector<ServiceEvent> seEvts;
#ifdef US_BUILD_SHARED_LIBS
  seEvts.push_back(ServiceEvent(ServiceEvent::UNREGISTERING, refs.front()));
  seEvts.push_back(ServiceEvent(ServiceEvent::UNREGISTERING, refs.back()));
#endif

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");
}

} // end unnamed namespace

int usStaticModuleTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("StaticModuleTest");

  ModuleContext* mc = GetModuleContext();
  TestModuleListener listener;

  ModuleListenerRegistrationHelper<TestModuleListener> ml(mc, &listener, &TestModuleListener::ModuleChanged);
  ServiceListenerRegistrationHelper<TestModuleListener> sl(mc, &listener, &TestModuleListener::ServiceChanged);

  SharedLibrary libB(LIB_PATH, "TestModuleB");
  frame020a(mc, listener, libB);
  frame030b(mc, listener, libB);

  US_TEST_END()
}
