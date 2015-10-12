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

#include <usModule.h>
#include <usModuleEvent.h>
#include <usServiceEvent.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModuleActivator.h>
#include <usModuleSettings.h>
#include <usLog.h>

#include "usTestUtils.h"
#include "usTestUtilModuleListener.h"
#include "usTestDriverActivator.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

using namespace us;

namespace {

// Check that the executable's activator was loaded and called
void frame01(ModuleContext* mc)
{
  try
  {
    Module* module = mc->InstallBundle(BIN_PATH + DIR_SEP + "usCoreTestDriver" + EXE_EXT + "/main");
    US_TEST_CONDITION_REQUIRED(module != NULL, "Test installation of module main")

    module->Start();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Install bundle exception: " << e.what() << " + in frame01:FAIL")
  }

  US_TEST_CONDITION_REQUIRED(TestDriverActivator::LoadCalled(), "ModuleActivator::Load() called for executable")
}

// Verify that the same member function pointers registered as listeners
// with different receivers works.
void frame02a(ModuleContext* mc)
{
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
  
  InstallTestBundle(mc, "TestModuleA");

  Module* moduleA = mc->GetModule("TestModuleA");
  US_TEST_CONDITION_REQUIRED(moduleA != nullptr, "Test for existing module TestModuleA")

  moduleA->Start();

  std::vector<ModuleEvent> pEvts;
  pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, moduleA));
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleA));
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleA));

  std::vector<ServiceEvent> seEvts;

  US_TEST_CONDITION(listener1.CheckListenerEvents(pEvts, seEvts), "Check first module listener")
  US_TEST_CONDITION(listener2.CheckListenerEvents(pEvts, seEvts), "Check second module listener")

  mc->RemoveModuleListener(&listener1, &TestModuleListener::ModuleChanged);
  mc->RemoveModuleListener(&listener2, &TestModuleListener::ModuleChanged);

  moduleA->Stop();

}

// Verify information from the ModuleInfo struct
void frame005a(ModuleContext* mc)
{
  Module* m = mc->GetModule();

  // check expected headers

  US_TEST_CONDITION("main" == m->GetName(), "Test module name")
  US_TEST_CONDITION(ModuleVersion(0,1,0) == m->GetVersion(), "Test test driver module version")
  US_TEST_CONDITION(ModuleVersion(CppMicroServices_MAJOR_VERSION, CppMicroServices_MINOR_VERSION, CppMicroServices_PATCH_VERSION) == mc->GetModule(1)->GetVersion(), "Test CppMicroServices version")
}

// Get context id, location, persistent storage and status of the module
void frame010a(Framework* framework, ModuleContext* mc)
{
  Module* m = mc->GetModule();

  long int contextid = m->GetModuleId();
  US_DEBUG << "CONTEXT ID:" << contextid;

  std::string location = m->GetLocation();
  US_DEBUG << "LOCATION:" << location;
  US_TEST_CONDITION(!location.empty(), "Test for non-empty module location")

  US_TEST_CONDITION(m->IsLoaded(), "Test for loaded flag")

  // launching properties should be accessible through any bundle
  US_TEST_CONDITION(framework->GetProperty("org.osgi.framework.storage").Empty(), "Test for empty base storage path")
  US_TEST_CONDITION(m->GetProperty("org.osgi.framework.storage").Empty(), "Test for empty base storage path")
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

// Start libA and check that it exists and that the service it registers exists,
// also check that the expected events occur
void frame020a(Framework* framework, TestModuleListener& listener)
{
  ModuleContext* mc = framework->GetModuleContext();

  Module* moduleA = mc->GetModule("TestModuleA");
  US_TEST_CONDITION_REQUIRED(moduleA != nullptr, "Test for existing module TestModuleA")

  US_TEST_CONDITION(moduleA->GetName() == "TestModuleA", "Test module name")

  moduleA->Start();

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
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleA));
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleA));

    std::vector<ServiceEvent> seEvts;
    seEvts.push_back(ServiceEvent(ServiceEvent::REGISTERED, sr1));

    US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");

  }
  catch (const ServiceException& /*se*/)
  {
    US_TEST_FAILED_MSG(<< "test module, expected service not found");
  }

  US_TEST_CONDITION(moduleA->IsLoaded() == true, "Test if loaded correctly");
}


// Start libA and check that it exists and that the storage paths are correct
void frame02b(Framework* framework)
{
  ModuleContext* mc = framework->GetModuleContext();

  US_TEST_CONDITION(framework->GetProperty("org.osgi.framework.storage").ToString() == "/tmp", "Test for valid base storage path")

  Module* moduleA = mc->GetModule("TestModuleA");
  US_TEST_CONDITION_REQUIRED(moduleA != nullptr, "Test for existing module TestModuleA")
  // launching properties should be accessible through any bundle
  US_TEST_CONDITION(moduleA->GetProperty("org.osgi.framework.storage").ToString() == "/tmp", "Test for valid base storage path")
  US_TEST_CONDITION(moduleA->GetName() == "TestModuleA", "Test module name")

  moduleA->Start();

  std::cout << moduleA->GetModuleContext()->GetDataFile("") << std::endl;
  std::stringstream ss;
  ss << moduleA->GetModuleId();
  const std::string baseStoragePath = std::string("/tmp") + DIR_SEP + ss.str() + "_TestModuleA" + DIR_SEP;
  US_TEST_CONDITION(moduleA->GetModuleContext()->GetDataFile("") == baseStoragePath, "Test for valid data path")
  US_TEST_CONDITION(moduleA->GetModuleContext()->GetDataFile("bla") == baseStoragePath + "bla", "Test for valid data file path")

  US_TEST_CONDITION(moduleA->IsLoaded() == true, "Test if loaded correctly");
}


