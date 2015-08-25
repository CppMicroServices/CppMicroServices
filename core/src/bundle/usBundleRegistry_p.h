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

#ifndef USBUNDLEREGISTRY_P_H
#define USBUNDLEREGISTRY_P_H

#include <vector>
#include <string>

#include <usCoreConfig.h>
#include <usThreads_p.h>

US_BEGIN_NAMESPACE

class CoreBundleContext;
class Framework;
class Bundle;
struct BundleInfo;
struct BundleActivator;

/**
 * Here we handle all the bundles that are loaded in the framework.
 * @remarks This class is thread-safe.
 */
class US_Core_EXPORT BundleRegistry {

public:
  
  BundleRegistry(CoreBundleContext* coreCtx);
  virtual ~BundleRegistry(void);

  /**
   * Get the bundle that has the specified bundle identifier.
   *
   * @param id The identifier of the bundle to get.
   * @return Bundle or null
   *         if the bundle was not found.
   */
  Bundle* GetBundle(long id);

  /**
   * Get the bundle that has specified bundle name.
   *
   * @param name The name of the bundle to get.
   * @return Bundle or null.
   */
  Bundle* GetBundle(const std::string& name);

  /**
   * Get all known bundles.
   *
   * @return A list which is filled with all known bundles.
   */
  std::vector<Bundle*> GetBundles();

  /**
   * Register a bundle with the Framework
   *
   * @return The registered bundle.
   */
  Bundle* Register(BundleInfo* info);
  
  /**
   * Register the system bundle.
   *
   * A helper function to help bootstrap the Framework.
   *
   * @param systemBundle The system bundle to register.
   */
  void RegisterSystemBundle(Framework* const systemBundle, BundleInfo* info);

  /**
   * Remove a bundle from the Framework.
   *
   * Register(BundleInfo* info) must be called to re-install the bundle. 
   * Upon which, the bundle will receive a new unique bundle id.
   *
   */
  void UnRegister(const BundleInfo* info);

private:
  // don't allow copying the BundleRegistry.
  BundleRegistry(const BundleRegistry& );
  BundleRegistry& operator=(const BundleRegistry& );

  CoreBundleContext* coreCtx;

  typedef US_UNORDERED_MAP_TYPE<std::string, Bundle*> BundleMap;

  /**
   * Table of all installed bundles in this framework.
   * Key is the bundle name.
   */
  BundleMap bundles;

  /**
   * Lock for protecting the bundles object
   */
  Mutex* bundlesLock;

  /**
   * Lock for protecting the register count
   */
  Mutex* countLock;

  /**
   * Stores the next Bundle ID.
   */
  long id;

};

US_END_NAMESPACE

#endif // USBUNDLEREGISTRY_P_H
