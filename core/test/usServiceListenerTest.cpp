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

#include <usTestUtils.h>
#include <usTestingMacros.h>
#include <usTestingConfig.h>

#include <usModule.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usSharedLibrary.h>

#include <usModulePropsInterface.h>

US_USE_NAMESPACE

class TestServiceListener
{

private:

  friend bool runLoadUnloadTest(const std::string&, int cnt, Module&,
                                ModuleContext* mc,
                                const std::vector<ServiceEvent::Type>&);

  const bool checkUsingModules;
  std::vector<ServiceEvent> events;

  bool teststatus;

  ModuleContext* mc;

public:

  TestServiceListener(ModuleContext* mc, bool checkUsingModules = true)
    : checkUsingModules(checkUsingModules), events(), teststatus(true), mc(mc)
  {}

  bool getTestStatus() const
  {
    return teststatus;
  }

  void clearEvents()
  {
    events.clear();
  }

  bool checkEvents(const std::vector<ServiceEvent::Type>& eventTypes)
  {
    if (events.size() != eventTypes.size())
    {
      dumpEvents(eventTypes);
      return false;
    }

    for (std::size_t i=0; i < eventTypes.size(); ++i)
    {
      if (eventTypes[i] != events[i].GetType())
      {
        dumpEvents(eventTypes);
        return false;
      }
    }
    return true;
  }

  void serviceChanged(const ServiceEvent evt)
  {
    events.push_back(evt);
    US_TEST_OUTPUT( << "ServiceEvent: " << evt );
    if (ServiceEvent::UNREGISTERING == evt.GetType())
    {
      ServiceReferenceU sr = evt.GetServiceReference();

      // Validate that no module is marked as using the service
      std::vector<Module*> usingModules;
      sr.GetUsingModules(usingModules);
      if (checkUsingModules && !usingModules.empty())
      {
        teststatus = false;
        printUsingModules(sr, "*** Using modules (unreg) should be empty but is: ");
      }

      // Check if the service can be fetched
      InterfaceMap service = mc->GetService(sr);
      sr.GetUsingModules(usingModules);
      // if (UNREGISTERSERVICE_VALID_DURING_UNREGISTERING) {
      // In this mode the service shall be obtainable during
      // unregistration.
      if (service.empty())
      {
        teststatus = false;
        US_TEST_OUTPUT( << "*** Service should be available to ServiceListener "
                          << "while handling unregistering event." );
      }
      US_TEST_OUTPUT( << "Service (unreg): " << service.begin()->first << " -> " << service.begin()->second );
      if (checkUsingModules && usingModules.size() != 1)
      {
        teststatus = false;
        printUsingModules(sr, "*** One using module expected "
                          "(unreg, after getService), found: ");
      }
      else
      {
        printUsingModules(sr, "Using modules (unreg, after getService): ");
      }
      //    } else {
      //      // In this mode the service shall NOT be obtainable during
      //      // unregistration.
      //      if (null!=service) {
      //        teststatus = false;
      //        out.print("*** Service should not be available to ServiceListener "
      //                  +"while handling unregistering event.");
      //      }
      //      if (checkUsingBundles && null!=usingBundles) {
      //        teststatus = false;
      //        printUsingBundles(sr,
      //                          "*** Using bundles (unreg, after getService), "
      //                          +"should be null but is: ");
      //      } else {
      //        printUsingBundles(sr,
      //                          "Using bundles (unreg, after getService): null");
      //      }
      //    }
      mc->UngetService(sr);

      // Check that the UNREGISTERING service can not be looked up
      // using the service registry.
      try
      {
        long sid = any_cast<long>(sr.GetProperty(ServiceConstants::SERVICE_ID()));
        std::stringstream ss;
        ss << "(" << ServiceConstants::SERVICE_ID() << "=" << sid << ")";
        std::vector<ServiceReferenceU> srs = mc->GetServiceReferences("", ss.str());
        if (srs.empty())
        {
          US_TEST_OUTPUT( << "ServiceReference for UNREGISTERING service is not"
                            " found in the service registry; ok." );
        }
        else
        {
          teststatus = false;
          US_TEST_OUTPUT( << "*** ServiceReference for UNREGISTERING service,"
                            << sr << ", not found in the service registry; fail." );
          US_TEST_OUTPUT( << "Found the following Service references:") ;
          for(std::vector<ServiceReferenceU>::const_iterator sr = srs.begin();
              sr != srs.end(); ++sr)
          {
            US_TEST_OUTPUT( << (*sr) );
          }
        }
      }
      catch (const std::exception& e)
      {
        teststatus = false;
        US_TEST_OUTPUT( << "*** Unexpected excpetion when trying to lookup a"
                          " service while it is in state UNREGISTERING: "
                          << e.what() );
      }
    }
  }

