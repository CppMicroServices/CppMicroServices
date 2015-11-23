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

#include <cassert>
#include <map>


namespace us {

BundleRegistry::BundleRegistry(CoreBundleContext* coreCtx) :
  coreCtx(coreCtx)
{
  id.value = 0;
}

BundleRegistry::~BundleRegistry(void)
{
}

std::shared_ptr<Bundle> BundleRegistry::Register(BundleInfo* info)
{
  std::shared_ptr<Bundle> bundle(GetBundle(info->name));

  if (!bundle)
  {
    bundle = std::shared_ptr<Bundle>(new Bundle());
    {
      Lock l(id);
      info->id = id.value++;
      assert(info->id == 0 ? info->name == "CppMicroServices" : true);
    }
    bundle->Init(coreCtx, info);

    Lock l(this);
    std::pair<BundleMap::iterator, bool> return_pair(bundles.insert(std::make_pair(info->name, bundle)));

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

  return bundle;
}

void BundleRegistry::RegisterSystemBundle(std::shared_ptr<Framework> systemBundle, BundleInfo* info)
{
  if (!systemBundle)
  {
    throw std::invalid_argument("Can't register a null system bundle");
  }

  {
    Lock l(id);
    info->id = id.value++;
    assert(info->id == 0 ? info->name == "CppMicroServices" : true);
  }

  systemBundle->Init(coreCtx, info);

  Lock l(this);
  bundles.insert(std::make_pair(info->name, systemBundle));
}

void BundleRegistry::UnRegister(const BundleInfo* info)
{
  // The system bundle cannot be uninstalled.
  if (info->id >= 1)
  {
    Lock l(this);
    bundles.erase(info->name);
  }
}

std::shared_ptr<Bundle> BundleRegistry::GetBundle(long id) const
{
  Lock l(this);

  for (auto const& m : bundles)
  {
    if (m.second->GetBundleId() == id)
    {
      return m.second;
    }
  }
  return nullptr;
}

std::shared_ptr<Bundle> BundleRegistry::GetBundle(const std::string& name) const
{
  Lock l(this);

  auto iter = bundles.find(name);
  if (iter != bundles.end())
  {
    return iter->second;
  }
  return nullptr;
}

std::vector<std::shared_ptr<Bundle>> BundleRegistry::GetBundles() const
{
  Lock l(this);

  std::vector<std::shared_ptr<Bundle>> result;
  for (auto& m : bundles)
  {
    result.push_back(m.second);
  }
  return result;
}

}
