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

#include "usFramework.h"

#include "usCoreBundleContext_p.h"
#include "usFrameworkPrivate.h"
#include "usBundleInfo.h"
#include "usBundleInitialization.h"
#include "usBundleSettings.h"
#include "usBundleUtils_p.h"
#include "usThreads_p.h"

namespace us {

    const std::string Framework::PROP_STORAGE_LOCATION{ "org.cppmicroservices.framework.storage" };
    const std::string Framework::PROP_THREADING_SUPPORT{ "org.cppmicroservices.framework.threading.support" };
    const std::string Framework::PROP_LOG_LEVEL{ "org.cppmicroservices.framework.log.level" };

    Framework::Framework(void) : d(new FrameworkPrivate())
    {

    }

    Framework::Framework(std::map<std::string, std::string>& configuration) :
        d(new FrameworkPrivate(configuration))
    {
  
    }

    Framework::~Framework(void)
    {

    }

    void Framework::Initialize(void)
    {
      FrameworkPrivate::Lock{d};
      if (d->initialized)
      {
        return;
      }

      BundleInfo* bundleInfo = new BundleInfo(US_CORE_FRAMEWORK_NAME);

      void(Framework::*initFncPtr)(void) = &Framework::Initialize;
      void* frameworkInit = NULL;
      std::memcpy(&frameworkInit, &initFncPtr, sizeof(void*));
      bundleInfo->location = BundleUtils::GetLibraryPath(frameworkInit);

      d->coreBundleContext.bundleRegistry.RegisterSystemBundle(this, bundleInfo);

      d->initialized = true;
    }

    void Framework::Start() 
    { 
      Initialize();
      Bundle::Start();
    }

    void Framework::Stop() 
    {
      FrameworkPrivate::Lock lock(d);
      std::vector<Bundle*> bundles(GetBundleContext()->GetBundles());
      for (std::vector<Bundle*>::const_iterator iter = bundles.begin();
          iter != bundles.end(); 
          ++iter)
      {
        if ((*iter)->GetName() != US_CORE_FRAMEWORK_NAME)
        {
          (*iter)->Stop();
        }
      }

      Bundle::Stop();
    }

    void Framework::Uninstall() 
    {
      throw std::runtime_error("Cannot uninstall a system bundle."); 
    }

    std::string Framework::GetLocation() const
    {
      // OSGi Core release 6, section 4.6:
      //  The system bundle GetLocation method returns the string: "System Bundle"
      return std::string("System Bundle");
    }

    void Framework::SetAutoLoadingEnabled(bool enable)
    {
      d->coreBundleContext.settings.SetAutoLoadingEnabled(enable);
    }

}
