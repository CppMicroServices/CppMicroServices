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


US_BEGIN_NAMESPACE

BundleRegistry::BundleRegistry(CoreBundleContext* coreCtx) :
    coreCtx(coreCtx),
    bundlesLock(new Mutex()),
    countLock(new Mutex()),
    id(0)
{

}

BundleRegistry::~BundleRegistry(void)
{
    if (bundlesLock)
    {
        delete bundlesLock;
    }

    if (countLock)
    {
        delete countLock;
    }

    id = 0;
}

Bundle* BundleRegistry::Register(BundleInfo* info)
{
  Bundle* bundle = GetBundle(info->name);

  if (!bundle)
  {
    bundle = new Bundle();
    countLock->Lock();
    info->id = id++;
    assert(info->id == 0 ? info->name == "CppMicroServices" : true);
    countLock->Unlock();
    bundle->Init(coreCtx, info);

    MutexLock lock(*bundlesLock);
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
      BundleMap::iterator iter(return_pair.first);
      delete bundle;
      bundle = (*iter).second;
    }    
  }

  return bundle;
}

void BundleRegistry::RegisterSystemBundle(Framework* const systemBundle, BundleInfo* info)
{
  if (!systemBundle)
  {
    throw std::invalid_argument("Can't register a null system bundle");
  }

  countLock->Lock();
  info->id = id++;
  assert(info->id == 0 ? info->name == "CppMicroServices" : true);
  countLock->Unlock();

  systemBundle->Init(coreCtx, info);

  MutexLock lock(*bundlesLock);
  bundles.insert(std::make_pair(info->name, systemBundle));
}

void BundleRegistry::UnRegister(const BundleInfo* info)
{
  // There is no use checking for >= 0 as the System Bundle's ID is 0 
  // and it can't be uninstalled.
  if (info->id >= 1)
  {
    MutexLock lock(*bundlesLock);
    bundles.erase(info->name);
  }
}

Bundle* BundleRegistry::GetBundle(long id)
{
  MutexLock lock(*bundlesLock);

  BundleMap::const_iterator iter = bundles.begin();
  BundleMap::const_iterator iterEnd = bundles.end();
  for (; iter != iterEnd; ++iter)
  {
    if (iter->second->GetBundleId() == id)
    {
      return iter->second;
    }
  }
  return 0;
}

Bundle* BundleRegistry::GetBundle(const std::string& name)
{
  MutexLock lock(*bundlesLock);

  BundleMap::const_iterator iter = bundles.find(name);
  if (iter != bundles.end())
  {
    return iter->second;
  }
  return 0;
}

std::vector<Bundle*> BundleRegistry::GetBundles()
{
  MutexLock lock(*bundlesLock);

  std::vector<Bundle*> result;
  BundleMap::const_iterator iter = bundles.begin();
  BundleMap::const_iterator iterEnd = bundles.end();
  for (; iter != iterEnd; ++iter)
  {
    result.push_back(iter->second);
  }
  return result;
}

US_END_NAMESPACE

