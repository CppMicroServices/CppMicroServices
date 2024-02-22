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

#ifndef CPPMICROSERVICES_SERVICEREFERENCEBASEPRIVATE_H
#define CPPMICROSERVICES_SERVICEREFERENCEBASEPRIVATE_H

#include "ServiceRegistrationCoreInfo.h"
#include "cppmicroservices/ServiceInterface.h"

#include "Properties.h"
#include "ServiceRegistrationLocks.h"

#include <atomic>
#include <string>

namespace cppmicroservices
{

    class Any;
    class Bundle;
    class BundlePrivate;
    class PropertiesHandle;
    class ServiceRegistrationBasePrivate;
    class ServiceReferenceBasePrivate;

    /**
     * \ingroup MicroServices
     */
    class ServiceReferenceBasePrivate
    {
      public:
        ServiceReferenceBasePrivate(ServiceReferenceBasePrivate const&) = delete;
        ServiceReferenceBasePrivate& operator=(ServiceReferenceBasePrivate const&) = delete;

        ServiceReferenceBasePrivate(std::weak_ptr<ServiceRegistrationBasePrivate> reg);

        ~ServiceReferenceBasePrivate();

        ServiceRegistrationLocks LockServiceRegistration() const;

        /**
         * Get the service object.
         *
         * @param bundle requester of service.
         * @return Service requested or null in case of failure.
         */
        std::shared_ptr<void> GetService(BundlePrivate* bundle);

        InterfaceMapConstPtr GetServiceInterfaceMap(BundlePrivate* bundle);

        /**
         * Get new service instance.
         *
         * @param bundle requester of service.
         * @return Service requested or null in case of failure.
         */
        InterfaceMapConstPtr GetPrototypeService(Bundle const& bundle);

        /**
         * Unget the service object.
         *
         * @param bundle Bundle who wants remove service.
         * @param checkRefCounter If true decrement refence counter and remove service
         *                        if we reach zero. If false remove service without
         *                        checking refence counter.
         * @return True if service was removed or false if only reference counter was
         *         decremented.
         */
        bool UngetService(std::shared_ptr<BundlePrivate> const& bundle, bool checkRefCounter);

        /**
         * Unget prototype scope service objects.
         *
         * @param bundle Bundle who wants to remove a prototype scope service.
         * @param service The prototype scope service pointer.
         * @return \c true if the service was removed, \c false otherwise.
         */
        bool UngetPrototypeService(std::shared_ptr<BundlePrivate> const& bundle, InterfaceMapConstPtr const& service);

        /**
         * Get a handle to the locked service properties.
         *
         * @return A locked ServicePropertiesImpl handle object.
         */
        PropertiesHandle GetProperties() const;

        bool IsConvertibleTo(std::string const& interfaceId) const;

        /**
         * Link to registration object for this reference.
         */
        std::weak_ptr<ServiceRegistrationBasePrivate> const registration;

        /**
         * The service interface id for this reference.
         */
        std::string interfaceId;

        /**
         * Core Information for the service used by ServiceReferenceBasePrivate
         */
        std::shared_ptr<ServiceRegistrationCoreInfo> coreInfo;

      private:
        InterfaceMapConstPtr GetServiceFromFactory(BundlePrivate* bundle,
                                                   std::shared_ptr<ServiceFactory> const& factory);
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_SERVICEREFERENCEBASEPRIVATE_H