// Stop libA and check for correct events
void frame030b(ModuleContext* mc, TestModuleListener& listener)
{
  Module* moduleA = mc->GetModule("TestModuleA");
  US_TEST_CONDITION_REQUIRED(moduleA != nullptr, "Test for non-null module")

  ServiceReferenceU sr1
      = mc->GetServiceReference("us::TestModuleAService");
  US_TEST_CONDITION(sr1, "Test for valid service reference")

  try
  {
    moduleA->Stop();
    US_TEST_CONDITION(moduleA->IsLoaded() == false, "Test for unloaded state")
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "UnLoad module exception: " << e.what())
  }

  std::vector<ModuleEvent> pEvts;
  pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADING, moduleA));
  pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADED, moduleA));

  std::vector<ServiceEvent> seEvts;
  seEvts.push_back(ServiceEvent(ServiceEvent::UNREGISTERING, sr1));

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");
}


struct LocalListener {
  void ServiceChanged(const ServiceEvent&) {}
};

// Add a service listener with a broken LDAP filter to Get an exception
void frame045a(ModuleContext* mc)
{
  LocalListener sListen1;
  std::string brokenFilter = "A broken LDAP filter";

  try
  {
    mc->AddServiceListener(&sListen1, &LocalListener::ServiceChanged, brokenFilter);
    US_TEST_FAILED_MSG(<< "test module, no exception on broken LDAP filter:");
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

void TestModuleStates()
{
    TestModuleListener listener;
    std::vector<ModuleEvent> bundleEvents;
    FrameworkFactory factory;

    Framework* framework = factory.NewFramework(std::map<std::string, std::string>());
    framework->Start();

    ModuleContext* frameworkCtx = framework->GetModuleContext();
    frameworkCtx->AddModuleListener(&listener, &TestModuleListener::ModuleChanged);

    Module* module = nullptr;

    // Test install -> uninstall
    // expect 2 event (INSTALLED, UNINSTALLED)
    module = InstallTestBundle(frameworkCtx, "TestModuleA");
    module->Uninstall();
    US_TEST_CONDITION(0 == frameworkCtx->GetModule("TestModuleA"), "Test bundle install -> uninstall")
    US_TEST_CONDITION(1 == frameworkCtx->GetModules().size(), "Test # of installed bundles")
    bundleEvents.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
    bundleEvents.push_back(ModuleEvent(ModuleEvent::UNINSTALLED, module));
    US_TEST_CONDITION(listener.CheckListenerEvents(bundleEvents), "Test for unexpected events");
    bundleEvents.clear();

    // Test install -> start -> uninstall
    // expect 6 events (INSTALLED, LOADING, LOADED, UNLOADING, UNLOADED, UNINSTALLED)
    module = InstallTestBundle(frameworkCtx, "TestModuleA");
    module->Start();
    module->Uninstall();
    US_TEST_CONDITION(0 == frameworkCtx->GetModule("TestModuleA"), "Test bundle install -> start -> uninstall")
    US_TEST_CONDITION(1 == frameworkCtx->GetModules().size(), "Test # of installed bundles")
    bundleEvents.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
    bundleEvents.push_back(ModuleEvent(ModuleEvent::LOADING, module));
    bundleEvents.push_back(ModuleEvent(ModuleEvent::LOADED, module));
    bundleEvents.push_back(ModuleEvent(ModuleEvent::UNLOADING, module));
    bundleEvents.push_back(ModuleEvent(ModuleEvent::UNLOADED, module));
    bundleEvents.push_back(ModuleEvent(ModuleEvent::UNINSTALLED, module));
    US_TEST_CONDITION(listener.CheckListenerEvents(bundleEvents), "Test for unexpected events");
    bundleEvents.clear();

    // Test install -> stop -> uninstall
    // expect 2 event (INSTALLED, UNINSTALLED)
    module = InstallTestBundle(frameworkCtx, "TestModuleA");
    module->Stop();
    module->Uninstall();
    US_TEST_CONDITION(0 == frameworkCtx->GetModule("TestModuleA"), "Test bundle install -> stop -> uninstall")
    US_TEST_CONDITION(1 == frameworkCtx->GetModules().size(), "Test # of installed bundles")
    bundleEvents.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
    bundleEvents.push_back(ModuleEvent(ModuleEvent::UNINSTALLED, module));
    US_TEST_CONDITION(listener.CheckListenerEvents(bundleEvents), "Test for unexpected events");
    bundleEvents.clear();

    // Test install -> start -> stop -> uninstall
    // expect 6 events (INSTALLED, LOADING, LOADED, UNLOADING, UNLOADED, UNINSTALLED)
    module = InstallTestBundle(frameworkCtx, "TestModuleA");
    module->Start();
    module->Stop();
    module->Uninstall();
    US_TEST_CONDITION(0 == frameworkCtx->GetModule("TestModuleA"), "Test bundle install -> start -> stop -> uninstall")
    US_TEST_CONDITION(1 == frameworkCtx->GetModules().size(), "Test # of installed bundles")
    bundleEvents.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
    bundleEvents.push_back(ModuleEvent(ModuleEvent::LOADING, module));
    bundleEvents.push_back(ModuleEvent(ModuleEvent::LOADED, module));
    bundleEvents.push_back(ModuleEvent(ModuleEvent::UNLOADING, module));
    bundleEvents.push_back(ModuleEvent(ModuleEvent::UNLOADED, module));
    bundleEvents.push_back(ModuleEvent(ModuleEvent::UNINSTALLED, module));
    US_TEST_CONDITION(listener.CheckListenerEvents(bundleEvents), "Test for unexpected events");
    bundleEvents.clear();

    framework->Stop();
    delete framework;
}

void TestForInstallFailure()
{
    FrameworkFactory factory;

    Framework* framework = factory.NewFramework(std::map<std::string, std::string>());
    framework->Start();

    ModuleContext* frameworkCtx = framework->GetModuleContext();

    // Test that bogus bundle installs throw the appropriate exception
    try
    {
        frameworkCtx->InstallBundle(std::string());
        US_TEST_FAILED_MSG(<< "Failed to throw a std::runtime_error")
    }
    catch (const std::runtime_error& ex)
    {
        US_TEST_OUTPUT(<< "Caught std::runtime_exception: " << ex.what())
    }
    catch (...)
    {
        US_TEST_FAILED_MSG(<< "Failed to throw a std::runtime_error")
    }

    try
    {
        frameworkCtx->InstallBundle(std::string("\\path\\which\\won't\\exist\\phantom_bundle"));
        US_TEST_FAILED_MSG(<< "Failed to throw a std::runtime_error")
    }
    catch (const std::runtime_error& ex)
    {
        US_TEST_OUTPUT(<< "Caught std::runtime_exception: " << ex.what())
    }
    catch (...)
    {
        US_TEST_FAILED_MSG(<< "Failed to throw a std::runtime_error")
    }

    US_TEST_CONDITION(1 == frameworkCtx->GetModules().size(), "Test # of installed bundles")

    framework->Stop();
    delete framework;
}

void TestDuplicateInstall()
{
    FrameworkFactory factory;

    Framework* framework = factory.NewFramework(std::map<std::string, std::string>());
    framework->Start();

    ModuleContext* frameworkCtx = framework->GetModuleContext();

    // Test installing the same bundle (i.e. a bundle with the same location) twice.
    // The exact same bundle should be returned on the second install.
    Module* module = InstallTestBundle(frameworkCtx, "TestModuleA");
    Module* moduleDuplicate = InstallTestBundle(frameworkCtx, "TestModuleA");

    US_TEST_CONDITION(module == moduleDuplicate, "Test for the same bundle instance");
    US_TEST_CONDITION(module->GetModuleId() == moduleDuplicate->GetModuleId(), "Test for the same bundle id");

    framework->Stop();
    delete framework;
}

} // end unnamed namespace

int usModuleTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ModuleTest");

  FrameworkFactory factory;
  Framework* framework = factory.NewFramework(std::map<std::string, std::string>());
  framework->Start();

  std::vector<Module*> modules = framework->GetModuleContext()->GetModules();
  for (std::vector<Module*>::iterator iter = modules.begin(), iterEnd = modules.end();
       iter != iterEnd; ++iter)
  {
    std::cout << "----- " << (*iter)->GetName() << std::endl;
  }

  ModuleContext* mc = framework->GetModuleContext();
  TestModuleListener listener;

  frame01(mc);
  frame02a(mc);

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

  frame005a(mc->GetModule("main")->GetModuleContext());
  frame010a(framework, mc);
  frame018a(mc);

  frame020a(framework, listener);
  frame030b(mc, listener);

  frame045a(mc);

  mc->RemoveModuleListener(&listener, &TestModuleListener::ModuleChanged);
  mc->RemoveServiceListener(&listener, &TestModuleListener::ServiceChanged);

  framework->Stop();
  delete framework;

  // test a non-default framework instance using a different persistent storage location.
  std::map<std::string, std::string> frameworkConfig;
  frameworkConfig.insert(std::pair<std::string, std::string>("org.osgi.framework.storage", "/tmp"));
  framework = factory.NewFramework(frameworkConfig);
  framework->Start();

  frame02a(framework->GetModuleContext());
  frame02b(framework);

  delete framework;

  TestModuleStates();
  TestForInstallFailure();
  TestDuplicateInstall();

  US_TEST_END()
}
