/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

#ifndef CPPMICROSERVICES_BUNDLEINSTALLHOOK_H
#define CPPMICROSERVICES_BUNDLEINSTALLHOOK_H

#include "cppmicroservices/ServiceInterface.h"
#include "cppmicroservices/ShrinkableVector.h"

#include <memory>

namespace cppmicroservices
{

    class Bundle;
    class BundleContext;

    /**
     * @ingroup MicroServices
     *
     * %Bundle Context Hook Service.
     *
     * <p>
     * Bundles registering this service will be called during bundle install operations.
     *
     * @remarks Implementations of this interface are required to be thread-safe.
     */
    struct US_Framework_EXPORT BundleInstallHook
    {

        virtual ~BundleInstallHook();

        /**
         * Install hook method. This method is called for bundle install operations
         * using BundleContext::InstallBundles. The install method can
         * filter the result of the install operation.
         *
         *
         * @param context The bundle context of the bundle performing the find
         *        operation.
         * @param bundles A list of Bundles to be installed into the BundleRegistry on an install
         *        operation. The implementation of this method may remove
         *        bundles from the list to prevent the bundles from being
         *        installed.
         */
        virtual void Install(BundleContext const& context, ShrinkableVector<Bundle>& bundles) = 0;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_BUNDLEINSTALLHOOK_H
