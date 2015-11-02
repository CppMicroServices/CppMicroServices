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

#include <usGetBundleContext.h>
#include <usBundleContext.h>
#include <usBundle.h>
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
  std::map<long, std::shared_ptr<MyService1>> m_idToServiceMap;

  virtual us::InterfaceMap GetService(us::Bundle* bundle, const us::ServiceRegistrationBase& /*registration*/)
  {
    std::shared_ptr<MyService1> s = std::make_shared<MyService1>();
    m_idToServiceMap.insert(std::make_pair(bundle->GetBundleId(), s));
    return us::MakeInterfaceMap<Interface1>(s);
  }

  virtual void UngetService(us::Bundle* bundle, const us::ServiceRegistrationBase& /*registration*/,
                            const us::InterfaceMap& service)
  {
    std::map<long, std::shared_ptr<MyService1>>::iterator iter = m_idToServiceMap.find(bundle->GetBundleId());
    if (iter != m_idToServiceMap.end())
    {
      US_TEST_CONDITION((iter->second) == us::ExtractInterface<Interface1>(service), "Compare service pointer")
      m_idToServiceMap.erase(iter);
    }
  }
};

struct MyFactory2 : public us::ServiceFactory
{
  std::map<long, std::shared_ptr<MyService2>> m_idToServiceMap;

  virtual us::InterfaceMap GetService(us::Bundle* bundle, const us::ServiceRegistrationBase& /*registration*/)
  {
    std::shared_ptr<MyService2> s = std::make_shared<MyService2>();
    m_idToServiceMap.insert(std::make_pair(bundle->GetBundleId(), s));
    return us::MakeInterfaceMap<Interface1,Interface2>(s);
  }

  virtual void UngetService(us::Bundle* bundle, const us::ServiceRegistrationBase& /*registration*/,
                            const us::InterfaceMap& service)
  {
    std::map<long, std::shared_ptr<MyService2>>::iterator iter = m_idToServiceMap.find(bundle->GetBundleId());
    if (iter != m_idToServiceMap.end())
    {
      US_TEST_CONDITION((iter->second) == us::ExtractInterface<Interface2>(service), "Compare service pointer")
      m_idToServiceMap.erase(iter);
    }
  }
};

struct MyFactory3 : public us::ServiceFactory
{
  std::map<long, std::shared_ptr<MyService3>> m_idToServiceMap;

  virtual us::InterfaceMap GetService(us::Bundle* bundle, const us::ServiceRegistrationBase& /*registration*/)
  {
    std::shared_ptr<MyService3> s = std::make_shared<MyService3>();
    m_idToServiceMap.insert(std::make_pair(bundle->GetBundleId(), s));
    return us::MakeInterfaceMap<Interface1,Interface2,Interface3>(s);
  }

  virtual void UngetService(us::Bundle* bundle, const us::ServiceRegistrationBase& /*registration*/,
                            const us::InterfaceMap& service)
  {
    std::map<long, std::shared_ptr<MyService3>>::iterator iter = m_idToServiceMap.find(bundle->GetBundleId());
    if (iter != m_idToServiceMap.end())
    {
      US_TEST_CONDITION((iter->second) == us::ExtractInterface<Interface3>(service), "Compare service pointer")
      m_idToServiceMap.erase(iter);
    }
  }
};


using namespace us;

int usServiceTemplateTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceTemplateTest");

  FrameworkFactory factory;
  std::shared_ptr<Framework> framework = factory.NewFramework(std::map<std::string, std::string>());
  framework->Start();

  BundleContext* mc = framework->GetBundleContext();

  // Register compile tests
  std::shared_ptr<MyService1> s1 = std::make_shared<MyService1>();
  std::shared_ptr<MyService2> s2 = std::make_shared<MyService2>();
  std::shared_ptr<MyService3> s3 = std::make_shared<MyService3>();

  us::ServiceRegistration<Interface1> sr1 = mc->RegisterService<Interface1>(s1);
  us::ServiceRegistration<Interface1,Interface2> sr2 = mc->RegisterService<Interface1,Interface2>(s2);
  us::ServiceRegistration<Interface1,Interface2,Interface3> sr3 = mc->RegisterService<Interface1,Interface2,Interface3>(s3);

  std::shared_ptr<MyFactory1> f1 = std::make_shared<MyFactory1>();
  us::ServiceRegistration<Interface1> sfr1 = mc->RegisterService<Interface1>(ToFactory(f1));

  std::shared_ptr<MyFactory2> f2 = std::make_shared<MyFactory2>();
  us::ServiceRegistration<Interface1,Interface2> sfr2 = mc->RegisterService<Interface1,Interface2>(ToFactory(f2));

  std::shared_ptr<MyFactory3> f3 = std::make_shared<MyFactory3>();
  us::ServiceRegistration<Interface1,Interface2,Interface3> sfr3 = mc->RegisterService<Interface1,Interface2,Interface3>(ToFactory(f3));

