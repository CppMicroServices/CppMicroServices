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


#include <usServiceInterface.h>
#include <usBundleActivator.h>
#include <usServiceFactory.h>
#include <usPrototypeServiceFactory.h>
#include <usBundle.h>
#include <usBundleContext.h>

#include <iostream>

namespace us {

struct TestBundleH
{
  virtual ~TestBundleH() {}
};

struct TestBundleH2
{
  virtual ~TestBundleH2() {}
};

class TestProduct : public TestBundleH
{
  // Bundle* caller;

public:

  TestProduct(Bundle* /*caller*/)
    //: caller(caller)
  {}

};

class TestProduct2 : public TestProduct, public TestBundleH2
{
public:

  TestProduct2(Bundle* caller)
    : TestProduct(caller)
  {}

};

class TestBundleHPrototypeServiceFactory : public PrototypeServiceFactory
{
  std::map<long, std::list<TestProduct2*> > fcbind;   // Map calling bundle with implementation

public:

  InterfaceMap GetService(Bundle* caller, const ServiceRegistrationBase& /*sReg*/)
  {
    std::cout << "GetService (prototype) in H" << std::endl;
    TestProduct2* product = new TestProduct2(caller);
    fcbind[caller->GetBundleId()].push_back(product);
    return MakeInterfaceMap<TestBundleH,TestBundleH2>(product);
  }

  void UngetService(Bundle* caller, const ServiceRegistrationBase& /*sReg*/, const InterfaceMap& service)
  {
    TestProduct2* product = dynamic_cast<TestProduct2*>(ExtractInterface<TestBundleH>(service));
    delete product;
    fcbind[caller->GetBundleId()].remove(product);
  }

};


class TestBundleHActivator : public BundleActivator, public ServiceFactory
{
  std::string thisServiceName;
  ServiceRegistration<TestBundleH> factoryService;
  ServiceRegistration<TestBundleH,TestBundleH2> prototypeFactoryService;
  BundleContext* mc;

  std::map<long, TestProduct*> fcbind;   // Map calling bundle with implementation
  TestBundleHPrototypeServiceFactory prototypeFactory;

public:

  TestBundleHActivator()
    : thisServiceName(us_service_interface_iid<TestBundleH>())
    , mc(nullptr)
  {}

  void Start(BundleContext* mc)
  {
    this->mc = mc;
    factoryService = mc->RegisterService<TestBundleH>(ToFactory(this));
    prototypeFactoryService = mc->RegisterService<TestBundleH,TestBundleH2>(ToFactory(prototypeFactory));
  }

  void Stop(BundleContext* /*mc*/)
  {
    factoryService.Unregister();
  }

  InterfaceMap GetService(Bundle* caller, const ServiceRegistrationBase& /*sReg*/)
  {
    TestProduct* product = new TestProduct(caller);
    fcbind.insert(std::make_pair(caller->GetBundleId(), product));
    return MakeInterfaceMap<TestBundleH>(product);
  }

  void UngetService(Bundle* caller, const ServiceRegistrationBase& /*sReg*/, const InterfaceMap& service)
  {
    TestBundleH* product = ExtractInterface<TestBundleH>(service);
    delete product;
    fcbind.erase(caller->GetBundleId());
  }

};


}

US_EXPORT_BUNDLE_ACTIVATOR(us::TestBundleHActivator)
