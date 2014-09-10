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
#include <usModuleSettings.h>
#include <usSharedLibrary.h>

#include "usTestUtilModuleListener.h"
#include "usTestDriverActivator.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

US_USE_NAMESPACE

namespace {

#ifdef US_PLATFORM_WINDOWS
  static const std::string LIB_PATH = US_RUNTIME_OUTPUT_DIRECTORY;
  static const char PATH_SEPARATOR = '\\';
#else
  static const std::string LIB_PATH = US_LIBRARY_OUTPUT_DIRECTORY;
  static const char PATH_SEPARATOR = '/';
#endif

// Check that the executable's activator was loaded and called
void frame01()
{
  US_TEST_CONDITION_REQUIRED(TestDriverActivator::LoadCalled(), "ModuleActivator::Load() called for executable")
}

// Verify that the same member function pointers registered as listeners
// with different receivers works.
void frame02a()
{
  ModuleContext* mc = GetModuleContext();

  TestModuleListener listener1;
  TestModuleListener listener2;

  try
  {
    mc->RemoveModuleListener(&listener1, &TestModuleListener::ModuleChanged);
    mc->AddModuleListener(&listener1, &TestModuleListener::ModuleChanged);
    mc->RemoveModuleListener(&listener2, &TestModuleListener::ModuleChanged);
    mc->AddModuleListener(&listener2, &TestModuleListener::ModuleChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_FAILED_MSG( << "module listener registration failed " << ise.what()
                        << " : frameSL02a:FAIL" );
  }

  SharedLibrary target(LIB_PATH, "TestModuleA");

#ifdef US_BUILD_SHARED_LIBS
  // Start the test target
  try
  {
    target.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG( << "Failed to load module, got exception: "
                        << e.what() << " + in frameSL02a:FAIL" );
  }
#endif

  Module* moduleA = ModuleRegistry::GetModule("TestModuleA");
  US_TEST_CONDITION_REQUIRED(moduleA != 0, "Test for existing module TestModuleA")

  std::vector<ModuleEvent> pEvts;
#ifdef US_BUILD_SHARED_LIBS
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleA));
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleA));
#endif

  std::vector<ServiceEvent> seEvts;

  US_TEST_CONDITION(listener1.CheckListenerEvents(pEvts, seEvts), "Check first module listener")
  US_TEST_CONDITION(listener2.CheckListenerEvents(pEvts, seEvts), "Check second module listener")

  mc->RemoveModuleListener(&listener1, &TestModuleListener::ModuleChanged);
  mc->RemoveModuleListener(&listener2, &TestModuleListener::ModuleChanged);

  target.Unload();
}

// Verify information from the ModuleInfo struct
void frame005a(ModuleContext* mc)
{
  Module* m = mc->GetModule();

  // check expected headers

  US_TEST_CONDITION("main" == m->GetName(), "Test module name")
  US_TEST_CONDITION(ModuleVersion(0,1,0) == m->GetVersion(), "Test test driver module version")
  US_TEST_CONDITION(ModuleVersion(CppMicroServices_MAJOR_VERSION, CppMicroServices_MINOR_VERSION, CppMicroServices_PATCH_VERSION) == ModuleRegistry::GetModule(1)->GetVersion(), "Test CppMicroServices version")
}

// Get context id, location and status of the module
void frame010a(ModuleContext* mc)
{
  Module* m = mc->GetModule();

  long int contextid = m->GetModuleId();
  US_DEBUG << "CONTEXT ID:" << contextid;

  std::string location = m->GetLocation();
  US_DEBUG << "LOCATION:" << location;
  US_TEST_CONDITION(!location.empty(), "Test for non-empty module location")

  US_TEST_CONDITION(m->IsLoaded(), "Test for loaded flag")

  US_TEST_CONDITION(ModuleSettings::GetStoragePath().empty(), "Test for empty base storage path")
  US_TEST_CONDITION(m->GetModuleContext()->GetDataFile("").empty(), "Test for empty data path")
  US_TEST_CONDITION(m->GetModuleContext()->GetDataFile("bla").empty(), "Test for empty data file path")
}

//----------------------------------------------------------------------------
//Test result of GetService(ServiceReference()). Should throw std::invalid_argument
void frame018a(ModuleContext* mc)
{
  try
  {
    mc->GetService(ServiceReferenceU());
    US_DEBUG << "Got service object, expected std::invalid_argument exception";
    US_TEST_FAILED_MSG(<< "Got service object, excpected std::invalid_argument exception")
  }
  catch (const std::invalid_argument& )
  {}
  catch (...)
  {
    US_TEST_FAILED_MSG(<< "Got wrong exception, expected std::invalid_argument")
  }
}

