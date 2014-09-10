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

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModule.h>
#include <usServiceFactory.h>

#include "usTestingMacros.h"

struct Interface1 {};
struct Interface2 {};
struct Interface3 {};

struct MyService1 : public Interface1
{};

struct MyService2 : public Interface1, public Interface2
{};

struct MyService3 : public Interface1, public Interface2, public Interface3
{};

struct MyFactory1 : public us::ServiceFactory
{
  std::map<long, MyService1*> m_idToServiceMap;

  virtual us::InterfaceMap GetService(us::Module* module, const us::ServiceRegistrationBase& /*registration*/)
  {
    MyService1* s = new MyService1;
    m_idToServiceMap.insert(std::make_pair(module->GetModuleId(), s));
    return us::MakeInterfaceMap<Interface1>(s);
  }

  virtual void UngetService(us::Module* module, const us::ServiceRegistrationBase& /*registration*/,
                            const us::InterfaceMap& service)
  {
    std::map<long, MyService1*>::iterator iter = m_idToServiceMap.find(module->GetModuleId());
    if (iter != m_idToServiceMap.end())
    {
      US_TEST_CONDITION(static_cast<Interface1*>(iter->second) == us::ExtractInterface<Interface1>(service), "Compare service pointer")
      delete iter->second;
      m_idToServiceMap.erase(iter);
    }
  }
};

struct MyFactory2 : public us::ServiceFactory
{
  std::map<long, MyService2*> m_idToServiceMap;

  virtual us::InterfaceMap GetService(us::Module* module, const us::ServiceRegistrationBase& /*registration*/)
  {
    MyService2* s = new MyService2;
    m_idToServiceMap.insert(std::make_pair(module->GetModuleId(), s));
    return us::MakeInterfaceMap<Interface1,Interface2>(s);
  }

  virtual void UngetService(us::Module* module, const us::ServiceRegistrationBase& /*registration*/,
                            const us::InterfaceMap& service)
  {
    std::map<long, MyService2*>::iterator iter = m_idToServiceMap.find(module->GetModuleId());
    if (iter != m_idToServiceMap.end())
    {
      US_TEST_CONDITION(static_cast<Interface2*>(iter->second) == us::ExtractInterface<Interface2>(service), "Compare service pointer")
      delete iter->second;
      m_idToServiceMap.erase(iter);
    }
  }
};

struct MyFactory3 : public us::ServiceFactory
{
  std::map<long, MyService3*> m_idToServiceMap;

  virtual us::InterfaceMap GetService(us::Module* module, const us::ServiceRegistrationBase& /*registration*/)
  {
    MyService3* s = new MyService3;
    m_idToServiceMap.insert(std::make_pair(module->GetModuleId(), s));
    return us::MakeInterfaceMap<Interface1,Interface2,Interface3>(s);
  }

  virtual void UngetService(us::Module* module, const us::ServiceRegistrationBase& /*registration*/,
                            const us::InterfaceMap& service)
  {
    std::map<long, MyService3*>::iterator iter = m_idToServiceMap.find(module->GetModuleId());
    if (iter != m_idToServiceMap.end())
    {
      US_TEST_CONDITION(static_cast<Interface3*>(iter->second) == us::ExtractInterface<Interface3>(service), "Compare service pointer")
      delete iter->second;
      m_idToServiceMap.erase(iter);
    }
  }
};


US_USE_NAMESPACE

int usServiceTemplateTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceTemplateTest");

  ModuleContext* mc = GetModuleContext();

  // Register compile tests
  MyService1 s1;
  MyService2 s2;
  MyService3 s3;

  us::ServiceRegistration<Interface1> sr1 = mc->RegisterService<Interface1>(&s1);
  us::ServiceRegistration<Interface1,Interface2> sr2 = mc->RegisterService<Interface1,Interface2>(&s2);
  us::ServiceRegistration<Interface1,Interface2,Interface3> sr3 = mc->RegisterService<Interface1,Interface2,Interface3>(&s3);

  MyFactory1 f1;
  us::ServiceRegistration<Interface1> sfr1 = mc->RegisterService<Interface1>(&f1);

  MyFactory2 f2;
  us::ServiceRegistration<Interface1,Interface2> sfr2 = mc->RegisterService<Interface1,Interface2>(static_cast<ServiceFactory*>(&f2));

  MyFactory3 f3;
  us::ServiceRegistration<Interface1,Interface2,Interface3> sfr3 = mc->RegisterService<Interface1,Interface2,Interface3>(static_cast<ServiceFactory*>(&f3));

#ifdef US_BUILD_SHARED_LIBS
  US_TEST_CONDITION(mc->GetModule()->GetRegisteredServices().size() == 6, "# of reg services")
