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

std::shared_ptr<Bundle> BundleContext::GetBundle() const
{
  return d->bundle->q;
}

std::shared_ptr<Bundle> BundleContext::GetBundle(long id) const
{
  return d->bundle->coreCtx->bundleHooks.FilterBundle(this, d->bundle->coreCtx->bundleRegistry.GetBundle(id));
}

std::shared_ptr<Bundle> BundleContext::GetBundle(const std::string& name)
{
  return d->bundle->coreCtx->bundleRegistry.GetBundle(name);
}

std::vector<std::shared_ptr<Bundle>> BundleContext::GetBundles() const
{
  std::vector<std::shared_ptr<Bundle>> bundles = d->bundle->coreCtx->bundleRegistry.GetBundles();
  d->bundle->coreCtx->bundleHooks.FilterBundles(this, bundles);
  return bundles;
}

ServiceRegistrationU BundleContext::RegisterService(const InterfaceMapConstPtr& service,
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

/* @brief Private helper struct used to facilitate the shared_ptr aliasing constructor
 *        in BundleContext::GetService method. The aliasing constructor helps automate
 *        the call to UngetService method.
 *
 *        Service consumers can simply call GetService to obtain a shared_ptr to the
 *        service object and not worry about calling UngetService when they are done.
 *        The UngetService is called when all instances of the returned shared_ptr object
 *        go out of scope.
 */
template <class S>
struct ServiceHolder
{
  BundleContext* const bc;
  const ServiceReferenceBase sref;
  const std::shared_ptr<S> service;

  ServiceHolder(BundleContext* bc, const ServiceReferenceBase& sr, const std::shared_ptr<S>& s)
    : bc(bc)
    , sref(sr)
    , service(s)
  {}

  ~ServiceHolder()
  {
    try
    {
      bc->UngetService(sref);
    }
    catch (const std::exception& exp)
    {
      US_INFO << "UngetService threw an exception - " << exp.what();
    }
  }
};

std::shared_ptr<void> BundleContext::GetService(const ServiceReferenceBase& reference)
{
  if (!reference)
  {
    throw std::invalid_argument("Default constructed ServiceReference is not a valid input to GetService()");
  }
  std::shared_ptr<ServiceHolder<void>> h(new ServiceHolder<void>(this, reference, reference.d->GetService(d->bundle->q)));
  return std::shared_ptr<void>(h, h->service.get());
}

InterfaceMapConstPtr BundleContext::GetService(const ServiceReferenceU& reference)
{
  if (!reference)
  {
    throw std::invalid_argument("Default constructed ServiceReference is not a valid input to GetService()");
  }

  // Although according to the API contract the returned map should not be modified, there is nothing stopping the consumer from
  // using a const_pointer_cast and modifying the map. This copy step is to protect the map stored within the framework.
  InterfaceMapConstPtr imap_copy = std::make_shared<const InterfaceMap>(*(reference.d->GetServiceInterfaceMap(d->bundle->q).get()));
  std::shared_ptr<ServiceHolder<const InterfaceMap>> h(new ServiceHolder<const InterfaceMap>(this, reference, imap_copy));
  return InterfaceMapConstPtr(h, h->service.get());
}

bool BundleContext::UngetService(const ServiceReferenceBase& reference)
{
  ServiceReferenceBase ref = reference;
  return ref.d->UngetService(d->bundle->q, true);
}

void BundleContext::AddServiceListener(const ServiceListener& delegate,
                                       const std::string& filter)
{
  d->bundle->coreCtx->listeners.AddServiceListener(this, delegate, NULL, filter);
}

void BundleContext::RemoveServiceListener(const ServiceListener& delegate)
{
  d->bundle->coreCtx->listeners.RemoveServiceListener(this, delegate, NULL);
}

void BundleContext::AddBundleListener(const BundleListener& delegate)
{
  d->bundle->coreCtx->listeners.AddBundleListener(this, delegate, NULL);
}

void BundleContext::RemoveBundleListener(const BundleListener& delegate)
{
  d->bundle->coreCtx->listeners.RemoveBundleListener(this, delegate, NULL);
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
  std::map<std::string, std::string>::iterator prop = d->bundle->coreCtx->frameworkProperties.find(Framework::PROP_STORAGE_LOCATION);
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

std::shared_ptr<Bundle> BundleContext::InstallBundle(const std::string& location)
{
    BundleInfo* bundleInfo = new BundleInfo(GetBundleNameFromLocation(location));
    bundleInfo->location = GetBundleLocation(location);

    auto bundle = d->bundle->coreCtx->bundleRegistry.Register(bundleInfo);

    d->bundle->coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::INSTALLED, bundle));

    return bundle;
}


}
