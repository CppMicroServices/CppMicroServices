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

#include "usBundlePrivate.h"

#include "usBundle.h"
#include "usBundleContext.h"
#include "usBundleActivator.h"
#include "usBundleUtils_p.h"
#include "usBundleSettings.h"
#include "usBundleResource.h"
#include "usBundleResourceStream.h"
#include "usCoreBundleContext_p.h"
#include "usServiceRegistration.h"
#include "usServiceReferenceBasePrivate.h"
#include "usGetBundleContext.h"
#include <algorithm>
#include <iterator>
#include <cassert>
#include <cstring>

namespace us {


BundlePrivate::BundlePrivate(const std::shared_ptr<Bundle>& qq, 
	                         CoreBundleContext* coreCtx,
                             BundleInfo* info)
  : coreCtx(coreCtx)
  , info(*info)
  , resourceContainer(info)
  , bundleContext(0)
  , bundleActivator(0)
  , q(qq)
  , lib(info->location)
{
  // Check if the bundle provides a manifest.json file and if yes, parse it.
  if (resourceContainer.IsValid())
  {
    BundleResource manifestRes("/manifest.json", resourceContainer);
    if (manifestRes)
    {
      BundleResourceStream manifestStream(manifestRes);
      try
      {
        bundleManifest.Parse(manifestStream);
      }
      catch (const std::exception& e)
      {
        US_ERROR << "Parsing of manifest.json for bundle " << info->location << " failed: " << e.what();
      }
    }
  }

  // Check if we got version information and validate the version identifier
  if (bundleManifest.Contains(Bundle::PROP_VERSION))
  {
    Any versionAny = bundleManifest.GetValue(Bundle::PROP_VERSION);
    std::string errMsg;
    if (versionAny.Type() != typeid(std::string))
    {
      errMsg = std::string("The version identifier must be a string");
    }
    try
    {
      version = BundleVersion(versionAny.ToString());
    }
    catch (const std::exception& e)
    {
      errMsg = std::string("The version identifier is invalid: ") + e.what();
    }

    if (!errMsg.empty())
    {
      throw std::invalid_argument(std::string("The Json value for ") + Bundle::PROP_VERSION + " for bundle " +
                                  info->location + " is not valid: " + errMsg);
    }
  }

  std::stringstream propId;
  propId << this->info.id;
  bundleManifest.SetValue(Bundle::PROP_ID, propId.str());
  bundleManifest.SetValue(Bundle::PROP_LOCATION, this->info.location);

  if (!bundleManifest.Contains(Bundle::PROP_NAME))
  {
    throw std::runtime_error(Bundle::PROP_NAME + " is not defined in the bundle manifest.");
  }

  Any bundleName(bundleManifest.GetValue(Bundle::PROP_NAME));
  if (bundleName.Empty())
  {
    throw std::runtime_error(Bundle::PROP_NAME + " is empty in the bundle manifest.");
  }

  this->info.name = bundleName.ToString();

  if (bundleManifest.Contains(Bundle::PROP_AUTOLOAD_DIR))
  {
    this->info.autoLoadDir = bundleManifest.GetValue(Bundle::PROP_AUTOLOAD_DIR).ToString();
  }
  else
  {
    this->info.autoLoadDir = this->info.name;
    bundleManifest.SetValue(Bundle::PROP_AUTOLOAD_DIR, Any(this->info.autoLoadDir));
  }

  std::vector<std::string> installedBundleNames;
#if defined(US_ENABLE_AUTOLOADING_SUPPORT) && defined(US_BUILD_SHARED_LIBS)
  if (coreCtx->settings.IsAutoLoadingEnabled())
  {
    installedBundleNames = AutoInstallBundles(this->info, this->coreCtx);
  }
#endif
  
  for (auto childBundleName : info->embeddedBundles)
  {
    BundleInfo* childBundleInfo = new BundleInfo(info->location, childBundleName);
	if (GetBundleContext() != NULL)
	{
		GetBundleContext()->InstallBundle(childBundleInfo);
	}
	else
	{
		// framework hasn't been started yet, so not posting install events
		this->coreCtx->bundleRegistry.Register(childBundleInfo);
	}
    
    installedBundleNames.push_back(childBundleName);
  }
  
  if (!installedBundleNames.empty())
  {
    bundleManifest.SetValue(Bundle::PROP_AUTOINSTALLED_BUNDLES, Any(installedBundleNames));
  }
}

BundlePrivate::~BundlePrivate()
{
  delete bundleContext;
}

void BundlePrivate::RemoveBundleResources()
{
  coreCtx->listeners.RemoveAllListeners(bundleContext);

  std::vector<ServiceRegistrationBase> srs;
  coreCtx->services.GetRegisteredByBundle(this, srs);
  for (std::vector<ServiceRegistrationBase>::iterator i = srs.begin();
       i != srs.end(); ++i)
  {
    try
    {
      i->Unregister();
    }
    catch (const std::logic_error& /*ignore*/)
    {
      // Someone has unregistered the service after stop completed.
      // This should not occur, but we don't want get stuck in
      // an illegal state so we catch it.
    }
  }

  srs.clear();
  coreCtx->services.GetUsedByBundle(q.lock(), srs);
  for (std::vector<ServiceRegistrationBase>::const_iterator i = srs.begin();
       i != srs.end(); ++i)
  {
    i->GetReference(std::string()).d->UngetService(q.lock(), false);
  }
}

}