#endif

  std::vector<us::ServiceReference<Interface1> > s1refs = mc->GetServiceReferences<Interface1>();
  US_TEST_CONDITION(s1refs.size() == 6, "# of interface1 regs")
  std::vector<us::ServiceReference<Interface2> > s2refs = mc->GetServiceReferences<Interface2>();
  US_TEST_CONDITION(s2refs.size() == 4, "# of interface2 regs")
  std::vector<us::ServiceReference<Interface3> > s3refs = mc->GetServiceReferences<Interface3>();
  US_TEST_CONDITION(s3refs.size() == 2, "# of interface3 regs")

  Interface1* i1 = mc->GetService(sr1.GetReference());
  US_TEST_CONDITION(i1 == static_cast<Interface1*>(&s1), "interface1 ptr")
  i1 = NULL;
  US_TEST_CONDITION(mc->UngetService(sr1.GetReference()), "unget interface1 ptr")
  i1 = mc->GetService(sfr1.GetReference());
  US_TEST_CONDITION(i1 == static_cast<Interface1*>(f1.m_idToServiceMap[mc->GetModule()->GetModuleId()]), "interface1 factory ptr")
  i1 = NULL;
  US_TEST_CONDITION(mc->UngetService(sfr1.GetReference()), "unget interface1 factory ptr")

  i1 = mc->GetService(sr2.GetReference(InterfaceType<Interface1>()));
  US_TEST_CONDITION(i1 == static_cast<Interface1*>(&s2), "interface1 ptr")
  i1 = NULL;
  US_TEST_CONDITION(mc->UngetService(sr2.GetReference(InterfaceType<Interface1>())), "unget interface1 ptr")
  i1 = mc->GetService(sfr2.GetReference(InterfaceType<Interface1>()));
  US_TEST_CONDITION(i1 == static_cast<Interface1*>(f2.m_idToServiceMap[mc->GetModule()->GetModuleId()]), "interface1 factory ptr")
  i1 = NULL;
  US_TEST_CONDITION(mc->UngetService(sfr2.GetReference(InterfaceType<Interface1>())), "unget interface1 factory ptr")
  Interface2* i2 = mc->GetService(sr2.GetReference(InterfaceType<Interface2>()));
  US_TEST_CONDITION(i2 == static_cast<Interface2*>(&s2), "interface2 ptr")
  i2 = NULL;
  US_TEST_CONDITION(mc->UngetService(sr2.GetReference(InterfaceType<Interface2>())), "unget interface2 ptr")
  i2 = mc->GetService(sfr2.GetReference(InterfaceType<Interface2>()));
  US_TEST_CONDITION(i2 == static_cast<Interface2*>(f2.m_idToServiceMap[mc->GetModule()->GetModuleId()]), "interface2 factory ptr")
  i2 = NULL;
  US_TEST_CONDITION(mc->UngetService(sfr2.GetReference(InterfaceType<Interface2>())), "unget interface2 factory ptr")

  i1 = mc->GetService(sr3.GetReference(InterfaceType<Interface1>()));
  US_TEST_CONDITION(i1 == static_cast<Interface1*>(&s3), "interface1 ptr")
  i1 = NULL;
  US_TEST_CONDITION(mc->UngetService(sr3.GetReference(InterfaceType<Interface1>())), "unget interface1 ptr")
  i1 = mc->GetService(sfr3.GetReference(InterfaceType<Interface1>()));
  US_TEST_CONDITION(i1 == static_cast<Interface1*>(f3.m_idToServiceMap[mc->GetModule()->GetModuleId()]), "interface1 factory ptr")
  i1 = NULL;
  US_TEST_CONDITION(mc->UngetService(sfr3.GetReference(InterfaceType<Interface1>())), "unget interface1 factory ptr")
  i2 = mc->GetService(sr3.GetReference(InterfaceType<Interface2>()));
  US_TEST_CONDITION(i2 == static_cast<Interface2*>(&s3), "interface2 ptr")
  i2 = NULL;
  US_TEST_CONDITION(mc->UngetService(sr3.GetReference(InterfaceType<Interface2>())), "unget interface2 ptr")
  i2 = mc->GetService(sfr3.GetReference(InterfaceType<Interface2>()));
  US_TEST_CONDITION(i2 == static_cast<Interface2*>(f3.m_idToServiceMap[mc->GetModule()->GetModuleId()]), "interface2 factory ptr")
  i2 = NULL;
  US_TEST_CONDITION(mc->UngetService(sfr3.GetReference(InterfaceType<Interface2>())), "unget interface2 factory ptr")
  Interface3* i3 = mc->GetService(sr3.GetReference(InterfaceType<Interface3>()));
  US_TEST_CONDITION(i3 == static_cast<Interface3*>(&s3), "interface3 ptr")
  i3 = NULL;
  US_TEST_CONDITION(mc->UngetService(sr3.GetReference(InterfaceType<Interface3>())), "unget interface3 ptr")
  i3 = mc->GetService(sfr3.GetReference(InterfaceType<Interface3>()));
  US_TEST_CONDITION(i3 == static_cast<Interface3*>(f3.m_idToServiceMap[mc->GetModule()->GetModuleId()]), "interface3 factory ptr")
  i3 = NULL;
  US_TEST_CONDITION(mc->UngetService(sfr3.GetReference(InterfaceType<Interface3>())), "unget interface3 factory ptr")

  sr1.Unregister();
  sr2.Unregister();
  sr3.Unregister();

  US_TEST_END()
}
