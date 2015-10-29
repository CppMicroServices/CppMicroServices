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
#include <usFooService.h>

namespace us {

class SL3BundlePropsImpl : public BundlePropsInterface
{

public:
  const Properties& GetProperties() const
  {
    return props;
  }

  void SetProperty(std::string propertyKey, bool propertyValue)
  {
    props[propertyKey] = propertyValue;
  }

private:

  BundlePropsInterface::Properties props;

};

class SL3ServiceTrackerCustomizer : public ServiceTrackerCustomizer<FooService>
{

public:

  SL3ServiceTrackerCustomizer(std::shared_ptr<SL3BundlePropsImpl> propService, BundleContext* bc) : bundlePropsService(propService), context(bc) {}
  virtual ~SL3ServiceTrackerCustomizer() { context = NULL; }

  std::shared_ptr<FooService> AddingService(const ServiceReferenceType& reference)
  {
    bundlePropsService->SetProperty("serviceAdded", true);
    
    std::shared_ptr<FooService> fooService = context->GetService<FooService>(reference);
    fooService->foo();
    return fooService;
  }

  void ModifiedService(const ServiceReferenceType& /*reference*/, std::shared_ptr<FooService> /*service*/)
  {
  
  }

  void RemovedService(const ServiceReferenceType& /*reference*/, std::shared_ptr<FooService> /*service*/)
  {
    bundlePropsService->SetProperty("serviceRemoved", true);
  }

private:

  std::shared_ptr<SL3BundlePropsImpl> bundlePropsService;
  BundleContext* context;

};

class ActivatorSL3 :
    public BundleActivator
{

public:

  ActivatorSL3() : tracker(0), context(0) {}

  ~ActivatorSL3()
  { delete tracker; }

  void Start(BundleContext* context)
  {
    this->context = context;
    bundlePropsService = std::make_shared<SL3BundlePropsImpl>();
    InterfaceMap im = MakeInterfaceMap<BundlePropsInterface>(bundlePropsService);
    im.insert(std::make_pair(std::string("ActivatorSL3"), bundlePropsService));
    sr = context->RegisterService(im);
    delete tracker;
    tracker = new FooTracker(context, new SL3ServiceTrackerCustomizer(bundlePropsService, context));
    tracker->Open();
  }

  void Stop(BundleContext* /*context*/)
  {
    tracker->Close();
  }

private:

  std::shared_ptr<SL3BundlePropsImpl> bundlePropsService;
  typedef ServiceTracker<FooService> FooTracker;
  FooTracker* tracker;
  BundleContext* context;

  ServiceRegistrationU sr;

};

}

US_EXPORT_BUNDLE_ACTIVATOR(us::ActivatorSL3)
