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

#include US_BASECLASS_HEADER

#include "usTestUtilSharedLibrary.cpp"
#include "usTestingMacros.h"

US_USE_NAMESPACE

extern ModuleActivator* _us_module_activator_instance_TestModuleA();

class TestModuleListener {

public:

  TestModuleListener(ModuleContext* mc) : mc(mc), serviceEvents(), moduleEvents()
  {}

  void ModuleChanged(const ModuleEvent event)
  {
    moduleEvents.push_back(event);
    US_DEBUG << "ModuleEvent:" << event;
  }

  void ServiceChanged(const ServiceEvent event)
  {
    serviceEvents.push_back(event);
    US_DEBUG << "ServiceEvent:" << event;
  }

  ModuleEvent GetModuleEvent() const
  {
    if (moduleEvents.empty())
    {
      return ModuleEvent();
    }
    return moduleEvents.back();
  }

  ServiceEvent GetServiceEvent() const
  {
    if (serviceEvents.empty())
    {
      return ServiceEvent();
    }
    return serviceEvents.back();
  }

  bool CheckListenerEvents(
      bool pexp, ModuleEvent::Type ptype,
      bool sexp, ServiceEvent::Type stype,
      Module* moduleX, ServiceReference* servX)
  {
    std::vector<ModuleEvent> pEvts;
    std::vector<ServiceEvent> seEvts;

    if (pexp) pEvts.push_back(ModuleEvent(ptype, moduleX));
    if (sexp) seEvts.push_back(ServiceEvent(stype, *servX));

    return CheckListenerEvents(pEvts, seEvts);
  }

  bool CheckListenerEvents(
      const std::vector<ModuleEvent>& pEvts,
      const std::vector<ServiceEvent>& seEvts)
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

    if (seEvts.size() != serviceEvents.size())
    {
      listenState = false;
      US_DEBUG << "*** Service event mismatch: expected "
          << seEvts.size() << " event(s), found "
          << serviceEvents.size() << " event(s).";

      const std::size_t max = seEvts.size() > serviceEvents.size()
                      ? seEvts.size() : serviceEvents.size();
      for (std::size_t i = 0; i < max; ++i)
      {
        const ServiceEvent& seE = i < seEvts.size() ? seEvts[i] : ServiceEvent();
        const ServiceEvent& seR = i < serviceEvents.size() ? serviceEvents[i] : ServiceEvent();
        US_DEBUG << "    " << seE << " - " << seR;
      }
    }
    else
    {
      for (std::size_t i = 0; i < seEvts.size(); ++i)
      {
        const ServiceEvent& seE = seEvts[i];
        const ServiceEvent& seR = serviceEvents[i];
        if (seE.GetType() != seR.GetType()
          || (!(seE.GetServiceReference() == seR.GetServiceReference())))
        {
          listenState = false;
          US_DEBUG << "*** Wrong service event: " << seR << " expected " << seE;
        }
      }
    }

    moduleEvents.clear();
    serviceEvents.clear();
    return listenState;
  }

private:

  ModuleContext* const mc;

  std::vector<ServiceEvent> serviceEvents;
  std::vector<ModuleEvent> moduleEvents;
};

// Verify information from the ModuleInfo struct
void frame005a(ModuleContext* mc)
{
  Module* m = mc->GetModule();

  // check expected headers

  US_TEST_CONDITION("CppMicroServicesTestDriver" == m->GetName(), "Test module name");
//  US_DEBUG << "Version: " << m->GetVersion();
  US_TEST_CONDITION(ModuleVersion(0,1,0) == m->GetVersion(), "Test module version")
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
}

//----------------------------------------------------------------------------
//Test result of GetService(ServiceReference()). Should throw std::invalid_argument
void frame018a(ModuleContext* mc)
{
  try
  {
    US_BASECLASS_NAME* obj = mc->GetService(ServiceReference());
    US_DEBUG << "Got service object = " << us_service_impl_name(obj) << ", expected std::invalid_argument exception";
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
// also check that the expected events occur
void frame020a(ModuleContext* mc, TestModuleListener& listener, SharedLibraryHandle& libA)
{
  try
  {
    libA.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

#ifdef US_BUILD_SHARED_LIBS
  Module* moduleA = ModuleRegistry::GetModule("TestModuleA");
  US_TEST_CONDITION_REQUIRED(moduleA != 0, "Test for existing moudle TestModuleA")

  US_TEST_CONDITION(moduleA->GetName() == "TestModuleA", "Test module name")
#endif

  // Check if libA registered the expected service
  try
  {
    ServiceReference sr1 = mc->GetServiceReference("org.cppmicroservices.TestModuleAService");
    US_BASECLASS_NAME* o1 = mc->GetService(sr1);
    US_TEST_CONDITION(o1 != 0, "Test if service object found");

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
    seEvts.push_back(ServiceEvent(ServiceEvent::REGISTERED, sr1));

    US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");

  }
  catch (const ServiceException& /*se*/)
  {
    US_TEST_FAILED_MSG(<< "test module, expected service not found");
  }

#ifdef US_BUILD_SHARED_LIBS
  US_TEST_CONDITION(moduleA->IsLoaded() == true, "Test if loaded correctly");
#endif
}


// Unload libA and check for correct events
void frame030b(ModuleContext* mc, TestModuleListener& listener, SharedLibraryHandle& libA)
{
#ifdef US_BUILD_SHARED_LIBS
  Module* moduleA = ModuleRegistry::GetModule("TestModuleA");
  US_TEST_CONDITION_REQUIRED(moduleA != 0, "Test for non-null module")
#endif

  ServiceReference sr1
      = mc->GetServiceReference("org.cppmicroservices.TestModuleAService");
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
  seEvts.push_back(ServiceEvent(ServiceEvent::UNREGISTERING, sr1));

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


int usModuleTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ModuleTest");

  ModuleContext* mc = GetModuleContext();
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

  SharedLibraryHandle libA("TestModuleA"
                             #ifndef US_BUILD_SHARED_LIBS
                               , _us_module_activator_instance_TestModuleA
                             #endif
                               );
  frame020a(mc, listener, libA);
  frame030b(mc, listener, libA);

  frame045a(mc);

  mc->RemoveModuleListener(&listener, &TestModuleListener::ModuleChanged);
  mc->RemoveServiceListener(&listener, &TestModuleListener::ServiceChanged);

  US_TEST_END()
}