#ifdef US_BUILD_SHARED_LIBS
  US_TEST_CONDITION(mc->GetBundle()->GetRegisteredServices().size() == 6, "# of reg services")
#endif

  std::vector<us::ServiceReference<Interface1> > s1refs = mc->GetServiceReferences<Interface1>();
  US_TEST_CONDITION(s1refs.size() == 6, "# of interface1 regs")
  std::vector<us::ServiceReference<Interface2> > s2refs = mc->GetServiceReferences<Interface2>();
  US_TEST_CONDITION(s2refs.size() == 4, "# of interface2 regs")
  std::vector<us::ServiceReference<Interface3> > s3refs = mc->GetServiceReferences<Interface3>();
  US_TEST_CONDITION(s3refs.size() == 2, "# of interface3 regs")

  std::shared_ptr<Interface1> i1 = mc->GetService(sr1.GetReference());
  US_TEST_CONDITION(i1 == s1, "interface1 ptr")
  i1.reset();
  
  i1 = mc->GetService(sfr1.GetReference());
  US_TEST_CONDITION(i1 == f1->m_idToServiceMap[mc->GetBundle()->GetBundleId()], "interface1 factory ptr")
  i1.reset();

  i1 = mc->GetService(sr2.GetReference<Interface1>());
  US_TEST_CONDITION(i1 == s2, "interface1 ptr")
  i1.reset();
  
  i1 = mc->GetService(sfr2.GetReference<Interface1>());
  US_TEST_CONDITION(i1 == f2->m_idToServiceMap[mc->GetBundle()->GetBundleId()], "interface1 factory ptr")
  i1.reset();
  
  std::shared_ptr<Interface2> i2 = mc->GetService(sr2.GetReference<Interface2>());
  US_TEST_CONDITION(i2 == s2, "interface2 ptr")
  i2.reset();
  
  i2 = mc->GetService(sfr2.GetReference<Interface2>());
  US_TEST_CONDITION(i2 == f2->m_idToServiceMap[mc->GetBundle()->GetBundleId()], "interface2 factory ptr")
  i2.reset();

  i1 = mc->GetService(sr3.GetReference<Interface1>());
  US_TEST_CONDITION(i1 == s3, "interface1 ptr")
  i1.reset();
  
  i1 = mc->GetService(sfr3.GetReference<Interface1>());
  US_TEST_CONDITION(i1 == f3->m_idToServiceMap[mc->GetBundle()->GetBundleId()], "interface1 factory ptr")
  i1.reset();
  
  i2 = mc->GetService(sr3.GetReference<Interface2>());
  US_TEST_CONDITION(i2 == s3, "interface2 ptr")
  i2.reset();
  
  i2 = mc->GetService(sfr3.GetReference<Interface2>());
  US_TEST_CONDITION(i2 == f3->m_idToServiceMap[mc->GetBundle()->GetBundleId()], "interface2 factory ptr")
  i2.reset();

    std::shared_ptr<Interface3> i3 = mc->GetService(sr3.GetReference<Interface3>());
  US_TEST_CONDITION(i3 == s3, "interface3 ptr")
  i3.reset();

  i3 = mc->GetService(sfr3.GetReference<Interface3>());
  US_TEST_CONDITION(i3 == f3->m_idToServiceMap[mc->GetBundle()->GetBundleId()], "interface3 factory ptr")
  i3.reset();

  sr1.Unregister();
  sr2.Unregister();
  sr3.Unregister();

  US_TEST_END()
}
