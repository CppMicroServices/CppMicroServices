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

#ifndef CPPMICROSERVICES_SERVICEREGISTRATIONBASEPRIVATE_H
#define CPPMICROSERVICES_SERVICEREGISTRATIONBASEPRIVATE_H

#include "cppmicroservices/ServiceInterface.h"
#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/detail/Threads.h"

#include "Properties.h"
#include "ServiceRegistrationCoreInfo.h"

#include <atomic>

namespace cppmicroservices
{

    class BundlePrivate;
    class ServiceRegistrationBase;

    /**
     * \ingroup MicroServices
     */
    class ServiceRegistrationBasePrivate
        : public detail::MultiThreaded<>
        , public std::enable_shared_from_this<ServiceRegistrationBasePrivate>
    {

      protected:
        friend class ServiceRegistrationBase;

      public:
        using BundleToRefsMap = std::unordered_map<BundlePrivate*, int>;
        using BundleToServiceMap = std::unordered_map<BundlePrivate*, InterfaceMapConstPtr>;
        using BundleToServicesMap = std::unordered_map<BundlePrivate*, std::list<InterfaceMapConstPtr>>;

        ServiceRegistrationBasePrivate(ServiceRegistrationBasePrivate const&) = delete;
        ServiceRegistrationBasePrivate& operator=(ServiceRegistrationBasePrivate const&) = delete;

        /**
         * Reference object to this service registration.
         */
        ServiceReferenceBase reference;

        /**
         * Pointer to CoreInfo object for this registration.
         */
        std::shared_ptr<ServiceRegistrationCoreInfo> coreInfo;

        ServiceRegistrationBasePrivate(BundlePrivate* bundle, InterfaceMapConstPtr service, Properties&& props);

        ~ServiceRegistrationBasePrivate();

        // Avoid Weak Ptr errors creating reference
        void CreateReference();

        /**
         * Check if a bundle uses this service
         *
         * @param bundle Bundle to check
         * @return true if bundle uses this service
         */
        bool IsUsedByBundle(BundlePrivate* bundle) const;

        InterfaceMapConstPtr GetInterfaces() const;

        std::shared_ptr<void> GetService(std::string const& interfaceId) const;

        std::shared_ptr<void> GetService_unlocked(std::string const& interfaceId) const;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_SERVICEREGISTRATIONBASEPRIVATE_H
