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
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usLDAPProp.h>
#include <usServiceFindHook.h>
#include <usServiceEventListenerHook.h>
#include <usServiceListenerHook.h>
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

class TestServiceListener
{
public:

  void ServiceChanged(const ServiceEvent serviceEvent)
  {
    this->events.push_back(serviceEvent);
  }

  std::vector<ServiceEvent> events;
};

class TestServiceEventListenerHook : public ServiceEventListenerHook
{
private:

  int id;

public:

  TestServiceEventListenerHook(int id)
  : id(id)
  {
  }

  typedef ShrinkableMap<ModuleContext*, ShrinkableVector<ServiceListenerHook::ListenerInfo> > MapType;

  void Event(const ServiceEvent& /*event*/, MapType& listeners)
  {
    US_TEST_CONDITION_REQUIRED(listeners.size() > 0 && listeners.find(GetModuleContext()) != listeners.end(), "Check listener content");
    ShrinkableVector<ServiceListenerHook::ListenerInfo>& listenerInfos = listeners[GetModuleContext()];

    // listener count should be 2 because the event listener hooks are called with
    // the list of listeners before filtering them according to ther LDAP filter
    if (id == 1)
    {
#ifdef US_BUILD_SHARED_LIBS
      US_TEST_CONDITION(listenerInfos.size() == 2, "2 service listeners expected");
#else
      US_TEST_CONDITION(listenerInfos.size() >= 2, "2 service listeners expected");
#endif
      US_TEST_CONDITION(listenerInfos[0].IsRemoved() == false, "Listener is not removed");
      US_TEST_CONDITION(listenerInfos[1].IsRemoved() == false, "Listener is not removed");
      US_TEST_CONDITION(!(listenerInfos[0] == listenerInfos[1]), "listener info inequality");
    }
    else
    {
      // there is already one listener filtered out
#ifdef US_BUILD_SHARED_LIBS
      US_TEST_CONDITION(listenerInfos.size() == 1, "1 service listener expected");
#else
      US_TEST_CONDITION(listenerInfos.size() >= 1, "1 service listener expected");
#endif
      US_TEST_CONDITION(listenerInfos[0].IsRemoved() == false, "Listener is not removed");
    }
    if (listenerInfo.IsNull())
    {
      listenerInfo = listenerInfos[0];
    }
    else
    {
      US_TEST_CONDITION(listenerInfo == listenerInfos[0], "Equal listener info objects");
    }

    // Remove the listener without a filter from the list
    for(ShrinkableVector<ServiceListenerHook::ListenerInfo>::iterator infoIter = listenerInfos.begin();
        infoIter != listenerInfos.end();)
    {
      if (infoIter->GetFilter().empty())
      {
        infoIter = listenerInfos.erase(infoIter);
      }
      else
      {
        ++infoIter;
      }
    }
#ifdef US_BUILD_SHARED_LIBS
    US_TEST_CONDITION(listenerInfos.size() == 1, "One listener with LDAP filter should remain");
#else
    US_TEST_CONDITION(listenerInfos.size() >= 1, "One listener with LDAP filter should remain");
#endif

    ordering.push_back(id);
  }

  ServiceListenerHook::ListenerInfo listenerInfo;

  static std::vector<int> ordering;
};

std::vector<int> TestServiceEventListenerHook::ordering;


class TestServiceFindHook : public ServiceFindHook
{
private:
  int id;

public:

  TestServiceFindHook(int id)
    : id(id)
  {
  }

  void Find(const ModuleContext* context, const std::string& /*name*/,
            const std::string& /*filter*/, ShrinkableVector<ServiceReferenceBase>& references)
  {
    US_TEST_CONDITION(context == GetModuleContext(), "Module context");

    references.clear();
    ordering.push_back(id);
  }

  static std::vector<int> ordering;
};

std::vector<int> TestServiceFindHook::ordering;

class TestServiceListenerHook : public ServiceListenerHook
{
private:
  int id;

public:

  TestServiceListenerHook(int id)
    : id(id)
  {
  }

  void Added(const std::vector<ListenerInfo>& listeners)
  {
    for (std::vector<ListenerInfo>::const_iterator iter = listeners.begin();
         iter != listeners.end(); ++iter)
    {
      if (iter->IsRemoved() || iter->GetModuleContext() != GetModuleContext()) continue;
      listenerInfos.insert(*iter);
      lastAdded = listeners.back();
      ordering.push_back(id);
    }
  }

