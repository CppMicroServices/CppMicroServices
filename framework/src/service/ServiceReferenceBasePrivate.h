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
        std::weak_ptr<BundlePrivate> const b;
        ServiceReferenceBase const sref;
        std::shared_ptr<S> const service;

        ServiceHolder(ServiceHolder&) = default;
        ServiceHolder(ServiceHolder&&) noexcept = default;
        ServiceHolder& operator=(ServiceHolder&) = delete;
        ServiceHolder& operator=(ServiceHolder&&) noexcept = delete;

        ServiceHolder(std::shared_ptr<BundlePrivate> const& b, ServiceReferenceBase const& sr, std::shared_ptr<S> s)
            : b(b)
            , sref(sr)
            , service(std::move(s))
        {
        }

        ~ServiceHolder()
        {
            try
            {
                sref.d.Load()->UngetService(b.lock(), true);
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
    };

    /* @brief Private helper struct used to facilitate the shared_ptr aliasing constructor
     *        in ServiceObjectsBase::GetService & ServiceObjectsBase::GetServiceInterfaceMap
     *        methods. The aliasing constructor helps automate the call to UngetService method.
     *
     *        Service consumers can simply call GetService to obtain a shared_ptr to the
     *        service object and not worry about calling UngetService when they are done.
     *        The UngetService is called when all instances of the returned shared_ptr object
     *        go out of scope.
     */
    struct UngetHelper
    {
        InterfaceMapConstPtr const interfaceMap;
        ServiceReferenceBase const sref;
        std::weak_ptr<BundlePrivate> const b;

        UngetHelper(UngetHelper&) = default;
        UngetHelper(UngetHelper&&) noexcept = default;
        UngetHelper& operator=(UngetHelper&) = delete;
        UngetHelper& operator=(UngetHelper&&) noexcept = delete;

        UngetHelper(InterfaceMapConstPtr im, ServiceReferenceBase const& sr, std::shared_ptr<BundlePrivate> const& b)
            : interfaceMap(std::move(im))
            , sref(sr)
            , b(b)
        {
        }
        ~UngetHelper()
        {
            try
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
            catch (...)
            {
                // Make sure that we don't crash if the shared_ptr service object outlives
                // the BundlePrivate or CoreBundleContext objects.
                if (!b.expired())
                {
                    DIAG_LOG(*b.lock()->coreCtx->sink)
                        << "UngetHelper threw an exception. " << util::GetLastExceptionStr();
                }
                // don't throw exceptions from the destructor. For an explanation, see:
                // https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md
                // Following this rule means that a FrameworkEvent isn't an option here
                // since it contains an exception object which clients could throw.
            }
        }
    };

    class MagicDeleterImpl : public MagicDeleter
    {
      public:
        MagicDeleterImpl(ServiceHolder<void>* sh) : sh_(sh), uh_(nullptr) {}
        MagicDeleterImpl(UngetHelper* uh) : sh_(nullptr), uh_(uh) {}

        void
        operator()(ServiceHolder<void>* sh)
        {
            delete sh;
        }

        void
        operator()(UngetHelper* uh)
        {
            delete uh;
        }

        [[nodiscard]] ServiceReferenceBase
        getServiceRef() const override
        {
            if (sh_)
            {
                return sh_->sref;
            }
            return uh_->sref;
        }

      private:
        ServiceHolder<void> const* const sh_;
        UngetHelper const* const uh_;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_SERVICEREFERENCEBASEPRIVATE_H
