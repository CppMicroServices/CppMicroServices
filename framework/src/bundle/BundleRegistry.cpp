/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

#include "BundleRegistry.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/GetBundleContext.h"

#include "cppmicroservices/util/Error.h"
#include "cppmicroservices/util/String.h"

#include "BundleContextPrivate.h"
#include "BundlePrivate.h"
#include "BundleResourceContainer.h"
#include "BundleStorage.h"
#include "CoreBundleContext.h"
#include "FrameworkPrivate.h"

#include <cassert>
#include <map>

namespace cppmicroservices {

// Helper class to ensure RAII for InitialBundleMap in case where Install0 throws
class InitialBundleMapCleanup {
public:
  InitialBundleMapCleanup(std::function<void()> cleanupFcn) : _cleanupFcn(std::move(cleanupFcn)) {}
  ~InitialBundleMapCleanup() {
	_cleanupFcn();
  }
private:
  std::function<void()> _cleanupFcn;
};

BundleRegistry::BundleRegistry(CoreBundleContext* coreCtx)
  : coreCtx(coreCtx)
{}

BundleRegistry::~BundleRegistry() = default;

void BundleRegistry::Init()
{
  bundles.v.insert(
    std::make_pair(coreCtx->systemBundle->location, coreCtx->systemBundle));
}

void BundleRegistry::Clear()
{
  auto l = bundles.Lock();
  US_UNUSED(l);
  bundles.v.clear();
}

/*
  This function acquires a lock and decrements the reference count
  for the specified bundle. If this decrement causes the count to be
  0, it is erased from the initialBundleInstallMap map.
*/
void BundleRegistry::DecrementInitialBundleMapRef(
  cppmicroservices::detail::MutexLockingStrategy<>::UniqueLock& l,
  const std::string& location)
{
  l.Lock();
  initialBundleInstallMap[location].first--;
  if (initialBundleInstallMap[location].first == 0) {
    initialBundleInstallMap.erase(location);
  }
  l.UnLock();
}

/*
  This function populates the res and alreadyInstalled vectors with the
  appropriate entries so that they can be used by the Install0 call. This was
  extracted from Install() for convenience.
*/
void BundleRegistry::GetAlreadyInstalledBundlesAtLocation(
  std::pair<BundleMap::iterator, BundleMap::iterator> foundBundles,
  std::vector<Bundle>& resultingBundles,
  std::vector<std::shared_ptr<BundlePrivate>>& alreadyInstalled)
{
  while (foundBundles.first != foundBundles.second) {
    auto installedBundlePrivate = foundBundles.first->second;
    alreadyInstalled.push_back(installedBundlePrivate);
    auto actualBundle = coreCtx->bundleHooks.FilterBundle(
      MakeBundleContext(installedBundlePrivate->bundleContext.Load()), MakeBundle(installedBundlePrivate));
    if (actualBundle) {
      resultingBundles.push_back(actualBundle);
    }
    ++foundBundles.first;
  }
}

std::vector<Bundle> BundleRegistry::Install(const std::string& location,
                                            BundlePrivate* caller)
{
  using namespace std::chrono_literals;

  CheckIllegalState();

  // Grab the lock for the BundleRegistry object so that we can
  // read into the map without any data races
  auto l = this->Lock();
  US_UNUSED(l);

  // Search the multimap for the current bundle location
  auto bundlesAtLocationRange = (bundles.Lock(), bundles.v.equal_range(location));

  /*
    If the bundle is already installed, then execute the regular
    install process. In this case, there are no data races to worry about.

    If the bundle isn't installed, then one of two things can happen: 1) either
    the current thread is the first thread trying to install this bundle or 2) the
    current thread is trying to install a bundle that is not installed which another
    thread is currently installing.

    If 1): Create an entry in the initialBundleInstallMap which keeps track of whether
      or not a given bundle is being installed for the first time. After creating this
      entry, the thread performs the install and notifies all threads waiting on that
      install to finish so that they too can perform an install (but in the context of it
      being already installed).

    If 2): Increment the reference counter for the bundle in the initialBundleInstallMap.
      This ensures that when we decrement the count after the installing thread is done,
      the map entry isn't deleted since the current thread needs access to the
      condition_variable and boolean flag. Once the installing thread notifies the thread
      that it is able to continue with it's install, it goes ahead and performs the regular
      install procedure.
  */
  if (bundlesAtLocationRange.first != bundlesAtLocationRange.second) {
    l.UnLock();

    std::vector<Bundle> resultingBundles;
    std::vector<std::shared_ptr<BundlePrivate>> alreadyInstalled;
    // Populate the res and alreadyInstalled vectors with the appropriate data
    // based on what bundles are already installed
    GetAlreadyInstalledBundlesAtLocation(bundlesAtLocationRange, resultingBundles, alreadyInstalled);

    // Perform the install
    auto newBundles = Install0(location, alreadyInstalled, caller);
    resultingBundles.insert(resultingBundles.end(), newBundles.begin(), newBundles.end());
    if (resultingBundles.empty()) {
      throw std::runtime_error("All bundles rejected by a bundle hook");
    } else {
      return resultingBundles;
    }
  } else {
    // Check to see if another thread is currently in the process of installing this
    // bundle for the first time.
    auto alreadyInstallingIterator = initialBundleInstallMap.find(location);

    /*
      If no other thread is installing the desired bundle, create a map entry,
      install the bundle, and notify all other threads wanting to install this bundle
      that it is safe to do so. If the current thread is not the installing thread, then
      it will increment the reference count in the initialBundleInstallMap so that the map
      entry isn't prematurely deleted, wait to be notified that the install thread is done,
      and then perform the regular install procedure.
    */
    if (alreadyInstallingIterator == initialBundleInstallMap.end()) {
      // Insert entry into the initialBundleInstallMap to prevent other threads from
      // trying to install this uninstalled bundle at the same time
      auto pairToInsert = std::make_pair(uint32_t(1), WaitCondition{});
      initialBundleInstallMap.insert(
        std::make_pair(location, std::move(pairToInsert)));
      l.UnLock();

      std::vector<Bundle> installedBundles;
      
      {
        // create instance of clean-up object to ensure RAII
        InitialBundleMapCleanup cleanup([this, &l, &location](){
          {
            // Notify all waiting threads that it is safe to install the bundle
            std::lock_guard<std::mutex> lock(*(initialBundleInstallMap[location].second.m));
            initialBundleInstallMap[location].second.waitFlag = false;
            initialBundleInstallMap[location].second.cv->notify_all();
          }
          DecrementInitialBundleMapRef(l, location);
        });
          
        // Perform the install
        installedBundles = Install0(location, {}, caller);
      }

      return installedBundles;
    } else {
      initialBundleInstallMap[location].first++;
      
      {
        // Wait for the install thread to notify this thread that it is safe
        // to install the current bundle
        std::unique_lock<std::mutex> lock(
          *(initialBundleInstallMap[location].second.m));

        // This while loop exists to prevent a known race condition. If the installing thread notifies before
        // another thread trying to install the same bundle reaches this wait, it will wait indefinitely. To
        // fix this, a wait_for is used intead (which utilizes a timeout to avoid this race) and the wait statement
        // as a whole acts as the while statement's predicate; once the timeout is reached, wait_for exits and the
        // statement is re-evaluated since it would have returned false.
        while (!initialBundleInstallMap[location].second.cv->wait_for(lock, 0.1ms, [&location, this] {
          return !initialBundleInstallMap[location].second.waitFlag;
          }));
      }
      
      // Re-acquire the range because while this thread was waiting, the installing
      // thread made a modification to bundles.v
      bundlesAtLocationRange = (bundles.Lock(), bundles.v.equal_range(location));
      l.UnLock();

      std::vector<Bundle> resultingBundles;
      std::vector<std::shared_ptr<BundlePrivate>> alreadyInstalled;
      GetAlreadyInstalledBundlesAtLocation(bundlesAtLocationRange, resultingBundles, alreadyInstalled);

      std::vector<Bundle> newBundles;
      
      {
        // create instance of clean-up object to ensure RAII
        InitialBundleMapCleanup cleanup([this, &l, &location](){
          DecrementInitialBundleMapRef(l, location);
        });

        // Perform the install
        newBundles = Install0(location, alreadyInstalled, caller);
      }

      resultingBundles.insert(resultingBundles.end(), newBundles.begin(), newBundles.end());
      if (resultingBundles.empty()) {
        throw std::runtime_error("All bundles rejected by a bundle hook");
      } else {
        return resultingBundles;
      }
    }
  }
}

std::vector<Bundle> BundleRegistry::Install0(
  const std::string& location,
  const std::vector<std::shared_ptr<BundlePrivate>>& exclude,
  BundlePrivate* /*caller*/)
{
  std::vector<Bundle> res;
  std::vector<std::shared_ptr<BundleArchive>> barchives;
  try {
    if (exclude.empty()) {
      barchives = coreCtx->storage->InsertBundleLib(location);
    } else {
      auto resCont =
        exclude.front()->GetBundleArchive()->GetResourceContainer();
      auto entries = resCont->GetTopLevelDirs();
      for (auto const& b : exclude) {
        entries.erase(
          std::remove(entries.begin(), entries.end(), b->symbolicName),
          entries.end());
      }
      barchives = coreCtx->storage->InsertArchives(resCont, entries);
    }

    for (auto& ba : barchives) {
      auto d = std::shared_ptr<BundlePrivate>(
        new BundlePrivate(coreCtx, std::move(ba)));
      res.emplace_back(MakeBundle(d));
    }

    {
      auto l = bundles.Lock();
      US_UNUSED(l);
      for (auto& b : res) {
        bundles.v.insert(std::make_pair(location, b.d));
      }
    }

    for (auto& b : res) {
      coreCtx->listeners.BundleChanged(
        BundleEvent(BundleEvent::BUNDLE_INSTALLED, b));
    }
    return res;
  } catch (...) {
    for (auto& ba : barchives) {
      ba->Purge();
    }
    throw std::runtime_error("Failed to install bundle library at " + location +
                             ": " + util::GetLastExceptionStr());
  }
}

void BundleRegistry::Remove(const std::string& location, long id)
{
  auto l = bundles.Lock();
  US_UNUSED(l);
  auto range = bundles.v.equal_range(location);
  for (auto iter = range.first; iter != range.second; ++iter) {
    if (iter->second->id == id) {
      bundles.v.erase(iter);
      return;
    }
  }
}

std::shared_ptr<BundlePrivate> BundleRegistry::GetBundle(long id) const
{
  CheckIllegalState();

  auto l = bundles.Lock();
  US_UNUSED(l);

  for (auto& m : bundles.v) {
    if (m.second->id == id) {
      return m.second;
    }
  }
  return nullptr;
}

std::vector<std::shared_ptr<BundlePrivate>> BundleRegistry::GetBundles(
  const std::string& location) const
{
  CheckIllegalState();

  auto l = bundles.Lock();
  US_UNUSED(l);

  auto range = bundles.v.equal_range(location);
  std::vector<std::shared_ptr<BundlePrivate>> result;
  std::transform(range.first,
                 range.second,
                 std::back_inserter(result),
                 [](const BundleMap::value_type& p) { return p.second; });
  return result;
}

std::vector<std::shared_ptr<BundlePrivate>> BundleRegistry::GetBundles(
  const std::string& name,
  const BundleVersion& version) const
{
  CheckIllegalState();

  std::vector<std::shared_ptr<BundlePrivate>> res;

  auto l = bundles.Lock();
  US_UNUSED(l);

  for (auto& p : bundles.v) {
    auto& b = p.second;
    if (name == b->symbolicName && version == b->version) {
      res.push_back(b);
    }
  }

  return res;
}

std::vector<std::shared_ptr<BundlePrivate>> BundleRegistry::GetBundles() const
{
  auto l = bundles.Lock();
  US_UNUSED(l);

  std::vector<std::shared_ptr<BundlePrivate>> result;
  std::transform(bundles.v.begin(),
                 bundles.v.end(),
                 std::back_inserter(result),
                 [](const BundleMap::value_type& p) { return p.second; });
  return result;
}

std::vector<std::shared_ptr<BundlePrivate>> BundleRegistry::GetActiveBundles()
  const
{
  CheckIllegalState();
  std::vector<std::shared_ptr<BundlePrivate>> result;

  auto l = bundles.Lock();
  US_UNUSED(l);
  for (auto& b : bundles.v) {
    auto s = b.second->state.load();
    if (s == Bundle::STATE_ACTIVE || s == Bundle::STATE_STARTING) {
      result.push_back(b.second);
    }
  }
  return result;
}

void BundleRegistry::Load()
{
  auto l = this->Lock();
  US_UNUSED(l);
  auto bas = coreCtx->storage->GetAllBundleArchives();
  for (auto const& ba : bas) {
    try {
      std::shared_ptr<BundlePrivate> impl(new BundlePrivate(coreCtx, ba));
      bundles.v.insert(std::make_pair(impl->location, impl));
    } catch (...) {
      ba->SetAutostartSetting(-1); // Do not start on launch
      std::cerr << "Failed to load bundle " << util::ToString(ba->GetBundleId())
                << " (" + ba->GetBundleLocation() + ") uninstalled it!"
                << " (execption: "
                << util::GetExceptionStr(std::current_exception()) << ")"
                << std::endl;
    }
  }
}

void BundleRegistry::CheckIllegalState() const
{
  if (coreCtx == nullptr) {
    throw std::logic_error("This framework instance is not active.");
  }
}
}