  void printUsingModules(const ServiceReferenceU& sr, const std::string& caption)
  {
    std::vector<Module*> usingModules;
    sr.GetUsingModules(usingModules);

    US_TEST_OUTPUT( << (caption.empty() ? "Using modules: " : caption) );
    for(std::vector<Module*>::const_iterator module = usingModules.begin();
        module != usingModules.end(); ++module)
    {
      US_TEST_OUTPUT( << "  -" << (*module) );
    }
  }

  // Print expected and actual service events.
  void dumpEvents(const std::vector<ServiceEvent::Type>& eventTypes)
  {
    std::size_t max = events.size() > eventTypes.size() ? events.size() : eventTypes.size();
    US_TEST_OUTPUT( << "Expected event type --  Actual event" );
    for (std::size_t i=0; i < max; ++i)
    {
      ServiceEvent evt = i < events.size() ? events[i] : ServiceEvent();
      if (i < eventTypes.size())
      {
        US_TEST_OUTPUT( << " " << eventTypes[i] << "--" << evt );
      }
      else
      {
        US_TEST_OUTPUT( << " " << "- NONE - " << "--" << evt );
      }
    }
  }

}; // end of class ServiceListener

bool runLoadUnloadTest(const std::string& name, int cnt, Module& module,
                       ModuleContext* mc,
                       const std::vector<ServiceEvent::Type>& events)
{
  bool teststatus = true;

  for (int i = 0; i < cnt && teststatus; ++i)
  {
    TestServiceListener sListen(mc);
    try
    {
      mc->AddServiceListener(&sListen, &TestServiceListener::serviceChanged);
      //mc->AddServiceListener(MessageDelegate1<ServiceListener, const ServiceEvent&>(&sListen, &ServiceListener::serviceChanged));
    }
    catch (const std::logic_error& ise)
    {
      teststatus  = false;
      US_TEST_FAILED_MSG( << "service listener registration failed " << ise.what()
                          << " :" << name << ":FAIL" );
    }

    // Start the test target to get a service published.
    try
    {
      module.Start();
    }
    catch (const std::exception& e)
    {
      teststatus  = false;
      US_TEST_FAILED_MSG( << "Failed to load module, got exception: "
                          << e.what() << " + in " << name << ":FAIL" );
    }

    // Stop the test target to get a service unpublished.
    try
    {
      module.Stop();
    }
    catch (const std::exception& e)
    {
      teststatus  = false;
      US_TEST_FAILED_MSG( << "Failed to unload module, got exception: "
                          << e.what() << " + in " << name << ":FAIL" );
    }

    if (teststatus && !sListen.checkEvents(events))
    {
      teststatus  = false;
      US_TEST_FAILED_MSG( << "Service listener event notification error :"
                          << name << ":FAIL" );
    }

    try
    {
      mc->RemoveServiceListener(&sListen, &TestServiceListener::serviceChanged);
      teststatus &= sListen.teststatus;
      sListen.clearEvents();
    }
    catch (const std::logic_error& ise)
    {
      teststatus  = false;
      US_TEST_FAILED_MSG( << "service listener removal failed " << ise.what()
                          << " :" << name << ":FAIL" );
    }
  }
  return teststatus;
}

