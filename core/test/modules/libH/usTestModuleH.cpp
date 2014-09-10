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


#include <usServiceInterface.h>
#include <usModuleActivator.h>
#include <usServiceFactory.h>
#include <usPrototypeServiceFactory.h>
#include <usModule.h>
#include <usModuleContext.h>

#include <iostream>

US_BEGIN_NAMESPACE

struct TestModuleH
{
  virtual ~TestModuleH() {}
};

struct TestModuleH2
{
  virtual ~TestModuleH2() {}
};

class TestProduct : public TestModuleH
{
  // Module* caller;

public:

  TestProduct(Module* /*caller*/)
    //: caller(caller)
  {}

};

class TestProduct2 : public TestProduct, public TestModuleH2
{
public:

  TestProduct2(Module* caller)
    : TestProduct(caller)
  {}

};

class TestModuleHPrototypeServiceFactory : public PrototypeServiceFactory
{
  std::map<long, std::list<TestProduct2*> > fcbind;   // Map calling module with implementation

public:

  InterfaceMap GetService(Module* caller, const ServiceRegistrationBase& /*sReg*/)
  {
    std::cout << "GetService (prototype) in H" << std::endl;
    TestProduct2* product = new TestProduct2(caller);
    fcbind[caller->GetModuleId()].push_back(product);
    return MakeInterfaceMap<TestModuleH,TestModuleH2>(product);
  }

  void UngetService(Module* caller, const ServiceRegistrationBase& /*sReg*/, const InterfaceMap& service)
  {
    TestProduct2* product = dynamic_cast<TestProduct2*>(ExtractInterface<TestModuleH>(service));
    delete product;
    fcbind[caller->GetModuleId()].remove(product);
  }

};


class TestModuleHActivator : public ModuleActivator, public ServiceFactory
{
  std::string thisServiceName;
  ServiceRegistration<TestModuleH> factoryService;
  ServiceRegistration<TestModuleH,TestModuleH2> prototypeFactoryService;
  ModuleContext* mc;

  std::map<long, TestProduct*> fcbind;   // Map calling module with implementation
  TestModuleHPrototypeServiceFactory prototypeFactory;

public:

  TestModuleHActivator()
    : thisServiceName(us_service_interface_iid<TestModuleH>())
    , mc(NULL)
  {}

  void Load(ModuleContext* mc)
  {
    std::cout << "start in H" << std::endl;
    this->mc = mc;
    factoryService = mc->RegisterService<TestModuleH>(this);
    prototypeFactoryService = mc->RegisterService<TestModuleH,TestModuleH2>(static_cast<ServiceFactory*>(&prototypeFactory));
  }

  void Unload(ModuleContext* /*mc*/)
  {
    factoryService.Unregister();
  }

  InterfaceMap GetService(Module* caller, const ServiceRegistrationBase& /*sReg*/)
  {
    std::cout << "GetService in H" << std::endl;
    TestProduct* product = new TestProduct(caller);
    fcbind.insert(std::make_pair(caller->GetModuleId(), product));
    return MakeInterfaceMap<TestModuleH>(product);
  }

  void UngetService(Module* caller, const ServiceRegistrationBase& /*sReg*/, const InterfaceMap& service)
  {
    TestModuleH* product = ExtractInterface<TestModuleH>(service);
    delete product;
    fcbind.erase(caller->GetModuleId());
  }

};


US_END_NAMESPACE

US_EXPORT_MODULE_ACTIVATOR(us::TestModuleHActivator)
