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

#include "CoreBundleContext.h"
#include "ServiceRegistrationCoreInfo.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/ServiceInterface.h"
#include "cppmicroservices/detail/Log.h"
#include "cppmicroservices/util/Error.h"

#include "Properties.h"
#include "ServiceRegistrationLocks.h"
#include "cppmicroservices/ServiceReference.h"

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

    /* @brief Private helper struct used to facilitate the shared_ptr aliasing constructor
     *        in BundleContext::GetService method. The aliasing constructor helps automate
     *        the call to UngetService method.
     *
     *        Service consumers can simply call GetService to obtain a shared_ptr to the
     *        service object and not worry about calling UngetService when they are done.
     *        The UngetService is called when all instances of the returned shared_ptr object
     *        go out of scope.
     */
    template <class S>
    struct ServiceHolder
    {
        bool singletonService;
        std::weak_ptr<BundlePrivate> const b;
        ServiceReferenceBase const sref;
        std::shared_ptr<S> const service;
        InterfaceMapConstPtr const interfaceMap;

        ServiceHolder(ServiceHolder&) = default;
        ServiceHolder(ServiceHolder&&) noexcept = default;
        ServiceHolder& operator=(ServiceHolder&) = delete;
        ServiceHolder& operator=(ServiceHolder&&) noexcept = delete;

        ServiceHolder(std::shared_ptr<BundlePrivate> const& b,
                      ServiceReferenceBase const& sr,
                      std::shared_ptr<S> s,
                      InterfaceMapConstPtr im)
            : singletonService(s ? true : false)
            , b(b)
            , sref(sr)
            , service(std::move(s))
            , interfaceMap(std::move(im))
        {
        }

        ~ServiceHolder()
        {
            try
            {
                singletonService ? destroySingleton() : destroyPrototype();
            }
            catch (...)
            {
                // Make sure that we don't crash if the shared_ptr service object outlives
                // the BundlePrivate or CoreBundleContext objects.
                if (!b.expired())
                {
                    DIAG_LOG(*b.lock()->coreCtx->sink)
                        << "UngetService threw an exception. " << util::GetLastExceptionStr();
                }
                // don't throw exceptions from the destructor. For an explanation, see:
                // https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md
                // Following this rule means that a FrameworkEvent isn't an option here
                // since it contains an exception object which clients could throw.
            }
        }

      private:
        void
        destroySingleton()
        {
            sref.d.Load()->UngetService(b.lock(), true);
        }

        void
        destroyPrototype()
        {
            auto bundle = b.lock();
            if (sref)
            {
                bool isPrototypeScope
                    = sref.GetProperty(Constants::SERVICE_SCOPE).ToString() == Constants::SCOPE_PROTOTYPE;

                if (isPrototypeScope)
                {
                    sref.d.Load()->UngetPrototypeService(bundle, interfaceMap);
                }
                else
                {
                    sref.d.Load()->UngetService(bundle, true);
                }
            }
        }
    };

    /* @brief Private helper struct used to facilitate the retrieval of a serviceReference from
     *        a serviceObject.
     *
     *        Service consumers can pass a service to the public API ServiceReferenceFromService.
     *        This method can use the std::get_deleter method to retrieve this object and through
     *        it the original serviceReference.
     */
    class CustomServiceDeleter
    {
      public:
        CustomServiceDeleter(ServiceHolder<void>* sh) : sHolder(sh) {}

        void
        operator()(ServiceHolder<void>* sh)
        {
            delete sh;
        }

        [[nodiscard]] ServiceReferenceBase
        getServiceRef() const
        {
            return sHolder->sref;
        }

      private:
        ServiceHolder<void> const* const sHolder;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_SERVICEREFERENCEBASEPRIVATE_H