  void Removed(const std::vector<ListenerInfo>& listeners)
  {
    for (std::vector<ListenerInfo>::const_iterator iter = listeners.begin();
         iter != listeners.end(); ++iter)
    {
      listenerInfos.erase(*iter);
      ordering.push_back(id*10);
    }
    lastRemoved = listeners.back();
  }

  static std::vector<int> ordering;

  US_UNORDERED_SET_TYPE<ListenerInfo> listenerInfos;
  ListenerInfo lastAdded;
  ListenerInfo lastRemoved;
};

std::vector<int> TestServiceListenerHook::ordering;


void TestEventListenerHook()
{
  ModuleContext* context = GetModuleContext();

  TestServiceListener serviceListener1;
  TestServiceListener serviceListener2;
  context->AddServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);
  context->AddServiceListener(&serviceListener2, &TestServiceListener::ServiceChanged, LDAPProp(ServiceConstants::OBJECTCLASS()) == "bla");

  TestServiceEventListenerHook serviceEventListenerHook1(1);
  ServiceProperties hookProps1;
  hookProps1[ServiceConstants::SERVICE_RANKING()] = 10;
  ServiceRegistration<ServiceEventListenerHook> eventListenerHookReg1 =
      context->RegisterService<ServiceEventListenerHook>(&serviceEventListenerHook1, hookProps1);

  TestServiceEventListenerHook serviceEventListenerHook2(2);
  ServiceProperties hookProps2;
  hookProps2[ServiceConstants::SERVICE_RANKING()] = 0;
  ServiceRegistration<ServiceEventListenerHook> eventListenerHookReg2 =
      context->RegisterService<ServiceEventListenerHook>(&serviceEventListenerHook2, hookProps2);

  std::vector<int> expectedOrdering;
  expectedOrdering.push_back(1);
  expectedOrdering.push_back(1);
  expectedOrdering.push_back(2);
  US_TEST_CONDITION(serviceEventListenerHook1.ordering == expectedOrdering, "Event listener hook call order");

  US_TEST_CONDITION(serviceListener1.events.empty(), "service event of service event listener hook");
  US_TEST_CONDITION(serviceListener2.events.empty(), "no service event for filtered listener");

#ifdef US_BUILD_SHARED_LIBS
  SharedLibrary libA(LIB_PATH, "TestModuleA");
  try
  {
    libA.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

  expectedOrdering.push_back(1);
  expectedOrdering.push_back(2);
  US_TEST_CONDITION(serviceEventListenerHook1.ordering == expectedOrdering, "Event listener hook call order");

  libA.Unload();
#endif

  US_TEST_CONDITION(serviceListener1.events.empty(), "no service event due to service event listener hook");
  US_TEST_CONDITION(serviceListener2.events.empty(), "no service event for filtered listener due to service event listener hook");

  eventListenerHookReg2.Unregister();
  eventListenerHookReg1.Unregister();

  context->RemoveServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);
  context->RemoveServiceListener(&serviceListener2, &TestServiceListener::ServiceChanged);
}

void TestListenerHook()
{
  ModuleContext* context = GetModuleContext();

  TestServiceListener serviceListener1;
  TestServiceListener serviceListener2;
  context->AddServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);
  context->AddServiceListener(&serviceListener2, &TestServiceListener::ServiceChanged, LDAPProp(ServiceConstants::OBJECTCLASS()) == "bla");

  TestServiceListenerHook serviceListenerHook1(1);
  ServiceProperties hookProps1;
  hookProps1[ServiceConstants::SERVICE_RANKING()] = 0;
  ServiceRegistration<ServiceListenerHook> listenerHookReg1 =
      context->RegisterService<ServiceListenerHook>(&serviceListenerHook1, hookProps1);

  TestServiceListenerHook serviceListenerHook2(2);
  ServiceProperties hookProps2;
  hookProps2[ServiceConstants::SERVICE_RANKING()] = 10;
  ServiceRegistration<ServiceListenerHook> listenerHookReg2 =
      context->RegisterService<ServiceListenerHook>(&serviceListenerHook2, hookProps2);

#ifdef US_BUILD_SHARED_LIBS
  // check if hooks got notified about the existing listeners
  US_TEST_CONDITION_REQUIRED(serviceListenerHook1.listenerInfos.size() == 2, "Notification about existing listeners")
#endif
  const std::size_t listenerInfoSizeOld = serviceListenerHook1.listenerInfos.size() - 2;

  context->AddServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);
  ServiceListenerHook::ListenerInfo lastAdded = serviceListenerHook1.lastAdded;

