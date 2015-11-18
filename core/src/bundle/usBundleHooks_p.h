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

#ifndef USBUNDLEHOOKS_P_H
#define USBUNDLEHOOKS_P_H

#include "usServiceListeners_p.h"

#include <vector>
#include <memory>

namespace us {

class CoreBundleContext;
class Bundle;
class BundleContext;
class BundleEvent;

class BundleHooks
{

private:

  CoreBundleContext* const coreCtx;

public:

  BundleHooks(CoreBundleContext* ctx);

  std::shared_ptr<Bundle> FilterBundle(const BundleContext* mc, const std::shared_ptr<Bundle>& bundle) const;

  void FilterBundles(const BundleContext* mc, std::vector<std::shared_ptr<Bundle>>& bundles) const;

  void FilterBundleEventReceivers(const BundleEvent& evt,
                                  ServiceListeners::BundleListenerMap& bundleListeners);

};

}

#endif // USBUNDLEHOOKS_P_H
