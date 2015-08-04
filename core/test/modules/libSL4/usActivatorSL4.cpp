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


#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usServiceRegistration.h>

#include <usFooService.h>

#include "usLog_p.h"

US_BEGIN_NAMESPACE

class ActivatorSL4 :
  public ModuleActivator, public FooService
{

public:

  ~ActivatorSL4()
  {

  }

  void foo()
  {
    US_INFO << "TestModuleSL4: Doing foo";
  }

  void Load(ModuleContext* context)
  {
    sr = context->RegisterService<FooService>(this);
    US_INFO << "TestModuleSL4: Registered " << sr;
  }

  void Unload(ModuleContext* /*context*/)
  {
  }

private:

  ServiceRegistration<FooService> sr;
};

US_END_NAMESPACE

US_EXPORT_MODULE_ACTIVATOR(US_PREPEND_NAMESPACE(ActivatorSL4))
