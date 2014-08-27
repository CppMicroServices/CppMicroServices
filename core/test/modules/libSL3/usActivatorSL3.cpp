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
#include <usFooService.h>

US_BEGIN_NAMESPACE

class ActivatorSL3 :
    public ModuleActivator, public ModulePropsInterface,
    public ServiceTrackerCustomizer<FooService>
{

public:

  ActivatorSL3() : tracker(0), context(0) {}

  ~ActivatorSL3()
  { delete tracker; }

  void Load(ModuleContext* context)
  {
    this->context = context;

    InterfaceMap im = MakeInterfaceMap<ModulePropsInterface>(this);
    im.insert(std::make_pair(std::string("ActivatorSL3"), this));
    sr = context->RegisterService(im);
    delete tracker;
    tracker = new FooTracker(context, this);
    tracker->Open();
  }

  void Unload(ModuleContext* /*context*/)
  {
    tracker->Close();
  }

  const ModulePropsInterface::Properties& GetProperties() const
  {
    return props;
  }

  FooService* AddingService(const ServiceReferenceType& reference)
  {
    props["serviceAdded"] = true;
    US_INFO << "SL3: Adding reference =" << reference;

    FooService* fooService = context->GetService(reference);
    fooService->foo();
    return fooService;
  }

  void ModifiedService(const ServiceReferenceType& /*reference*/, FooService* /*service*/)
  {

  }

  void RemovedService(const ServiceReferenceType& reference, FooService* /*service*/)
  {
    props["serviceRemoved"] = true;
    US_INFO << "SL3: Removing reference =" << reference;
  }

private:

  typedef ServiceTracker<FooService> FooTracker;
  FooTracker* tracker;
  ModuleContext* context;

  ServiceRegistrationU sr;

  ModulePropsInterface::Properties props;

};

US_END_NAMESPACE

US_EXPORT_MODULE_ACTIVATOR(US_PREPEND_NAMESPACE(ActivatorSL3))
