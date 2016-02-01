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

#ifndef USBUNDLEPRIVATE_H
#define USBUNDLEPRIVATE_H

#include "usBundleVersion.h"
#include "usBundleInfo.h"
#include "usBundleManifest_p.h"
#include "usBundleResourceContainer_p.h"
#include "usSharedLibrary.h"

#include "usThreads_p.h"

#include <memory>

namespace us {

class CoreBundleContext;
class Bundle;
class BundleContext;
struct BundleActivator;

/**
 * \ingroup MicroServices
 */
class BundlePrivate : public MultiThreaded<> {

public:

  BundlePrivate(const BundlePrivate&) = delete;
  BundlePrivate& operator=(const BundlePrivate&) = delete;

  /**
   * Construct a new bundle based on a BundleInfo object.
   */
  BundlePrivate(const std::shared_ptr<Bundle>& qq, CoreBundleContext* coreCtx, BundleInfo* info);

  virtual ~BundlePrivate();

  void RemoveBundleResources();
  
  /**
   * Set the bundleContext member and update the cached instance in bundle code 
   * See US_INITIALIZE_BUNDLE
   */
  void SetBundleContext(BundleContext* context);

  CoreBundleContext* const coreCtx;

  /**
   * Bundle version
   */
  BundleVersion version;

  BundleInfo info;

  BundleResourceContainer resourceContainer;

  /**
   * BundleContext for the bundle
   */
  BundleContext* bundleContext;

  BundleActivator* bundleActivator;

  BundleManifest bundleManifest;

  std::string baseStoragePath;
  std::string storagePath;

  const std::weak_ptr<Bundle> q;

  /** 
   * Responsible for platform specific loading and unloading
   * of the bundle's physical form.
   */
  SharedLibrary lib;

private:

  void InitializeResources();

};

}

#endif // USBUNDLEPRIVATE_H
