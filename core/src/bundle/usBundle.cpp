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


#include "usBundle.h"

#include "usBundleEvent.h"
#include "usBundleContext.h"
#include "usBundleContextPrivate.h"
#include "usBundleActivator.h"
#include "usBundlePrivate.h"
#include "usBundleResource.h"
#include "usBundleSettings.h"
#include "usBundleUtils_p.h"
#include "usCoreBundleContext_p.h"

#include "usCoreConfig.h"

#include "usSharedLibrary.h"

#include <thread>
#include <chrono>

namespace us {

const std::string Bundle::PROP_ID{ "bundle.id" };
const std::string Bundle::PROP_NAME{ "bundle.name" };
const std::string Bundle::PROP_LOCATION{ "bundle.location" };
const std::string Bundle::PROP_VERSION{ "bundle.version" };
const std::string Bundle::PROP_VENDOR{ "bundle.vendor" };
const std::string Bundle::PROP_DESCRIPTION{ "bundle.description" };
const std::string Bundle::PROP_AUTOLOAD_DIR{ "bundle.autoload_dir" };
const std::string Bundle::PROP_AUTOINSTALLED_BUNDLES{ "bundle.autoinstalled_bundles" };

#if !defined(__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ < 7
  typedef std::chrono::monotonic_clock Clock;
#else
  typedef std::chrono::steady_clock Clock;
#endif

// TODO needs to be replaced with proper state handling
struct StateReset
{
  std::function<void()> f;
  StateReset(const std::function<void()>& f) : f(f) {}
  ~StateReset() { f(); }
};

Bundle::Bundle(CoreBundleContext* coreCtx, const BundleInfo& info)
  : d(new BundlePrivate(this, info))
{
  d->Init(coreCtx);
}

Bundle::Bundle(std::unique_ptr<BundlePrivate> d)
  : d(std::move(d))
{
}

Bundle::~Bundle()
{
}

bool Bundle::IsStarted() const
{
  // TODO use proper states
  return d->bundleContext.load() != nullptr;
}

void Bundle::Start()
{
  // TODO Use proper locks for state changes
  if (IsStarted()) return;
  if (d->starting.exchange(true)) return;

  StateReset reset([this]{ this->d->starting = false; });

  /// TODO replace with proper state handling
  auto tp = Clock::now();
  while (d->stopping)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - tp);
    if (duration.count() > 2000) throw std::runtime_error("Timeout while waiting to start bundle " + d->info.name);
  }

  d->coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::STARTING, this->shared_from_this()));

  typedef BundleActivator*(*CreateActivatorHook)(void);
  CreateActivatorHook createActivatorHook = nullptr;

  {
    auto l = d->Lock(); US_UNUSED(l);

    // loading a library isn't necessary if it isn't supported
#ifdef US_BUILD_SHARED_LIBS
    if(IsSharedLibrary(d->lib.GetFilePath()) && !d->lib.IsLoaded())
    {
      d->lib.Load();
    }
#endif

    d->bundleContext = new BundleContext(this->d.get());

    // save this bundle's context so that it can be accessible anywhere
    // from within this bundle's code.
    typedef void(*SetBundleContext)(BundleContext*);
    SetBundleContext setBundleContext = nullptr;

    std::string set_bundle_context_func = "_us_set_bundle_context_instance_" + d->info.name;
    void* setBundleContextSym = BundleUtils::GetSymbol(d->info, set_bundle_context_func.c_str());
    std::memcpy(&setBundleContext, &setBundleContextSym, sizeof(void*));

    if (setBundleContext)
    {
      setBundleContext(d->bundleContext);
    }

    std::string create_activator_func = "_us_create_activator_" + d->info.name;
    void* createActivatorHookSym = BundleUtils::GetSymbol(d->info, create_activator_func.c_str());
    std::memcpy(&createActivatorHook, &createActivatorHookSym, sizeof(void*));
  }

  // try to get a BundleActivator instance
  if (createActivatorHook)
  {

    try
    {
      d->bundleActivator = createActivatorHook();
      d->bundleActivator->Start(d->bundleContext);
    }
    catch (const std::exception& e)
    {
      d->bundleActivator = nullptr;
      this->Stop();
      throw std::runtime_error(std::string("Starting bundle " + d->info.name + " failed: " + e.what()));
    }

  }

  d->coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::STARTED, this->shared_from_this()));
}

