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
#include "usFrameworkPrivate.h"

namespace us {


const std::string Framework::PROP_STORAGE_LOCATION{ "org.cppmicroservices.framework.storage" };
const std::string Framework::PROP_THREADING_SUPPORT{ "org.cppmicroservices.framework.threading.support" };
const std::string Framework::PROP_LOG_LEVEL{ "org.cppmicroservices.framework.log.level" };

Framework::Framework(const BundleInfo& info, const std::map<std::string, Any>& configuration)
  : Bundle(std::unique_ptr<BundlePrivate>(new FrameworkPrivate(this, info, configuration)))
{
}

Framework::~Framework(void)
{
  // We are going down, make sure the code invoked by Stop()
  // can still create shared_ptr instances from this while
  // not deleting the Framework twice.
  std::shared_ptr<Bundle> dummy(this, [](Bundle*){});
  Stop();
}

void Framework::Start()
{
  // TODO framework states
  Bundle::Start();
}

void Framework::Stop()
{
  if (!this->IsStarted()) return;

  auto bundles = GetBundleContext()->GetBundles();
  for (auto bundle : bundles)
  {
    if (bundle->GetBundleId() > 0)
    {
      bundle->Stop();
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
  static_cast<FrameworkPrivate*>(d.get())->coreBundleContext.settings.SetAutoLoadingEnabled(enable);
}

}
