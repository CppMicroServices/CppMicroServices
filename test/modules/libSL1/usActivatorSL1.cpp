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


#include <usModuleActivator.h>
#include <usModulePropsInterface.h>

#include <usServiceTracker.h>
#include <usServiceTrackerCustomizer.h>

#include "usFooService.h"

#include US_BASECLASS_HEADER

US_BEGIN_NAMESPACE

class ActivatorSL1 :
    public US_BASECLASS_NAME, public ModuleActivator, public ModulePropsInterface,
    public ServiceTrackerCustomizer<FooService*>
{

public:

  ActivatorSL1()
    : tracker(0), context(0)
  {

  }

  ~ActivatorSL1()
  {
    delete tracker;
  }

  void Load(ModuleContext* context)
  {
    this->context = context;

    sr = context->RegisterService("ActivatorSL1", this);

    delete tracker;
    tracker = new FooTracker(context, this);
    tracker->Open();
  }

  void Unload(ModuleContext* /*context*/)
  {
    tracker->Close();
  }

  const Properties& GetProperties() const
  {
    return props;
  }

  FooService* AddingService(const ServiceReference& reference)
  {
    props["serviceAdded"] = true;

    FooService* fooService = context->GetService<FooService>(reference);
    fooService->foo();
    return fooService;
  }

  void ModifiedService(const ServiceReference& /*reference*/, FooService* /*service*/)
  {}

  void RemovedService(const ServiceReference& /*reference*/, FooService* /*service*/)
  {
    props["serviceRemoved"] = true;
  }

private:

  ModulePropsInterface::Properties props;

  ServiceRegistration sr;

  typedef ServiceTracker<FooService*> FooTracker;

  FooTracker* tracker;
  ModuleContext* context;

}; // ActivatorSL1

US_END_NAMESPACE

US_EXPORT_MODULE_ACTIVATOR(TestModuleSL1, US_PREPEND_NAMESPACE(ActivatorSL1))