// Load libA and check that it exists and that the service it registers exists,
// also check that the expected events occur and that the storage paths are correct
void frame020a(ModuleContext* mc, TestModuleListener& listener,
#ifdef US_BUILD_SHARED_LIBS
               SharedLibrary& libA)
{
  try
  {
    libA.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }
#else
               SharedLibrary& /*libA*/)
{
#endif

  ModuleSettings::SetStoragePath(std::string("/tmp") + PATH_SEPARATOR);
  US_TEST_CONDITION(ModuleSettings::GetStoragePath() == "/tmp", "Test for valid base storage path")

  Module* moduleA = ModuleRegistry::GetModule("TestModuleA");
  US_TEST_CONDITION_REQUIRED(moduleA != 0, "Test for existing module TestModuleA")

  US_TEST_CONDITION(moduleA->GetName() == "TestModuleA", "Test module name")

  std::cout << moduleA->GetModuleContext()->GetDataFile("") << std::endl;
  std::stringstream ss;
  ss << moduleA->GetModuleId();
  const std::string baseStoragePath = std::string("/tmp") + PATH_SEPARATOR + ss.str() + "_TestModuleA" + PATH_SEPARATOR;
  US_TEST_CONDITION(moduleA->GetModuleContext()->GetDataFile("") == baseStoragePath, "Test for valid data path")
  US_TEST_CONDITION(moduleA->GetModuleContext()->GetDataFile("bla") == baseStoragePath + "bla", "Test for valid data file path")

  // Check if libA registered the expected service
  try
  {
    ServiceReferenceU sr1 = mc->GetServiceReference("us::TestModuleAService");
    InterfaceMap o1 = mc->GetService(sr1);
    US_TEST_CONDITION(!o1.empty(), "Test if service object found");

    try
    {
      US_TEST_CONDITION(mc->UngetService(sr1), "Test if Service UnGet returns true");
    }
    catch (const std::logic_error le)
    {
      US_TEST_FAILED_MSG(<< "UnGetService exception: " << le.what())
    }

    // check the listeners for events
    std::vector<ModuleEvent> pEvts;
#ifdef US_BUILD_SHARED_LIBS
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleA));
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleA));
#endif

    std::vector<ServiceEvent> seEvts;
#ifdef US_BUILD_SHARED_LIBS
    seEvts.push_back(ServiceEvent(ServiceEvent::REGISTERED, sr1));
#endif

    US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");

  }
  catch (const ServiceException& /*se*/)
  {
    US_TEST_FAILED_MSG(<< "test module, expected service not found");
  }

  US_TEST_CONDITION(moduleA->IsLoaded() == true, "Test if loaded correctly");
}


// Unload libA and check for correct events
void frame030b(ModuleContext* mc, TestModuleListener& listener, SharedLibrary& libA)
{
  Module* moduleA = ModuleRegistry::GetModule("TestModuleA");
  US_TEST_CONDITION_REQUIRED(moduleA != 0, "Test for non-null module")

  ServiceReferenceU sr1
      = mc->GetServiceReference("us::TestModuleAService");
  US_TEST_CONDITION(sr1, "Test for valid service reference")

  try
  {
    libA.Unload();
#ifdef US_BUILD_SHARED_LIBS
    US_TEST_CONDITION(moduleA->IsLoaded() == false, "Test for unloaded state")
#endif
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "UnLoad module exception: " << e.what())
  }

  std::vector<ModuleEvent> pEvts;
#ifdef US_BUILD_SHARED_LIBS
  pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADING, moduleA));
  pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADED, moduleA));
#endif

  std::vector<ServiceEvent> seEvts;
#ifdef US_BUILD_SHARED_LIBS
  seEvts.push_back(ServiceEvent(ServiceEvent::UNREGISTERING, sr1));
#endif

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");
}


struct LocalListener {
  void ServiceChanged(const ServiceEvent) {}
};

// Add a service listener with a broken LDAP filter to Get an exception
void frame045a(ModuleContext* mc)
{
  LocalListener sListen1;
  std::string brokenFilter = "A broken LDAP filter";

  try
  {
    mc->AddServiceListener(&sListen1, &LocalListener::ServiceChanged, brokenFilter);
  }
  catch (const std::invalid_argument& /*ia*/)
  {
    //assertEquals("InvalidSyntaxException.GetFilter should be same as input string", brokenFilter, ise.GetFilter());
  }
  catch (...)
  {
    US_TEST_FAILED_MSG(<< "test module, wrong exception on broken LDAP filter:");
  }
}

} // end unnamed namespace

int usModuleTest(int /*argc*/, char* /*argv*/[])
{
  //US_TEST_BEGIN("ModuleTest");

  std::vector<Module*> modules = ModuleRegistry::GetModules();
  for (std::vector<Module*>::iterator iter = modules.begin(), iterEnd = modules.end();
       iter != iterEnd; ++iter)
  {
    std::cout << "----- " << (*iter)->GetName() << std::endl;
  }

  frame01();
  frame02a();

  ModuleContext* mc = GetModuleContext();
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

  try
  {
    mc->AddServiceListener(&listener, &TestModuleListener::ServiceChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_OUTPUT( << "service listener registration failed " << ise.what() );
    throw;
  }

  frame005a(mc);
  frame010a(mc);
  frame018a(mc);

  SharedLibrary libA(LIB_PATH, "TestModuleA");
  frame020a(mc, listener, libA);
  frame030b(mc, listener, libA);

  frame045a(mc);

  mc->RemoveModuleListener(&listener, &TestModuleListener::ModuleChanged);
  mc->RemoveServiceListener(&listener, &TestModuleListener::ServiceChanged);

  //US_TEST_END()
  return 0;
}
