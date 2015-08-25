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
#include "usFrameworkPrivate_p.h"
#include "usBundleInfo.h"
#include "usBundleInitialization.h"
#include "usBundleSettings.h"
#include "usBundleUtils_p.h"
#include "usThreads_p.h"

US_BEGIN_NAMESPACE

Framework::Framework(void) : f(new FrameworkPrivate())
{

}

Framework::Framework(std::map<std::string, std::string>& configuration) :
    f(new FrameworkPrivate(configuration))
{
  
}

Framework::~Framework(void)
{

}

void Framework::init(void)
{
  MutexLock lock(*f->initLock);
  if (f->initialized)
  {
    return;
  }

  BundleInfo* bundleInfo = new BundleInfo(US_CORE_FRAMEWORK_NAME);

  void(Framework::*initFncPtr)(void) = &Framework::init;
  void* frameworkInit = NULL;
  std::memcpy(&frameworkInit, &initFncPtr, sizeof(void*));
  bundleInfo->location = BundleUtils::GetLibraryPath(frameworkInit);

  f->coreBundleContext->bundleRegistry.RegisterSystemBundle(this, bundleInfo);

  Bundle::Start();
  f->initialized = true;
}

void Framework::Start() 
{ 
  if(!f->initialized)
  {
    init();
  }
}

void Framework::Stop() 
{ 
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

  MutexLock lock(*f->initLock);
  f->initialized = false;
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
  f->coreBundleContext->settings.SetAutoLoadingEnabled(enable);
}

US_END_NAMESPACE