void frameSL02a(Framework* framework)
{
  ModuleContext* mc = framework->GetModuleContext();

  TestServiceListener listener1(mc);
  TestServiceListener listener2(mc);

  try
  {
    mc->RemoveServiceListener(&listener1, &TestServiceListener::serviceChanged);
    mc->AddServiceListener(&listener1, &TestServiceListener::serviceChanged);
    mc->RemoveServiceListener(&listener2, &TestServiceListener::serviceChanged);
    mc->AddServiceListener(&listener2, &TestServiceListener::serviceChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_FAILED_MSG( << "service listener registration failed " << ise.what()
                        << " : frameSL02a:FAIL" );
  }

  Module* module = InstallTestBundle(mc, "TestModuleA");
  module->Start();

  std::vector<ServiceEvent::Type> events;
  events.push_back(ServiceEvent::REGISTERED);

  US_TEST_CONDITION(listener1.checkEvents(events), "Check first service listener")
  US_TEST_CONDITION(listener2.checkEvents(events), "Check second service listener")

  mc->RemoveServiceListener(&listener1, &TestServiceListener::serviceChanged);
  mc->RemoveServiceListener(&listener2, &TestServiceListener::serviceChanged);

  module->Stop();
}

void frameSL05a(Framework* framework)
{
  std::vector<ServiceEvent::Type> events;
  events.push_back(ServiceEvent::REGISTERED);
  events.push_back(ServiceEvent::UNREGISTERING);
  
  Module* module = InstallTestBundle(framework->GetModuleContext(), "TestModuleA");

  bool testStatus = runLoadUnloadTest("FrameSL05a", 1, *module, framework->GetModuleContext(), events);
  US_TEST_CONDITION(testStatus, "FrameSL05a")
}

void frameSL10a(Framework* framework)
{
  std::vector<ServiceEvent::Type> events;
  events.push_back(ServiceEvent::REGISTERED);
  events.push_back(ServiceEvent::UNREGISTERING);
  
  Module* module = InstallTestBundle(framework->GetModuleContext(), "TestModuleA2");

  bool testStatus = runLoadUnloadTest("FrameSL10a", 1, *module, framework->GetModuleContext(), events);
  US_TEST_CONDITION(testStatus, "FrameSL10a")
}

void frameSL25a(Framework* framework)
{
  ModuleContext* mc = framework->GetModuleContext();

  TestServiceListener sListen(mc, false);
  try
  {
    mc->AddServiceListener(&sListen, &TestServiceListener::serviceChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_OUTPUT( << "service listener registration failed " << ise.what() );
    throw;
  }

  Module* libSL1 = InstallTestBundle(mc, "TestModuleSL1");
  Module* libSL3 = InstallTestBundle(mc, "TestModuleSL3");
  Module* libSL4 = InstallTestBundle(mc, "TestModuleSL4");

  std::vector<ServiceEvent::Type> expectedServiceEventTypes;

  // Startup
  expectedServiceEventTypes.push_back(ServiceEvent::REGISTERED); // at start of libSL1
  expectedServiceEventTypes.push_back(ServiceEvent::REGISTERED); // FooService at start of libSL4
  expectedServiceEventTypes.push_back(ServiceEvent::REGISTERED); // at start of libSL3

  // Stop libSL4
  expectedServiceEventTypes.push_back(ServiceEvent::UNREGISTERING); // FooService at first stop of libSL4

#ifdef US_BUILD_SHARED_LIBS
  // Shutdown
  expectedServiceEventTypes.push_back(ServiceEvent::UNREGISTERING); // at stop of libSL1
  expectedServiceEventTypes.push_back(ServiceEvent::UNREGISTERING); // at stop of libSL3
#endif

  // Start libModuleTestSL1 to ensure that the Service interface is available.
  try
  {
    US_TEST_OUTPUT( << "Starting libModuleTestSL1: " << libSL1->GetLocation() );
    libSL1->Start();
  }
  catch (const std::exception& e)
  {
    US_TEST_OUTPUT( << "Failed to load module, got exception: " << e.what() );
    throw;
  }

  // Start libModuleTestSL4 that will require the serivce interface and publish
  // us::FooService
  try
  {
    US_TEST_OUTPUT( << "Starting libModuleTestSL4: " << libSL4->GetLocation() );
    libSL4->Start();
  }
  catch (const std::exception& e)
  {
    US_TEST_OUTPUT( << "Failed to load module, got exception: " << e.what() );
    throw;
  }

  // Start libModuleTestSL3 that will require the serivce interface and get the service
  try
  {
    US_TEST_OUTPUT( << "Starting libModuleTestSL3: " << libSL3->GetLocation() );
    libSL3->Start();
  }
  catch (const std::exception& e)
  {
    US_TEST_OUTPUT( << "Failed to load module, got exception: " << e.what() );
    throw;
  }

  // Check that libSL3 has been notified about the FooService.
  US_TEST_OUTPUT( << "Check that FooService is added to service tracker in libSL3" );
  try
  {
    ServiceReferenceU libSL3SR = mc->GetServiceReference("ActivatorSL3");
    InterfaceMap libSL3Activator = mc->GetService(libSL3SR);
    US_TEST_CONDITION_REQUIRED(!libSL3Activator.empty(), "ActivatorSL3 service != 0");

    ServiceReference<ModulePropsInterface> libSL3PropsI(libSL3SR);
    ModulePropsInterface* propsInterface = mc->GetService(libSL3PropsI);
    US_TEST_CONDITION_REQUIRED(propsInterface, "ModulePropsInterface != 0");

    ModulePropsInterface::Properties::const_iterator i = propsInterface->GetProperties().find("serviceAdded");
    US_TEST_CONDITION_REQUIRED(i != propsInterface->GetProperties().end(), "Property serviceAdded");
    Any serviceAddedField3 = i->second;
    US_TEST_CONDITION_REQUIRED(!serviceAddedField3.Empty() && any_cast<bool>(serviceAddedField3),
                                 "libSL3 notified about presence of FooService");
    mc->UngetService(libSL3SR);
  }
  catch (const ServiceException& se)
  {
    US_TEST_FAILED_MSG( << "Failed to get service reference:" << se );
  }

  // Check that libSL1 has been notified about the FooService.
  US_TEST_OUTPUT( << "Check that FooService is added to service tracker in libSL1" );
  try
  {
    ServiceReferenceU libSL1SR = mc->GetServiceReference("ActivatorSL1");
    InterfaceMap libSL1Activator = mc->GetService(libSL1SR);
    US_TEST_CONDITION_REQUIRED(!libSL1Activator.empty(), "ActivatorSL1 service != 0");

    ServiceReference<ModulePropsInterface> libSL1PropsI(libSL1SR);
    ModulePropsInterface* propsInterface = mc->GetService(libSL1PropsI);
    US_TEST_CONDITION_REQUIRED(propsInterface, "Cast to ModulePropsInterface");

    ModulePropsInterface::Properties::const_iterator i = propsInterface->GetProperties().find("serviceAdded");
    US_TEST_CONDITION_REQUIRED(i != propsInterface->GetProperties().end(), "Property serviceAdded");
    Any serviceAddedField1 = i->second;
    US_TEST_CONDITION_REQUIRED(!serviceAddedField1.Empty() && any_cast<bool>(serviceAddedField1),
                                 "libSL1 notified about presence of FooService");
    mc->UngetService(libSL1SR);
  }
  catch (const ServiceException& se)
  {
    US_TEST_FAILED_MSG( << "Failed to get service reference:" << se );
  }

  // Stop the service provider: libSL4
  try
  {
    US_TEST_OUTPUT( << "Stop libSL4: " << libSL4->GetLocation() );
    libSL4->Stop();
  }
  catch (const std::exception& e)
  {
    US_TEST_OUTPUT( << "Failed to unload module, got exception:" << e.what() );
    throw;
  }

  // Check that libSL3 has been notified about the removal of FooService.
  US_TEST_OUTPUT( << "Check that FooService is removed from service tracker in libSL3" );
  try
  {
    ServiceReferenceU libSL3SR = mc->GetServiceReference("ActivatorSL3");
    InterfaceMap libSL3Activator = mc->GetService(libSL3SR);
    US_TEST_CONDITION_REQUIRED(!libSL3Activator.empty(), "ActivatorSL3 service != 0");

    ServiceReference<ModulePropsInterface> libSL3PropsI(libSL3SR);
    ModulePropsInterface* propsInterface = mc->GetService(libSL3PropsI);
    US_TEST_CONDITION_REQUIRED(propsInterface, "Cast to ModulePropsInterface");

    ModulePropsInterface::Properties::const_iterator i = propsInterface->GetProperties().find("serviceRemoved");
    US_TEST_CONDITION_REQUIRED(i != propsInterface->GetProperties().end(), "Property serviceRemoved");

    Any serviceRemovedField3 = i->second;
    US_TEST_CONDITION(!serviceRemovedField3.Empty() && any_cast<bool>(serviceRemovedField3),
                        "libSL3 notified about removal of FooService");
    mc->UngetService(libSL3SR);
  }
  catch (const ServiceException& se)
  {
    US_TEST_FAILED_MSG( << "Failed to get service reference:" << se );
  }

  // Stop libSL1
  try
  {
    US_TEST_OUTPUT( << "Stop libSL1:" << libSL1->GetLocation() );
    libSL1->Stop();
  }
  catch (const std::exception& e)
  {
    US_TEST_OUTPUT( << "Failed to unload module got exception" << e.what() );
    throw;
  }

  // Stop pSL3
  try
  {
    US_TEST_OUTPUT( << "Stop libSL3:" << libSL3->GetLocation() );
    libSL3->Stop();
  }
  catch (const std::exception& e)
  {
    US_TEST_OUTPUT( << "Failed to unload module got exception" << e.what() );
    throw;
  }

  // Check service events seen by this class
  US_TEST_OUTPUT( << "Checking ServiceEvents(ServiceListener):" );
  if (!sListen.checkEvents(expectedServiceEventTypes))
  {
    US_TEST_FAILED_MSG( << "Service listener event notification error" );
  }

  US_TEST_CONDITION_REQUIRED(sListen.getTestStatus(), "Service listener checks");
  try
  {
    mc->RemoveServiceListener(&sListen, &TestServiceListener::serviceChanged);
    sListen.clearEvents();
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_FAILED_MSG( << "service listener removal failed: " << ise.what() );
  }

}

int usServiceListenerTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceListenerTest");

  FrameworkFactory factory;
  Framework* framework = factory.newFramework(std::map<std::string, std::string>());
  framework->init();
  framework->Start();

  frameSL02a(framework);
  frameSL05a(framework);
  frameSL10a(framework);
  frameSL25a(framework);

  delete framework;

  US_TEST_END()
}