void Bundle::Stop()
{
  if (d->bundleContext.load() == nullptr) return;
  if (d->stopping.exchange(true)) return;

  StateReset reset([this]{
    try
    {
      auto l = d->Lock(); US_UNUSED(l);
      d->coreCtx->listeners.HooksBundleStopped(d->bundleContext);
      d->RemoveBundleResources();
      d->bundleContext.load()->d->Lock(), d->bundleContextOrphans.emplace_back(d->bundleContext), (d->bundleContext.load()->d->bundle = nullptr);
      d->bundleContext = nullptr;
      d->coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::STOPPED, this->shared_from_this()));

      d->bundleActivator = nullptr;
    }
    catch (...) {}
    this->d->stopping = false;
  });

  // TODO replace with proper state handling
  auto tp = Clock::now();
  while (d->starting)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - tp);
    if (duration.count() > 2000) throw std::runtime_error("Timeout while waiting to stop bundle " + d->info.name);
  }

  d->coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::STOPPING, this->shared_from_this()));

  if (d->bundleActivator)
  {
    try
    {
      d->bundleActivator->Stop(d->bundleContext);
    }
    catch (const std::exception& e)
    {
      throw std::runtime_error("Stopping bundle " + d->info.name + " failed: " + e.what());
    }

    // delete the activator
    typedef void(*DestroyActivatorHook)(BundleActivator*);
    DestroyActivatorHook destroyActivatorHook = nullptr;
    std::string destroy_activator_func = "_us_destroy_activator_" + d->info.name;
    void* destroyActivatorHookSym = BundleUtils::GetSymbol(d->info, destroy_activator_func.c_str());
    std::memcpy(&destroyActivatorHook, &destroyActivatorHookSym, sizeof(void*));
    if (destroyActivatorHook)
    {
      destroyActivatorHook(d->bundleActivator);
    }
  }
}

void Bundle::Uninstall()
{
    Stop();
    d->coreCtx->bundleRegistry.UnRegister(d->info);
    d->coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::UNINSTALLED, this->shared_from_this()));
}

BundleContext* Bundle::GetBundleContext() const
{
  return d->bundleContext;
}

long Bundle::GetBundleId() const
{
  return d->info.id;
}

std::string Bundle::GetLocation() const
{
  return d->info.location;
}

std::string Bundle::GetName() const
{
  return d->info.name;
}

BundleVersion Bundle::GetVersion() const
{
  return d->version;
}

Any Bundle::GetProperty(const std::string& key) const
{
  Any property(d->bundleManifest.GetValue(key));

  // Clients must be able to query both a bundle's properties
  // and the framework's properties through any Bundle's
  // GetProperty function.
  // The Framework's properties include both the launch properties
  // used to initialize the Framework and all relevant
  // "org.cppmicroservices.*" properties.
  if (property.Empty())
  {
    auto props = d->coreCtx->frameworkProperties.find(key);
    if (props != d->coreCtx->frameworkProperties.end())
    {
      property = (*props).second;
    }
  }
  return property;
}

std::vector<std::string> Bundle::GetPropertyKeys() const
{
  return d->bundleManifest.GetKeys();
}

std::vector<ServiceReferenceU> Bundle::GetRegisteredServices() const
{
  std::vector<ServiceRegistrationBase> sr;
  std::vector<ServiceReferenceU> res;
  d->coreCtx->services.GetRegisteredByBundle(d.get(), sr);
  for (std::vector<ServiceRegistrationBase>::const_iterator i = sr.begin();
       i != sr.end(); ++i)
  {
    res.push_back(i->GetReference());
  }
  return res;
}

std::vector<ServiceReferenceU> Bundle::GetServicesInUse() const
{
  std::vector<ServiceRegistrationBase> sr;
  std::vector<ServiceReferenceU> res;
  d->coreCtx->services.GetUsedByBundle(const_cast<Bundle*>(this), sr);
  for (std::vector<ServiceRegistrationBase>::const_iterator i = sr.begin();
       i != sr.end(); ++i)
  {
    res.push_back(i->GetReference());
  }
  return res;
}

BundleResource Bundle::GetResource(const std::string& path) const
{
  if (!d->resourceContainer.IsValid())
  {
    return BundleResource();
  }
  BundleResource result(path, d->resourceContainer);
  if (result) return result;
  return BundleResource();
}

std::vector<BundleResource> Bundle::FindResources(const std::string& path, const std::string& filePattern,
                                                  bool recurse) const
{
  std::vector<BundleResource> result;
  if (!d->resourceContainer.IsValid())
  {
    return result;
  }

  std::string normalizedPath = path;
  // add a leading and trailing slash
  if (normalizedPath.empty()) normalizedPath.push_back('/');
  if (*normalizedPath.begin() != '/') normalizedPath = '/' + normalizedPath;
  if (*normalizedPath.rbegin() != '/') normalizedPath.push_back('/');
  d->resourceContainer.FindNodes(d->info.name + normalizedPath,
                                 filePattern.empty() ? "*" : filePattern,
                                 recurse, result);
  return result;
}

std::ostream& operator<<(std::ostream& os, const Bundle& bundle)
{
  os << "Bundle[" << "id=" << bundle.GetBundleId() <<
        ", loc=" << bundle.GetLocation() <<
        ", name=" << bundle.GetName() << "]";
  return os;
}

std::ostream& operator<<(std::ostream& os, Bundle const * bundle)
{
  return operator<<(os, *bundle);
}

}
