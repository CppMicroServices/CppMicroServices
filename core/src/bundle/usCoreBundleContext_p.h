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


#ifndef USCOREBUNDLECONTEXT_H
#define USCOREBUNDLECONTEXT_H

#include "usServiceListeners_p.h"
#include "usServiceRegistry_p.h"
#include "usBundleHooks_p.h"
#include "usServiceHooks_p.h"
#include "usBundleRegistry_p.h"
#include "usBundleSettings.h"

namespace us {

/**
 * This class is not part of the public API.
 */
class CoreBundleContext
{
public:

  /**
   * All listeners in this framework.
   */
  ServiceListeners listeners;

  /**
   * All registered services in this framework.
   */
  ServiceRegistry services;

  /**
   * All service hooks.
   */
  ServiceHooks serviceHooks;

  /**
   * All bundle hooks.
   */
  BundleHooks bundleHooks;

  /**
   * All installed bundles.
   */
  BundleRegistry bundleRegistry;

  /**
   * This framework instance's settings
   */
  BundleSettings settings;

  /*
   * Framework properties, which contain both the
   * launch properties and the system properties.
   * See OSGi spec revision 6, section 4.2.2
   */
  std::map<std::string, std::string> frameworkProperties;

  /**
   * Contruct a core context
   *
   */
  CoreBundleContext();

  ~CoreBundleContext();

  void Init();

  void Uninit();

};

}

#endif // USCOREBUNDLECONTEXT_H
