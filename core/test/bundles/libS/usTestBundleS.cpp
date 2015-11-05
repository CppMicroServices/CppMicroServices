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

#include "../../usServiceControlInterface.h"

#include "usTestBundleSService0.h"
#include "usTestBundleSService1.h"
#include "usTestBundleSService2.h"
#include "usTestBundleSService3.h"

#include <usServiceRegistration.h>
#include <usBundleContext.h>
#include <usBundleActivator.h>


namespace us {

class TestBundleS : public ServiceControlInterface,
                    public TestBundleSService0,
                    public TestBundleSService1,
                    public TestBundleSService2,
                    public TestBundleSService3
{

public:

  TestBundleS(BundleContext* mc)
    : mc(mc)
  {
    for(int i = 0; i <= 3; ++i)
    {
      servregs.push_back(ServiceRegistrationU());
    }
    sreg = mc->RegisterService<TestBundleSService0>(this);
    sciReg = mc->RegisterService<ServiceControlInterface>(this);
  }

  virtual const char* GetNameOfClass() const
  {
    return "TestBundleS";
  }

  void ServiceControl(int offset, const std::string& operation, int ranking)
  {
    if (0 <= offset && offset <= 3)
    {
      if (operation == "register")
      {
        if (!servregs[offset])
        {
          std::stringstream servicename;
          servicename << SERVICE << offset;
          InterfaceMap ifm;
          ifm.insert(std::make_pair(servicename.str(), static_cast<void*>(this)));
          ServiceProperties props;
          props.insert(std::make_pair(ServiceConstants::SERVICE_RANKING(), Any(ranking)));
          servregs[offset] = mc->RegisterService(ifm, props);
        }
      }
      if (operation == "unregister")
      {
        if (servregs[offset])
        {
          ServiceRegistrationU sr1 = servregs[offset];
          sr1.Unregister();
          servregs[offset] = nullptr;
        }
      }
    }
  }

  void Unregister()
  {
    if (sreg)
    {
      sreg.Unregister();
    }
    if (sciReg)
    {
      sciReg.Unregister();
    }
  }

private:

  static const std::string SERVICE; // = "us::TestBundleSService"

  BundleContext* mc;
  std::vector<ServiceRegistrationU> servregs;
  ServiceRegistration<TestBundleSService0> sreg;
  ServiceRegistration<ServiceControlInterface> sciReg;
};

const std::string TestBundleS::SERVICE = "us::TestBundleSService";

class TestBundleSActivator : public BundleActivator
{

public:

  TestBundleSActivator() {}
  ~TestBundleSActivator() {}

  void Start(BundleContext* context)
  {
    s.reset(new TestBundleS(context));
  }

  void Stop(BundleContext* /*context*/)
  {
  #ifndef US_BUILD_SHARED_LIBS
    s->Unregister();
  #endif
  }

private:

  std::unique_ptr<TestBundleS> s;

};

}

US_EXPORT_BUNDLE_ACTIVATOR(us::TestBundleSActivator)
