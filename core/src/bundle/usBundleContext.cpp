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
#include "usBundleContextPrivate.h"
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

BundleContext::BundleContext(BundlePrivate* bundle)
  : d(new BundleContextPrivate(bundle))
{}

BundleContext::~BundleContext()
{
}

std::shared_ptr<Bundle> BundleContext::GetBundle() const
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  return b->q->shared_from_this();
}

std::shared_ptr<Bundle> BundleContext::GetBundle(long id) const
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  return b->coreCtx->bundleHooks.FilterBundle(this, b->coreCtx->bundleRegistry.GetBundle(id));
}

std::shared_ptr<Bundle> BundleContext::GetBundle(const std::string& name)
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  return d->bundle->coreCtx->bundleRegistry.GetBundleByName(name);
}

std::vector<std::shared_ptr<Bundle>> BundleContext::GetBundles() const
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  auto bundles = b->coreCtx->bundleRegistry.GetBundles();
  b->coreCtx->bundleHooks.FilterBundles(this, bundles);
  return bundles;
}

ServiceRegistrationU BundleContext::RegisterService(const InterfaceMapConstPtr& service,
                                                    const ServiceProperties& properties)
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  return b->coreCtx->services.RegisterService(b, service, properties);
}

std::vector<ServiceReferenceU > BundleContext::GetServiceReferences(const std::string& clazz,
                                                                    const std::string& filter)
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  std::vector<ServiceReferenceU> result;
  std::vector<ServiceReferenceBase> refs;
  b->coreCtx->services.Get(clazz, filter, b, refs);
  for (std::vector<ServiceReferenceBase>::const_iterator iter = refs.begin();
       iter != refs.end(); ++iter)
  {
    result.push_back(ServiceReferenceU(*iter));
  }
  return result;
}

ServiceReferenceU BundleContext::GetServiceReference(const std::string& clazz)
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  return b->coreCtx->services.Get(d->bundle, clazz);
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

  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  std::shared_ptr<ServiceHolder<void>> h(new ServiceHolder<void>(this, reference, reference.d.load()->GetService(b->q->shared_from_this())));
  return std::shared_ptr<void>(h, h->service.get());
}

InterfaceMapConstPtr BundleContext::GetService(const ServiceReferenceU& reference)
{
  if (!reference)
  {
    throw std::invalid_argument("Default constructed ServiceReference is not a valid input to GetService()");
  }

  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  // Although according to the API contract the returned map should not be modified, there is nothing stopping the consumer from
  // using a const_pointer_cast and modifying the map. This copy step is to protect the map stored within the framework.
  InterfaceMapConstPtr imap_copy = std::make_shared<const InterfaceMap>(
        *(reference.d.load()->GetServiceInterfaceMap(b->q->shared_from_this()))
        );
  std::shared_ptr<ServiceHolder<const InterfaceMap>> h(new ServiceHolder<const InterfaceMap>(this, reference, imap_copy));
  return InterfaceMapConstPtr(h, h->service.get());
}

void BundleContext::AddServiceListener(const ServiceListener& delegate,
                                       const std::string& filter)
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  b->coreCtx->listeners.AddServiceListener(this, delegate, nullptr, filter);
}

void BundleContext::RemoveServiceListener(const ServiceListener& delegate)
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  b->coreCtx->listeners.RemoveServiceListener(this, delegate, nullptr);
}

void BundleContext::AddBundleListener(const BundleListener& delegate)
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  b->coreCtx->listeners.AddBundleListener(this, delegate, nullptr);
}

void BundleContext::RemoveBundleListener(const BundleListener& delegate)
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  b->coreCtx->listeners.RemoveBundleListener(this, delegate, nullptr);
}

bool BundleContext::UngetService(const ServiceReferenceBase& reference)
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  ServiceReferenceBase ref = reference;
  return ref.d.load()->UngetService(b->q->shared_from_this(), true);
}

void BundleContext::AddServiceListener(const ServiceListener& delegate, void* data,
                                       const std::string &filter)
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  b->coreCtx->listeners.AddServiceListener(this, delegate, data, filter);
}

void BundleContext::RemoveServiceListener(const ServiceListener& delegate, void* data)
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  b->coreCtx->listeners.RemoveServiceListener(this, delegate, data);
}

void BundleContext::AddBundleListener(const BundleListener& delegate, void* data)
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  b->coreCtx->listeners.AddBundleListener(this, delegate, data);
}

void BundleContext::RemoveBundleListener(const BundleListener& delegate, void* data)
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  b->coreCtx->listeners.RemoveBundleListener(this, delegate, data);
}

std::string BundleContext::GetDataFile(const std::string &filename) const
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  // compute the bundle storage path
#ifdef US_PLATFORM_WINDOWS
    static const char separator = '\\';
#else
    static const char separator = '/';
#endif

  std::string baseStoragePath;
  auto prop = b->coreCtx->frameworkProperties.find(Framework::PROP_STORAGE_LOCATION);
  if(prop != b->coreCtx->frameworkProperties.end() &&
     prop->second.Type() == typeid(std::string))
  {
    baseStoragePath = ref_any_cast<std::string>(prop->second);
  }

  if (baseStoragePath.empty()) return std::string();
  if (baseStoragePath != b->baseStoragePath)
  {
    b->baseStoragePath = baseStoragePath;
    b->storagePath.clear();
  }

  if (b->storagePath.empty())
  {
    char buf[50];
    sprintf(buf, "%ld", b->info.id);
    b->storagePath = baseStoragePath + separator + buf + "_" + b->info.name + separator;
  }
  return b->storagePath + filename;
}

std::shared_ptr<Bundle> BundleContext::InstallBundle(const std::string& location)
{
  auto b = (d->Lock(), d->IsValid_unlocked(), d->bundle);

  // CONCURRENCY NOTE: This is a check-then-act situation,
  // but we ignore it since the time window is small and
  // the result is the same as if the calling thread had
  // won the race condition.

  // TODO: Remove the optional bundlename in the location input param
  // The workaround is to support unittests only.
  std::string bundleLocation, bundleName;
  ExtractBundleNameAndLocation(location, bundleLocation, bundleName);
  BundleInfo bundleInfo(bundleLocation, bundleName);
  auto bundle = b->coreCtx->bundleRegistry.Register(bundleInfo);

  return bundle;
}

}
