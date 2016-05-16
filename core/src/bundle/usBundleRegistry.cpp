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

#include "usBundleRegistry_p.h"

#include "usFramework.h"
#include "usBundle.h"
#include "usBundleInfo.h"
#include "usBundleContext.h"
#include "usBundleActivator.h"
#include "usBundlePrivate.h"
#include "usCoreBundleContext_p.h"
#include "usGetBundleContext.h"
#include "usBundleEvent.h"
#include <cassert>
#include <map>


namespace us {

BundleRegistry::BundleRegistry(CoreBundleContext* coreCtx)
  : coreCtx(coreCtx)
  , id(1)
{
}

BundleRegistry::~BundleRegistry(void)
{
}

std::shared_ptr<Bundle> BundleRegistry::Register(BundleInfo info)
{
  std::vector<std::string> embeddedBundles; // statically-linked bundles in the binary
  if (info.name.empty())
  {
    embeddedBundles = GetBundleNamesFromLibrary(info.location);
  }
  if(!embeddedBundles.empty())
  {
    info.name = embeddedBundles.at(0);
    // remove this bundle name from the list to avoid an extra call to Register
    embeddedBundles.erase(embeddedBundles.begin());
  }
  
  std::string bundleKey = info.location + "/" + info.name;
  std::shared_ptr<Bundle> bundle = GetBundle(bundleKey);

  if (!bundle)
  {
    info.id = id++;
    assert(info.id > 0);
    bundle.reset(new Bundle(coreCtx, info));

    auto return_pair = (this->Lock(), bundles.insert(std::make_pair(bundleKey, bundle)));

    // A race condition exists when creating a new bundle instance. To resolve
    // this requires either scoping the mutex to the entire function or adding
    // additional logic to check for duplicates on insert into the bundle registry.
    // Otherwise, an "orphan" bundle instance could be returned to the caller,
    // which isn't actually contained within the bundle registry.
    //
    // Based on the bundle registry performance unit test, deleting the
    // orphaned bundle instance is faster than increasing the scope of the
    // mutex.
    if (!return_pair.second)
    {
      bundle = return_pair.first->second;
    }
  }
  if (!embeddedBundles.empty())
  {
    // register all the statically-linked bundles
    for (auto childBundleName : embeddedBundles)
    {
      Register(BundleInfo(info.location, childBundleName));
    }
    bundle->d->bundleManifest.SetValue(Bundle::PROP_STATIC_LINKED_BUNDLES, Any(embeddedBundles));
  }
  coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::INSTALLED, bundle));
  return bundle;
}

void BundleRegistry::UnRegister(const BundleInfo& info)
{
  // The system bundle cannot be uninstalled.
  if (info.id > 0)
  {
    this->Lock(), bundles.erase(info.location + "/" + info.name);
  }
}

std::shared_ptr<Bundle> BundleRegistry::GetBundle(long id) const
{
  if (id == 0) return coreCtx->systemBundle->shared_from_this();

  auto l = this->Lock(); US_UNUSED(l);

  for (auto& m : bundles)
  {
    if (m.second->GetBundleId() == id)
    {
      return m.second;
    }
  }
  return nullptr;
}

std::shared_ptr<Bundle> BundleRegistry::GetBundle(const std::string& location) const
{
  if (coreCtx->systemBundle->d->info.location == location) return coreCtx->systemBundle->shared_from_this();

  auto l = this->Lock(); US_UNUSED(l);

  auto iter = bundles.find(location);
  if (iter != bundles.end())
  {
    return iter->second;
  }
  return nullptr;
}
  
std::shared_ptr<Bundle> BundleRegistry::GetBundleByName(const std::string& name) const
{
  auto l = this->Lock(); US_UNUSED(l);
  
  for (auto& m : bundles)
  {
    if (m.second->GetName() == name)
    {
      return m.second;
    }
  }
  return 0;
}

std::vector<std::shared_ptr<Bundle>> BundleRegistry::GetBundles() const
{
  auto l = this->Lock(); US_UNUSED(l);

  std::vector<std::shared_ptr<Bundle>> result;
  result.push_back(coreCtx->systemBundle->shared_from_this());
  for (auto& m : bundles)
  {
    result.push_back(m.second);
  }
  return result;
}

}
