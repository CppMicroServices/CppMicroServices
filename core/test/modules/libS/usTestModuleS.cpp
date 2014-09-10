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

#include "../../usServiceControlInterface.h"

#include "usTestModuleSService0.h"
#include "usTestModuleSService1.h"
#include "usTestModuleSService2.h"
#include "usTestModuleSService3.h"

#include <usServiceRegistration.h>
#include <usModuleContext.h>
#include <usModuleActivator.h>


US_BEGIN_NAMESPACE

class TestModuleS : public ServiceControlInterface,
                    public TestModuleSService0,
                    public TestModuleSService1,
                    public TestModuleSService2,
                    public TestModuleSService3
{

public:

  TestModuleS(ModuleContext* mc)
    : mc(mc)
  {
    for(int i = 0; i <= 3; ++i)
    {
      servregs.push_back(ServiceRegistrationU());
    }
    sreg = mc->RegisterService<TestModuleSService0>(this);
    sciReg = mc->RegisterService<ServiceControlInterface>(this);
  }

  virtual const char* GetNameOfClass() const
  {
    return "TestModuleS";
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
          servregs[offset] = 0;
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

  static const std::string SERVICE; // = "us::TestModuleSService"

  ModuleContext* mc;
  std::vector<ServiceRegistrationU> servregs;
  ServiceRegistration<TestModuleSService0> sreg;
  ServiceRegistration<ServiceControlInterface> sciReg;
};

const std::string TestModuleS::SERVICE = "us::TestModuleSService";

class TestModuleSActivator : public ModuleActivator
{

public:

  TestModuleSActivator() : s(0) {}
  ~TestModuleSActivator() { delete s; }

  void Load(ModuleContext* context)
  {
    s = new TestModuleS(context);
  }

  void Unload(ModuleContext* /*context*/)
  {
  #ifndef US_BUILD_SHARED_LIBS
    s->Unregister();
  #endif
  }

private:

  TestModuleS* s;

};

US_END_NAMESPACE

US_EXPORT_MODULE_ACTIVATOR(US_PREPEND_NAMESPACE(TestModuleSActivator))
