/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See USCopyright.txt or http://www.us.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <usConfig.h>

#include <usTestingMacros.h>

#include <usModule.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usServiceInterface.h>
#include <usServiceTracker.h>

#include US_BASECLASS_HEADER

#include "usServiceControlInterface.h"
#include "usTestUtilSharedLibrary.cpp"

US_USE_NAMESPACE

extern ModuleActivator* _us_module_activator_instance_TestModuleS();

int usServiceTrackerTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceTrackerTest")

  ModuleContext* mc = GetModuleContext();
  SharedLibraryHandle libS("TestModuleS"
                             #ifndef US_BUILD_SHARED_LIBS
                               , _us_module_activator_instance_TestModuleS
                             #endif
                               );

  // Start the test target to get a service published.
  try
  {
    libS.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG( << "Failed to load module, got exception: " << e.what() );
  }

  // 1. Create a ServiceTracker with ServiceTrackerCustomizer == null

  std::string s1("org.cppmicroservices.TestModuleSService");
  ServiceReference servref = mc->GetServiceReference(s1 + "0");

  US_TEST_CONDITION_REQUIRED(servref != 0, "Test if registered service of id org.cppmicroservices.TestModuleSService0");

  ServiceControlInterface* serviceController = mc->GetService<ServiceControlInterface>(servref);
  US_TEST_CONDITION_REQUIRED(serviceController != 0, "Test valid service controller");

  ServiceTracker<>* st1 = new ServiceTracker<>(mc, servref);

  // 2. Check the size method with an unopened service tracker

  US_TEST_CONDITION_REQUIRED(st1->Size() == 0, "Test if size == 0");

  // 3. Open the service tracker and see what it finds,
  // expect to find one instance of the implementation,
  // "org.cppmicroservices.TestModuleSService0"

  st1->Open();
  std::string expName  = "TestModuleS";
  std::list<ServiceReference> sa2;
  st1->GetServiceReferences(sa2);

  US_TEST_CONDITION_REQUIRED(sa2.size() == 1, "Checking ServiceTracker size");
  std::string name(us_service_impl_name(mc->GetService(sa2.front())));
  US_TEST_CONDITION_REQUIRED(name == expName, "Checking service implementation name");

  // 5. Close this service tracker
  st1->Close();

  // 6. Check the size method, now when the servicetracker is closed
  US_TEST_CONDITION_REQUIRED(st1->Size() == 0, "Checking ServiceTracker size");

  // 7. Check if we still track anything , we should get null
  sa2.clear();
  st1->GetServiceReferences(sa2);
  US_TEST_CONDITION_REQUIRED(sa2.empty(), "Checking ServiceTracker size");

  // 8. A new Servicetracker, this time with a filter for the object
  std::string fs = std::string("(") + ServiceConstants::OBJECTCLASS() + "=" + s1 + "*" + ")";
  LDAPFilter f1(fs);
  delete st1;
  st1 = new ServiceTracker<>(mc, f1);
  // add a service
  serviceController->ServiceControl(1, "register", 7);

  // 9. Open the service tracker and see what it finds,
  // expect to find two instances of references to
  // "org.cppmicroservices.TestModuleSService*"
  // i.e. they refer to the same piece of code

  st1->Open();
  sa2.clear();
  st1->GetServiceReferences(sa2);
  US_TEST_CONDITION_REQUIRED(sa2.size() == 2, "Checking service reference count");
  for (std::list<ServiceReference>::const_iterator i = sa2.begin();  i != sa2.end(); ++i)
  {
    std::string name(mc->GetService(*i)->GetNameOfClass());
    US_TEST_CONDITION_REQUIRED(name == expName, "Check for expected class name");
  }

  // 10. Get libTestModuleS to register one more service and see if it appears
  serviceController->ServiceControl(2, "register", 1);
  sa2.clear();
  st1->GetServiceReferences(sa2);
  US_TEST_CONDITION_REQUIRED(sa2.size() == 3, "Checking service reference count");
  for (std::list<ServiceReference>::const_iterator i = sa2.begin(); i != sa2.end(); ++i)
  {
    std::string name(mc->GetService(*i)->GetNameOfClass());
    US_TEST_CONDITION_REQUIRED(name == expName, "Check for expected class name");
  }

  // 11. Get libTestModuleS to register one more service and see if it appears
  serviceController->ServiceControl(3, "register", 2);
  sa2.clear();
  st1->GetServiceReferences(sa2);
  US_TEST_CONDITION_REQUIRED(sa2.size() == 4, "Checking service reference count");
  for (std::list<ServiceReference>::const_iterator i = sa2.begin(); i != sa2.end(); ++i)
  {
    std::string name = mc->GetService(*i)->GetNameOfClass();
    US_TEST_CONDITION_REQUIRED(name == expName, "Check for expected class name");
  }

  // 12. Get libTestModuleS to unregister one service and see if it disappears
  serviceController->ServiceControl(3, "unregister", 0);
  sa2.clear();
  st1->GetServiceReferences(sa2);
  US_TEST_CONDITION_REQUIRED(sa2.size() == 3, "Checking service reference count");
  for (std::list<ServiceReference>::const_iterator i = sa2.begin(); i != sa2.end(); ++i)
  {
    std::string name = mc->GetService(*i)->GetNameOfClass();
    US_TEST_CONDITION_REQUIRED(name == expName, "Check for expected class name");
  }

  // 13. Get the highest ranking service reference, it should have ranking 7
  ServiceReference h1 = st1->GetServiceReference();
  int rank = any_cast<int>(h1.GetProperty(ServiceConstants::SERVICE_RANKING()));
  US_TEST_CONDITION_REQUIRED(rank == 7, "Check service rank");

  // 14. Get the service of the highest ranked service reference

  US_BASECLASS_NAME* o1 = st1->GetService(h1);
  US_TEST_CONDITION_REQUIRED(o1 != 0, "Check for non-null service");

  // 14a Get the highest ranked service, directly this time
  US_BASECLASS_NAME* o3 = st1->GetService();
  US_TEST_CONDITION_REQUIRED(o3 != 0, "Check for non-null service");
  US_TEST_CONDITION_REQUIRED(o1 == o3, "Check for equal service instances");

  // 15. Now release the tracking of that service and then try to get it
  //     from the servicetracker, which should yield a null object
  serviceController->ServiceControl(1, "unregister", 7);
  US_BASECLASS_NAME* o2 = st1->GetService(h1);
  US_TEST_CONDITION_REQUIRED(o2 == 0, "Checkt that service is null");

  // 16. Get all service objects this tracker tracks, it should be 2
  std::list<US_BASECLASS_NAME*> ts1;
  st1->GetServices(ts1);
  US_TEST_CONDITION_REQUIRED(ts1.size() == 2, "Check service count");

  // 17. Test the remove method.
  //     First register another service, then remove it being tracked
  serviceController->ServiceControl(1, "register", 7);
  h1 = st1->GetServiceReference();
  std::list<ServiceReference> sa3;
  st1->GetServiceReferences(sa3);
  US_TEST_CONDITION_REQUIRED(sa3.size() == 3, "Check service reference count");
  for (std::list<ServiceReference>::const_iterator i = sa3.begin(); i != sa3.end(); ++i)
  {
    std::string name = mc->GetService(*i)->GetNameOfClass();
    US_TEST_CONDITION_REQUIRED(name == expName, "Checking for expected class name");
  }

  st1->Remove(h1);           // remove tracking on one servref
  sa2.clear();
  st1->GetServiceReferences(sa2);
  US_TEST_CONDITION_REQUIRED(sa2.size() == 2, "Check service reference count");

  // 18. Test the addingService method,add a service reference

  // 19. Test the removedService method, remove a service reference


  // 20. Test the waitForService method
  US_BASECLASS_NAME* o9 = st1->WaitForService(50);
  US_TEST_CONDITION_REQUIRED(o9 != 0, "Checking WaitForService method");

  US_TEST_END()
}
