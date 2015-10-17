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

#include "usBundleContext.h"

#include "usBundle.h"
#include "usBundleEvent.h"
#include "usBundleRegistry_p.h"
#include "usBundlePrivate.h"
#include "usBundleSettings.h"
#include "usCoreBundleContext_p.h"
#include "usFramework.h"
#include "usServiceRegistry_p.h"
#include "usServiceReferenceBasePrivate.h"
#include "usUtils_p.h"

#include <stdio.h>

namespace us {

class BundleContextPrivate {

public:

  BundleContextPrivate(BundlePrivate* bundle)
  : bundle(bundle)
  {}

  BundlePrivate* bundle;
};


BundleContext::BundleContext(BundlePrivate* bundle)
  : d(new BundleContextPrivate(bundle))
{}

BundleContext::~BundleContext()
{
  delete d;
}

Bundle* BundleContext::GetBundle() const
{
  return d->bundle->q;
}

Bundle* BundleContext::GetBundle(long id) const
{
  return d->bundle->coreCtx->bundleHooks.FilterBundle(this, d->bundle->coreCtx->bundleRegistry.GetBundle(id));
}

Bundle* BundleContext::GetBundle(const std::string& name)
{
  return d->bundle->coreCtx->bundleRegistry.GetBundle(name);
}

std::vector<Bundle*> BundleContext::GetBundles() const
{
  std::vector<Bundle*> bundles = d->bundle->coreCtx->bundleRegistry.GetBundles();
  d->bundle->coreCtx->bundleHooks.FilterBundles(this, bundles);
  return bundles;
}

ServiceRegistrationU BundleContext::RegisterService(const InterfaceMap& service,
                                                    const ServiceProperties& properties)
{
  return d->bundle->coreCtx->services.RegisterService(d->bundle, service, properties);
}

std::vector<ServiceReferenceU > BundleContext::GetServiceReferences(const std::string& clazz,
                                                                    const std::string& filter)
{
  std::vector<ServiceReferenceU> result;
  std::vector<ServiceReferenceBase> refs;
  d->bundle->coreCtx->services.Get(clazz, filter, d->bundle, refs);
  for (std::vector<ServiceReferenceBase>::const_iterator iter = refs.begin();
       iter != refs.end(); ++iter)
  {
    result.push_back(ServiceReferenceU(*iter));
  }
  return result;
}

ServiceReferenceU BundleContext::GetServiceReference(const std::string& clazz)
{
  return d->bundle->coreCtx->services.Get(d->bundle, clazz);
}

void* BundleContext::GetService(const ServiceReferenceBase& reference)
{
  if (!reference)
  {
    throw std::invalid_argument("Default constructed ServiceReference is not a valid input to GetService()");
  }
  return reference.d->GetService(d->bundle->q);
}

InterfaceMap BundleContext::GetService(const ServiceReferenceU& reference)
{
  if (!reference)
  {
    throw std::invalid_argument("Default constructed ServiceReference is not a valid input to GetService()");
  }
  return reference.d->GetServiceInterfaceMap(d->bundle->q);
}

bool BundleContext::UngetService(const ServiceReferenceBase& reference)
{
  ServiceReferenceBase ref = reference;
  return ref.d->UngetService(d->bundle->q, true);
}

void BundleContext::AddServiceListener(const ServiceListener& delegate,
                                       const std::string& filter)
{
  d->bundle->coreCtx->listeners.AddServiceListener(this, delegate, nullptr, filter);
}

void BundleContext::RemoveServiceListener(const ServiceListener& delegate)
{
  d->bundle->coreCtx->listeners.RemoveServiceListener(this, delegate, nullptr);
}

void BundleContext::AddBundleListener(const BundleListener& delegate)
{
  d->bundle->coreCtx->listeners.AddBundleListener(this, delegate, nullptr);
}

void BundleContext::RemoveBundleListener(const BundleListener& delegate)
{
  d->bundle->coreCtx->listeners.RemoveBundleListener(this, delegate, nullptr);
}

void BundleContext::AddServiceListener(const ServiceListener& delegate, void* data,
                                       const std::string &filter)
{
  d->bundle->coreCtx->listeners.AddServiceListener(this, delegate, data, filter);
}

void BundleContext::RemoveServiceListener(const ServiceListener& delegate, void* data)
{
  d->bundle->coreCtx->listeners.RemoveServiceListener(this, delegate, data);
}

void BundleContext::AddBundleListener(const BundleListener& delegate, void* data)
{
  d->bundle->coreCtx->listeners.AddBundleListener(this, delegate, data);
}

void BundleContext::RemoveBundleListener(const BundleListener& delegate, void* data)
{
  d->bundle->coreCtx->listeners.RemoveBundleListener(this, delegate, data);
}

std::string BundleContext::GetDataFile(const std::string &filename) const
{
  // compute the bundle storage path
#ifdef US_PLATFORM_WINDOWS
    static const char separator = '\\';
#else
    static const char separator = '/';
#endif
  
  std::string baseStoragePath;
  auto prop = d->bundle->coreCtx->frameworkProperties.find(Framework::PROP_STORAGE_LOCATION);
  if(prop != d->bundle->coreCtx->frameworkProperties.end())
  { 
    baseStoragePath = (*prop).second;
  }
  
  if (baseStoragePath.empty()) return std::string();
  if (baseStoragePath != d->bundle->baseStoragePath)
  {
    d->bundle->baseStoragePath = baseStoragePath;
    d->bundle->storagePath.clear();
  }

  if (d->bundle->storagePath.empty())
  {
    char buf[50];
    sprintf(buf, "%ld", d->bundle->info.id);
    d->bundle->storagePath = baseStoragePath + separator + buf + "_" + d->bundle->info.name + separator;
  }
  return d->bundle->storagePath + filename;
}

Bundle* BundleContext::InstallBundle(const std::string& location)
{
    BundleInfo* bundleInfo = new BundleInfo(GetBundleNameFromLocation(location));
    bundleInfo->location = GetBundleLocation(location);

    Bundle* bundle = d->bundle->coreCtx->bundleRegistry.Register(bundleInfo);

    d->bundle->coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::INSTALLED, bundle));

    return bundle;
}


}