#ifdef US_BUILD_SHARED_LIBS
  std::vector<int> expectedOrdering;
  expectedOrdering.push_back(1);
  expectedOrdering.push_back(1);
  expectedOrdering.push_back(2);
  expectedOrdering.push_back(2);
  expectedOrdering.push_back(20);
  expectedOrdering.push_back(10);
  expectedOrdering.push_back(2);
  expectedOrdering.push_back(1);
  US_TEST_CONDITION(serviceListenerHook1.ordering == expectedOrdering, "Listener hook call order");
#endif

  context->AddServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged, LDAPProp(ServiceConstants::OBJECTCLASS()) == "blub");
  US_TEST_CONDITION(lastAdded == serviceListenerHook1.lastRemoved, "Same ListenerInfo object)");
  US_TEST_CONDITION(!(lastAdded == serviceListenerHook1.lastAdded), "New ListenerInfo object)");

#ifdef US_BUILD_SHARED_LIBS
  expectedOrdering.push_back(20);
  expectedOrdering.push_back(10);
  expectedOrdering.push_back(2);
  expectedOrdering.push_back(1);
  US_TEST_CONDITION(serviceListenerHook1.ordering == expectedOrdering, "Listener hook call order");
#endif

  context->RemoveServiceListener(&serviceListener1, &TestServiceListener::ServiceChanged);
  context->RemoveServiceListener(&serviceListener2, &TestServiceListener::ServiceChanged);

#ifdef US_BUILD_SHARED_LIBS
  expectedOrdering.push_back(20);
  expectedOrdering.push_back(10);
  expectedOrdering.push_back(20);
  expectedOrdering.push_back(10);
  US_TEST_CONDITION(serviceListenerHook1.ordering == expectedOrdering, "Listener hook call order");
#endif

  US_TEST_CONDITION_REQUIRED(serviceListenerHook1.listenerInfos.size() == listenerInfoSizeOld, "Removed listener infos")

  listenerHookReg2.Unregister();
  listenerHookReg1.Unregister();
}

void TestFindHook()
{
  ModuleContext* context = GetModuleContext();

  TestServiceFindHook serviceFindHook1(1);
  ServiceProperties hookProps1;
  hookProps1[ServiceConstants::SERVICE_RANKING()] = 0;
  ServiceRegistration<ServiceFindHook> findHookReg1 =
      context->RegisterService<ServiceFindHook>(&serviceFindHook1, hookProps1);

  TestServiceFindHook serviceFindHook2(2);
  ServiceProperties hookProps2;
  hookProps2[ServiceConstants::SERVICE_RANKING()] = 10;
  ServiceRegistration<ServiceFindHook> findHookReg2 =
      context->RegisterService<ServiceFindHook>(&serviceFindHook2, hookProps2);

  std::vector<int> expectedOrdering;
  US_TEST_CONDITION(serviceFindHook1.ordering == expectedOrdering, "Find hook call order");

  TestServiceListener serviceListener;
  context->AddServiceListener(&serviceListener, &TestServiceListener::ServiceChanged);

#ifdef US_BUILD_SHARED_LIBS
  SharedLibrary libA(LIB_PATH, "TestModuleA");
  try
  {
    libA.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

  US_TEST_CONDITION(serviceListener.events.size() == 1, "Service registered");
#endif

  std::vector<ServiceReferenceU> refs = context->GetServiceReferences("us::TestModuleAService");
  US_TEST_CONDITION(refs.empty(), "Empty references");
  ServiceReferenceU ref = context->GetServiceReference("us::TestModuleAService");
  US_TEST_CONDITION(!ref, "Invalid reference (filtered out)");

  expectedOrdering.push_back(2);
  expectedOrdering.push_back(1);
  expectedOrdering.push_back(2);
  expectedOrdering.push_back(1);

  US_TEST_CONDITION(serviceFindHook1.ordering == expectedOrdering, "Find hook call order");

  findHookReg2.Unregister();
  findHookReg1.Unregister();

  refs = context->GetServiceReferences("us::TestModuleAService");
  US_TEST_CONDITION(!refs.empty(), "Non-empty references");
  ref = context->GetServiceReference("us::TestModuleAService");
  US_TEST_CONDITION(ref, "Valid reference");

#ifdef US_BUILD_SHARED_LIBS
  libA.Unload();
#endif

  context->RemoveServiceListener(&serviceListener, &TestServiceListener::ServiceChanged);
}

} // end unnamed namespace

int usServiceHooksTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceHooksTest");

  TestListenerHook();
  TestFindHook();
  TestEventListenerHook();

  US_TEST_END()
}
