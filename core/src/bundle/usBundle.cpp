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
#include "usBundleActivator.h"
#include "usBundlePrivate.h"
#include "usBundleResource.h"
#include "usBundleSettings.h"
#include "usBundleUtils_p.h"
#include "usCoreBundleContext_p.h"

#include "usCoreConfig.h"

#include "usSharedLibrary.h"

namespace us {

const std::string Bundle::PROP_ID{ "bundle.id" };
const std::string Bundle::PROP_NAME{ "bundle.name" };
const std::string Bundle::PROP_LOCATION{ "bundle.location" };
const std::string Bundle::PROP_VERSION{ "bundle.version" };
const std::string Bundle::PROP_VENDOR{ "bundle.vendor" };
const std::string Bundle::PROP_DESCRIPTION{ "bundle.description" };
const std::string Bundle::PROP_AUTOLOAD_DIR{ "bundle.autoload_dir" };
const std::string Bundle::PROP_AUTOINSTALLED_BUNDLES{ "bundle.autoinstalled_bundles" };

Bundle::Bundle()
: d(0)
{

}

Bundle::~Bundle()
{
  delete d;
}

void Bundle::Init(CoreBundleContext* coreCtx,
                    BundleInfo* info)
{
  BundlePrivate* mp = new BundlePrivate(shared_from_this(), coreCtx, info);
  std::swap(mp, d);
  delete mp;
}

void Bundle::Uninit()
{
  if (d->bundleContext != NULL)
  {
    //d->coreCtx->listeners.HooksBundleStopped(d->bundleContext);
    d->RemoveBundleResources();
    delete d->bundleContext;
    d->bundleContext = nullptr;
    d->coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::STOPPED, shared_from_this()));

    d->bundleActivator = nullptr;
  }
}

bool Bundle::IsStarted() const
{
  return d->bundleContext != nullptr;
}

void Bundle::Start()
{
  BundlePrivate::Lock l(this->d);
  if (d->bundleContext)
  {
    US_WARN << "Bundle " << d->info.name << " already started.";
    return;
  }

  // loading a library isn't necessary if it isn't supported
#ifdef US_BUILD_SHARED_LIBS
  if(IsSharedLibrary(d->lib.GetFilePath()) && !d->lib.IsLoaded())
  {
    d->lib.Load();
  }
#endif

  d->bundleContext = new BundleContext(this->d);

  // save this bundle's context so that it can be accessible anywhere
  // from within this bundle's code.
  typedef void(*SetBundleContext)(BundleContext*);
  SetBundleContext setBundleContext = NULL;

  std::string set_bundle_context_func = "_us_set_bundle_context_instance_" + d->info.name;
  void* setBundleContextSym = BundleUtils::GetSymbol(d->info, set_bundle_context_func.c_str());
  std::memcpy(&setBundleContext, &setBundleContextSym, sizeof(void*));

  if (setBundleContext)
  {
    setBundleContext(d->bundleContext);
  }

  typedef BundleActivator*(*BundleActivatorHook)(void);
  BundleActivatorHook activatorHook = NULL;

  std::string activator_func = "_us_bundle_activator_instance_" + d->info.name;
  void* activatorHookSym = BundleUtils::GetSymbol(d->info, activator_func.c_str());
  std::memcpy(&activatorHook, &activatorHookSym, sizeof(void*));

  d->coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::STARTING, shared_from_this()));
  // try to get a BundleActivator instance

  if (activatorHook)
  {
    try
    {
      d->bundleActivator = activatorHook();
    }
    catch (...)
    {
      US_ERROR << "Creating the bundle activator of " << d->info.name << " failed";
      throw;
    }

    // This method should be "noexcept" and by not catching exceptions
    // here we semantically treat it that way since any exception during
    // static initialization will either terminate the program or cause
    // the dynamic loader to report an error.
    d->bundleActivator->Start(d->bundleContext);
  }

  d->coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::STARTED, shared_from_this()));
}

void Bundle::Stop()
{
  BundlePrivate::Lock l(this->d);
  if (d->bundleContext == nullptr)
  {
    US_WARN << "Bundle " << d->info.name << " already stopped.";
    return;
  }

  try
  {
    d->coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::STOPPING, shared_from_this()));

    if (d->bundleActivator)
    {
      d->bundleActivator->Stop(d->bundleContext);
    }
	// remove the cached bundle context. TODO : This block can be moved
	// to a setter method in BundlePrivate class.
	typedef void(*SetBundleContext)(BundleContext*);
	SetBundleContext setBundleContext = NULL;

	std::string set_bundle_context_func = "_us_set_bundle_context_instance_" + d->info.name;
	void* setBundleContextSym = BundleUtils::GetSymbol(d->info, set_bundle_context_func.c_str());
	std::memcpy(&setBundleContext, &setBundleContextSym, sizeof(void*));

	if (setBundleContext)
	{
		setBundleContext(NULL);
	}
  }
  catch (...)
  {
    US_WARN << "Calling the bundle activator Stop() method of " << d->info.name << " failed!";

    try
    {
      this->Uninit();
    }
    catch (...) {}

    throw;
  }

  this->Uninit();
}

void Bundle::Uninstall()
{
  Stop();
  d->coreCtx->bundleRegistry.UnRegister(&d->info);
  d->coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::UNINSTALLED, shared_from_this()));
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
    std::map<std::string, std::string>::iterator props = d->coreCtx->frameworkProperties.find(key);
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
  d->coreCtx->services.GetRegisteredByBundle(d, sr);
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
  d->coreCtx->services.GetUsedByBundle(std::const_pointer_cast<Bundle>(shared_from_this()), sr);
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

}

using namespace us;

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
