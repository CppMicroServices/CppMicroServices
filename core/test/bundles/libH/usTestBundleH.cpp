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
  // std::shared_ptr<Bundle> caller;

public:

  TestProduct(const std::shared_ptr<Bundle>& /*caller*/)
    //: caller(caller)
  {}

};

class TestProduct2 : public TestProduct, public TestBundleH2
{
public:

  TestProduct2(const std::shared_ptr<Bundle>& caller)
    : TestProduct(caller)
  {}

};

class TestBundleHPrototypeServiceFactory : public PrototypeServiceFactory
{
  std::map<long, std::list<std::shared_ptr<TestProduct2>> > fcbind;   // Map calling bundle with implementation

public:

  InterfaceMapConstPtr GetService(const std::shared_ptr<Bundle>& caller, const ServiceRegistrationBase& /*sReg*/)
  {
    std::cout << "GetService (prototype) in H" << std::endl;
    std::shared_ptr<TestProduct2> product = std::make_shared<TestProduct2>(caller);
    fcbind[caller->GetBundleId()].push_back(product);
    return MakeInterfaceMap<TestBundleH,TestBundleH2>(product);
  }

  void UngetService(const std::shared_ptr<Bundle>& caller, const ServiceRegistrationBase& /*sReg*/, const InterfaceMapConstPtr& service)
  {
    std::shared_ptr<TestProduct2> product = std::dynamic_pointer_cast<TestProduct2>(ExtractInterface<TestBundleH>(service));
    fcbind[caller->GetBundleId()].remove(product);
  }

};

class TestBundleHServiceFactory : public ServiceFactory
{
  std::map<long, std::shared_ptr<TestProduct>> fcbind;   // Map calling bundle with implementation
public:
  
  InterfaceMapConstPtr GetService(const std::shared_ptr<Bundle>& caller, const ServiceRegistrationBase& /*sReg*/)
  {
    std::cout << "GetService in H" << std::endl;
    std::shared_ptr<TestProduct> product = std::make_shared<TestProduct>(caller);
    fcbind.insert(std::make_pair(caller->GetBundleId(), product));
    return MakeInterfaceMap<TestBundleH>(product);
  }
  
  void UngetService(const std::shared_ptr<Bundle>& caller, const ServiceRegistrationBase& /*sReg*/, const InterfaceMapConstPtr& service)
  {
    std::shared_ptr<TestBundleH> product = ExtractInterface<TestBundleH>(service);
    fcbind.erase(caller->GetBundleId());
  }
  
};

class TestBundleHActivator : public BundleActivator
{
  std::string thisServiceName;
  ServiceRegistration<TestBundleH> factoryService;
  ServiceRegistration<TestBundleH,TestBundleH2> prototypeFactoryService;
  BundleContext* mc;
  std::shared_ptr<ServiceFactory> factoryObj;
  std::shared_ptr<TestBundleHPrototypeServiceFactory> prototypeFactoryObj;

public:

  TestBundleHActivator()
    : thisServiceName(us_service_interface_iid<TestBundleH>())
    , mc(NULL)
  {}

  void Start(BundleContext* mc)
  {
    std::cout << "start in H" << std::endl;
    this->mc = mc;
    factoryObj = std::make_shared<TestBundleHServiceFactory>();
    factoryService = mc->RegisterService<TestBundleH>(ToFactory(factoryObj));
    prototypeFactoryObj = std::make_shared<TestBundleHPrototypeServiceFactory>();
    prototypeFactoryService = mc->RegisterService<TestBundleH,TestBundleH2>(ToFactory(prototypeFactoryObj));
  }

  void Stop(BundleContext* /*mc*/)
  {
    factoryService.Unregister();
  }

};


}

US_EXPORT_BUNDLE_ACTIVATOR(us::TestBundleHActivator)
