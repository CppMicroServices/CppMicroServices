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


#include <usBundleActivator.h>
#include <usBundlePropsInterface.h>

#include <usServiceTracker.h>
#include <usServiceTrackerCustomizer.h>

#include "usFooService.h"

US_BEGIN_NAMESPACE

class ActivatorSL1 :
    public BundleActivator, public BundlePropsInterface,
    public ServiceTrackerCustomizer<FooService>
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

  void Start(BundleContext* context)
  {
    this->context = context;

    InterfaceMap im = MakeInterfaceMap<BundlePropsInterface>(this);
    im.insert(std::make_pair(std::string("ActivatorSL1"), this));
    sr = context->RegisterService(im);

    delete tracker;
    tracker = new FooTracker(context, this);
    tracker->Open();
  }

  void Stop(BundleContext* /*context*/)
  {
    tracker->Close();
  }

  const Properties& GetProperties() const
  {
    return props;
  }

  FooService* AddingService(const ServiceReferenceType& reference)
  {
    props["serviceAdded"] = true;

    FooService* fooService = context->GetService<FooService>(reference);
    fooService->foo();
    return fooService;
  }

  void ModifiedService(const ServiceReferenceType& /*reference*/, FooService* /*service*/)
  {}

  void RemovedService(const ServiceReferenceType& /*reference*/, FooService* /*service*/)
  {
    props["serviceRemoved"] = true;
  }

private:

  BundlePropsInterface::Properties props;

  ServiceRegistrationU sr;

  typedef ServiceTracker<FooService> FooTracker;

  FooTracker* tracker;
  BundleContext* context;

}; // ActivatorSL1

US_END_NAMESPACE

US_EXPORT_BUNDLE_ACTIVATOR(US_PREPEND_NAMESPACE(ActivatorSL1))
