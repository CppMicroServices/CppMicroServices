/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/PrototypeServiceFactory.h"
#include "cppmicroservices/ServiceFactory.h"
#include "cppmicroservices/ServiceInterface.h"

#include <iostream>
#include <mutex>

namespace cppmicroservices {

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
  // Bundle caller;

public:

  TestProduct(const Bundle& /*caller*/)
    //: caller(caller)
  {}

};

class TestProduct2 : public TestProduct, public TestBundleH2
{
public:

  TestProduct2(const Bundle& caller)
    : TestProduct(caller)
  {}

};

class TestBundleHPrototypeServiceFactory : public PrototypeServiceFactory
{
  std::map<long, std::list<std::shared_ptr<TestProduct2>> > fcbind;   // Map calling bundle with implementation
  std::mutex fcbindLock;
public:

  InterfaceMapConstPtr GetService(const Bundle& caller, const ServiceRegistrationBase& /*sReg*/)
  {
    std::unique_lock<std::mutex> lock(fcbindLock);
    std::shared_ptr<TestProduct2> product = std::make_shared<TestProduct2>(caller);
    fcbind[caller.GetBundleId()].push_back(product);
    return MakeInterfaceMap<TestBundleH,TestBundleH2>(product);
  }

  void UngetService(const Bundle& caller, const ServiceRegistrationBase& /*sReg*/, const InterfaceMapConstPtr& service)
  {
    std::unique_lock<std::mutex> lock(fcbindLock);
    std::shared_ptr<TestProduct2> product = std::dynamic_pointer_cast<TestProduct2>(ExtractInterface<TestBundleH>(service));
    fcbind[caller.GetBundleId()].remove(product);
  }

};

class TestBundleHServiceFactory : public ServiceFactory
{
  std::map<long, std::shared_ptr<TestProduct>> fcbind;   // Map calling bundle with implementation
  std::mutex fcbindLock;
public:

  InterfaceMapConstPtr GetService(const Bundle& caller, const ServiceRegistrationBase& /*sReg*/)
  {
    std::unique_lock<std::mutex> lock(fcbindLock);
    std::shared_ptr<TestProduct> product = std::make_shared<TestProduct>(caller);
    fcbind.insert(std::make_pair(caller.GetBundleId(), product));
    return MakeInterfaceMap<TestBundleH>(product);
  }

  void UngetService(const Bundle& caller, const ServiceRegistrationBase& /*sReg*/, const InterfaceMapConstPtr& service)
  {
    std::unique_lock<std::mutex> lock(fcbindLock);
    std::shared_ptr<TestBundleH> product = ExtractInterface<TestBundleH>(service);
    fcbind.erase(caller.GetBundleId());
  }

};

class TestBundleHActivator : public BundleActivator
{
  std::string thisServiceName;
  ServiceRegistration<TestBundleH> factoryService;
  ServiceRegistration<TestBundleH,TestBundleH2> prototypeFactoryService;
  std::shared_ptr<ServiceFactory> factoryObj;
  std::shared_ptr<TestBundleHPrototypeServiceFactory> prototypeFactoryObj;

public:

  TestBundleHActivator()
    : thisServiceName(us_service_interface_iid<TestBundleH>())
  {}

  void Start(BundleContext context)
  {
    factoryObj = std::make_shared<TestBundleHServiceFactory>();
    factoryService = context.RegisterService<TestBundleH>(ToFactory(factoryObj));
    prototypeFactoryObj = std::make_shared<TestBundleHPrototypeServiceFactory>();
    prototypeFactoryService = context.RegisterService<TestBundleH,TestBundleH2>(ToFactory(prototypeFactoryObj));
  }

  void Stop(BundleContext /*context*/)
  {
    factoryService.Unregister();
    prototypeFactoryService.Unregister();
  }

};


}

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(cppmicroservices::TestBundleHActivator)
