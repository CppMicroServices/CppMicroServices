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

#include <usTestUtils.h>
#include <usTestingMacros.h>
#include <usTestingConfig.h>

#include <usBundle.h>
#include <usBundleContext.h>
#include <usGetBundleContext.h>
#include <usSharedLibrary.h>
#include <usServiceEvent.h>

#include <usBundlePropsInterface.h>

using namespace us;

class TestServiceListener
{

private:

  friend bool runStartStopTest(const std::string&, int cnt, const std::shared_ptr<Bundle>&,
                                BundleContext* context,
                                const std::vector<ServiceEvent::Type>&);

  const bool checkUsingBundles;
  std::vector<ServiceEvent> events;

  bool teststatus;

  BundleContext* context;

public:

  TestServiceListener(BundleContext* context, bool checkUsingBundles = true)
    : checkUsingBundles(checkUsingBundles), events(), teststatus(true), context(context)
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

  void serviceChanged(const ServiceEvent& evt)
  {
    events.push_back(evt);
    US_TEST_OUTPUT( << "ServiceEvent: " << evt );
    if (ServiceEvent::UNREGISTERING == evt.GetType())
    {
      ServiceReferenceU sr = evt.GetServiceReference();

      // Validate that no bundle is marked as using the service
      std::vector<std::shared_ptr<Bundle>> usingBundles;
      sr.GetUsingBundles(usingBundles);
      if (checkUsingBundles && !usingBundles.empty())
      {
        teststatus = false;
        printUsingBundles(sr, "*** Using bundles (unreg) should be empty but is: ");
      }

      // Check if the service can be fetched
      InterfaceMapConstPtr service = context->GetService(sr);
      sr.GetUsingBundles(usingBundles);
      // if (UNREGISTERSERVICE_VALID_DURING_UNREGISTERING) {
      // In this mode the service shall be obtainable during
      // unregistration.
      if (!service || service->empty())
      {
        teststatus = false;
        US_TEST_OUTPUT( << "*** Service should be available to ServiceListener "
                          << "while handling unregistering event." );
      }
      US_TEST_OUTPUT( << "Service (unreg): " << service->begin()->first << " -> " << service->begin()->second );
      if (checkUsingBundles && usingBundles.size() != 1)
      {
        teststatus = false;
        printUsingBundles(sr, "*** One using bundle expected "
                          "(unreg, after getService), found: ");
      }
      else
      {
        printUsingBundles(sr, "Using bundles (unreg, after getService): ");
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
      service.reset(); // results in UngetService

      // Check that the UNREGISTERING service can not be looked up
      // using the service registry.
      try
      {
        long sid = any_cast<long>(sr.GetProperty(ServiceConstants::SERVICE_ID()));
        std::stringstream ss;
        ss << "(" << ServiceConstants::SERVICE_ID() << "=" << sid << ")";
        std::vector<ServiceReferenceU> srs = context->GetServiceReferences("", ss.str());
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

  void printUsingBundles(const ServiceReferenceU& sr, const std::string& caption)
  {
    std::vector<std::shared_ptr<Bundle>> usingBundles;
    sr.GetUsingBundles(usingBundles);

    US_TEST_OUTPUT( << (caption.empty() ? "Using bundles: " : caption) );
    for (std::vector<std::shared_ptr<Bundle>>::const_iterator bundle = usingBundles.begin();
        bundle != usingBundles.end(); ++bundle)
    {
      US_TEST_OUTPUT( << "  -" << (*bundle) );
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


bool runStartStopTest(const std::string& name, int cnt, const std::shared_ptr<Bundle>& bundle,
                       BundleContext* context,
                       const std::vector<ServiceEvent::Type>& events)
{
  bool teststatus = true;

  for (int i = 0; i < cnt && teststatus; ++i)
  {
    TestServiceListener sListen(context);
    try
    {
      context->AddServiceListener(&sListen, &TestServiceListener::serviceChanged);
      //context->AddServiceListener(MessageDelegate1<ServiceListener, const ServiceEvent&>(&sListen, &ServiceListener::serviceChanged));
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
      bundle->Start();
    }
    catch (const std::exception& e)
    {
      teststatus  = false;
      US_TEST_FAILED_MSG( << "Failed to load bundle, got exception: "
                          << e.what() << " + in " << name << ":FAIL" );
    }

    // Stop the test target to get a service unpublished.
    try
    {
      bundle->Stop();
    }
    catch (const std::exception& e)
    {
      teststatus  = false;
      US_TEST_FAILED_MSG( << "Failed to unload bundle, got exception: "
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
      context->RemoveServiceListener(&sListen, &TestServiceListener::serviceChanged);
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

void frameSL02a(const std::shared_ptr<Framework>& framework)
{
  BundleContext* context = framework->GetBundleContext();

  TestServiceListener listener1(context);
  TestServiceListener listener2(context);

  try
  {
    context->RemoveServiceListener(&listener1, &TestServiceListener::serviceChanged);
    context->AddServiceListener(&listener1, &TestServiceListener::serviceChanged);
    context->RemoveServiceListener(&listener2, &TestServiceListener::serviceChanged);
    context->AddServiceListener(&listener2, &TestServiceListener::serviceChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_FAILED_MSG( << "service listener registration failed " << ise.what()
                        << " : frameSL02a:FAIL" );
  }

  auto bundle = InstallTestBundle(context, "TestBundleA");
  bundle->Start();

  std::vector<ServiceEvent::Type> events;
  events.push_back(ServiceEvent::REGISTERED);

  US_TEST_CONDITION(listener1.checkEvents(events), "Check first service listener")
  US_TEST_CONDITION(listener2.checkEvents(events), "Check second service listener")

  context->RemoveServiceListener(&listener1, &TestServiceListener::serviceChanged);
  context->RemoveServiceListener(&listener2, &TestServiceListener::serviceChanged);

  bundle->Stop();
}

void frameSL05a(const std::shared_ptr<Framework>& framework)
{
  std::vector<ServiceEvent::Type> events;
  events.push_back(ServiceEvent::REGISTERED);
  events.push_back(ServiceEvent::UNREGISTERING);

  auto bundle = InstallTestBundle(framework->GetBundleContext(), "TestBundleA");

  bool testStatus = runStartStopTest("FrameSL05a", 1, bundle, framework->GetBundleContext(), events);
  US_TEST_CONDITION(testStatus, "FrameSL05a")
}

void frameSL10a(const std::shared_ptr<Framework>& framework)
{
  std::vector<ServiceEvent::Type> events;
  events.push_back(ServiceEvent::REGISTERED);
  events.push_back(ServiceEvent::UNREGISTERING);

  auto bundle = InstallTestBundle(framework->GetBundleContext(), "TestBundleA2");

  bool testStatus = runStartStopTest("FrameSL10a", 1, bundle, framework->GetBundleContext(), events);
  US_TEST_CONDITION(testStatus, "FrameSL10a")
}

void frameSL25a(const std::shared_ptr<Framework>& framework)
{
  BundleContext* context = framework->GetBundleContext();

  TestServiceListener sListen(context, false);
  try
  {
    context->AddServiceListener(&sListen, &TestServiceListener::serviceChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_OUTPUT( << "service listener registration failed " << ise.what() );
    throw;
  }

  auto libSL1 = InstallTestBundle(context, "TestBundleSL1");
  auto libSL3 = InstallTestBundle(context, "TestBundleSL3");
  auto libSL4 = InstallTestBundle(context, "TestBundleSL4");

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

  // Start libBundleTestSL1 to ensure that the Service interface is available.
  try
  {
    US_TEST_OUTPUT( << "Starting libBundleTestSL1: " << libSL1->GetLocation() );
    libSL1->Start();
  }
  catch (const std::exception& e)
  {
    US_TEST_OUTPUT( << "Failed to load bundle, got exception: " << e.what() );
    throw;
  }

  // Start libBundleTestSL4 that will require the serivce interface and publish
  // us::FooService
  try
  {
    US_TEST_OUTPUT( << "Starting libBundleTestSL4: " << libSL4->GetLocation() );
    libSL4->Start();
  }
  catch (const std::exception& e)
  {
    US_TEST_OUTPUT( << "Failed to load bundle, got exception: " << e.what() );
    throw;
  }

  // Start libBundleTestSL3 that will require the serivce interface and get the service
  try
  {
    US_TEST_OUTPUT( << "Starting libBundleTestSL3: " << libSL3->GetLocation() );
    libSL3->Start();
  }
  catch (const std::exception& e)
  {
    US_TEST_OUTPUT( << "Failed to load bundle, got exception: " << e.what() );
    throw;
  }

  // Check that libSL3 has been notified about the FooService.
  US_TEST_OUTPUT( << "Check that FooService is added to service tracker in libSL3" );
  try
  {
    ServiceReferenceU libSL3SR = context->GetServiceReference("ActivatorSL3");
    InterfaceMapConstPtr libSL3Activator = context->GetService(libSL3SR);
    US_TEST_CONDITION_REQUIRED(libSL3Activator && !libSL3Activator->empty(), "ActivatorSL3 service != 0");

    ServiceReference<BundlePropsInterface> libSL3PropsI(libSL3SR);
    std::shared_ptr<BundlePropsInterface> propsInterface = context->GetService(libSL3PropsI);
    US_TEST_CONDITION_REQUIRED(propsInterface, "BundlePropsInterface != 0");

    BundlePropsInterface::Properties::const_iterator i = propsInterface->GetProperties().find("serviceAdded");
    US_TEST_CONDITION_REQUIRED(i != propsInterface->GetProperties().end(), "Property serviceAdded");
    Any serviceAddedField3 = i->second;
    US_TEST_CONDITION_REQUIRED(!serviceAddedField3.Empty() && any_cast<bool>(serviceAddedField3),
                                 "libSL3 notified about presence of FooService");
  }
  catch (const ServiceException& se)
  {
    US_TEST_FAILED_MSG( << "Failed to get service reference:" << se );
  }

  // Check that libSL1 has been notified about the FooService.
  US_TEST_OUTPUT( << "Check that FooService is added to service tracker in libSL1" );
  try
  {
    ServiceReferenceU libSL1SR = context->GetServiceReference("ActivatorSL1");
    InterfaceMapConstPtr libSL1Activator = context->GetService(libSL1SR);
    US_TEST_CONDITION_REQUIRED(libSL1Activator && !libSL1Activator->empty(), "ActivatorSL1 service != 0");

    ServiceReference<BundlePropsInterface> libSL1PropsI(libSL1SR);
    std::shared_ptr<BundlePropsInterface> propsInterface = context->GetService(libSL1PropsI);
    US_TEST_CONDITION_REQUIRED(propsInterface, "Cast to BundlePropsInterface");

    BundlePropsInterface::Properties::const_iterator i = propsInterface->GetProperties().find("serviceAdded");
    US_TEST_CONDITION_REQUIRED(i != propsInterface->GetProperties().end(), "Property serviceAdded");
    Any serviceAddedField1 = i->second;
    US_TEST_CONDITION_REQUIRED(!serviceAddedField1.Empty() && any_cast<bool>(serviceAddedField1),
                                 "libSL1 notified about presence of FooService");
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
    US_TEST_OUTPUT( << "Failed to unload bundle, got exception:" << e.what() );
    throw;
  }

  // Check that libSL3 has been notified about the removal of FooService.
  US_TEST_OUTPUT( << "Check that FooService is removed from service tracker in libSL3" );
  try
  {
    ServiceReferenceU libSL3SR = context->GetServiceReference("ActivatorSL3");
    InterfaceMapConstPtr libSL3Activator = context->GetService(libSL3SR);
    US_TEST_CONDITION_REQUIRED(libSL3Activator && !libSL3Activator->empty(), "ActivatorSL3 service != 0");

    ServiceReference<BundlePropsInterface> libSL3PropsI(libSL3SR);
    std::shared_ptr<BundlePropsInterface> propsInterface = context->GetService(libSL3PropsI);
    US_TEST_CONDITION_REQUIRED(propsInterface, "Cast to BundlePropsInterface");

    BundlePropsInterface::Properties::const_iterator i = propsInterface->GetProperties().find("serviceRemoved");
    US_TEST_CONDITION_REQUIRED(i != propsInterface->GetProperties().end(), "Property serviceRemoved");

    Any serviceRemovedField3 = i->second;
    US_TEST_CONDITION(!serviceRemovedField3.Empty() && any_cast<bool>(serviceRemovedField3),
                        "libSL3 notified about removal of FooService");
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
    US_TEST_OUTPUT( << "Failed to unload bundle got exception" << e.what() );
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
    US_TEST_OUTPUT( << "Failed to unload bundle got exception" << e.what() );
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
    context->RemoveServiceListener(&sListen, &TestServiceListener::serviceChanged);
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
  std::shared_ptr<Framework> framework = factory.NewFramework(std::map<std::string, std::string>());
  framework->Start();

  frameSL02a(framework);
  frameSL05a(framework);
  frameSL10a(framework);
  frameSL25a(framework);

  US_TEST_END()
}
