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

#ifndef CPPMICROSERVICES_SERVICEREGISTRATIONCOREINFO_H
#define CPPMICROSERVICES_SERVICEREGISTRATIONCOREINFO_H

#include "cppmicroservices/ServiceInterface.h"
#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/detail/Threads.h"

#include "BundlePrivate.h"
#include "Properties.h"

#include <atomic>

namespace cppmicroservices
{

    /**
     * \ingroup MicroServices
     */
    class ServiceRegistrationCoreInfo final: public detail::MultiThreaded<>
    {

      public:
        ServiceRegistrationCoreInfo(BundlePrivate* bundle, InterfaceMapConstPtr service, Properties&& props);
        ~ServiceRegistrationCoreInfo() = default;

        ServiceRegistrationCoreInfo(ServiceRegistrationCoreInfo &&) = delete;
        ServiceRegistrationCoreInfo& operator=(ServiceRegistrationCoreInfo &&) = delete;

        ServiceRegistrationCoreInfo(ServiceRegistrationCoreInfo const&) = delete;
        ServiceRegistrationCoreInfo& operator=(ServiceRegistrationCoreInfo const&) = delete;

        using BundleToRefsMap = std::unordered_map<BundlePrivate*, int>;
        using BundleToServiceMap = std::unordered_map<BundlePrivate*, InterfaceMapConstPtr>;
        using BundleToServicesMap = std::unordered_map<BundlePrivate*, std::list<InterfaceMapConstPtr>>;

        /**
         * Service or ServiceFactory object.
         */
        InterfaceMapConstPtr service;
        /**
         * Bundles dependent on this service. Integer is used as
         * reference counter, counting number of unbalanced getService().
         */
        BundleToRefsMap dependents;

        /**
         * Object instances that a prototype factory has produced.
         */
        BundleToServicesMap prototypeServiceInstances;

        /**
         * Object instance with bundle scope that a factory may have produced.
         */
        BundleToServiceMap bundleServiceInstance;

        /**
         * Bundle registering this service.
         */
        std::weak_ptr<BundlePrivate> bundle_;

        /**
         * Service properties.
         */
        Properties properties;

        /**
         * Is service available. I.e., if <code>true</code> then holders
         * of a ServiceReference for the service are allowed to get it.
         */
        std::atomic<bool> available;

        /**
         * Avoid recursive unregistrations. I.e., if <code>true</code> then
         * unregistration of this service has started but is not yet
         * finished.
         */
        std::atomic<bool> unregistering;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_SERVICEREGISTRATIONCOREINFO_H