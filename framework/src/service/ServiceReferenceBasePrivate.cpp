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

#include "ServiceReferenceBasePrivate.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/ServiceException.h"
#include "cppmicroservices/ServiceFactory.h"
#include "cppmicroservices/SharedLibraryException.h"

#include "BundlePrivate.h"
#include "CoreBundleContext.h"
#include "ServiceRegistrationBasePrivate.h"
#include "ServiceRegistry.h"

#include <cassert>

US_MSVC_DISABLE_WARNING(4503) // decorated name length exceeded, name was truncated

namespace cppmicroservices
{

    using ThreadMarksMapType = std::unordered_map<BundlePrivate*, std::unordered_set<ServiceRegistrationBasePrivate*>>;

    ServiceReferenceBasePrivate::ServiceReferenceBasePrivate(std::weak_ptr<ServiceRegistrationBasePrivate> reg)
        : registration(reg)
    {
        if (auto reg = registration.lock(); reg)
        {
            coreInfo = reg->coreInfo;
        }
    }

    ServiceReferenceBasePrivate::~ServiceReferenceBasePrivate() = default;

    ServiceRegistrationLocks
    ServiceReferenceBasePrivate::LockServiceRegistration() const
    {
        return ServiceRegistrationLocks(registration.lock(), coreInfo);
    }

    InterfaceMapConstPtr
    ServiceReferenceBasePrivate::GetServiceFromFactory(BundlePrivate* bundle,
                                                       std::shared_ptr<ServiceFactory> const& factory)
    {
        assert(factory && "Factory service pointer is nullptr");
        try
        {
            InterfaceMapConstPtr smap = factory->GetService(MakeBundle(bundle->shared_from_this()),
                                                            ServiceRegistrationBase(registration.lock()));
            if (!smap || smap->empty())
            {
                if (auto bundle_ = coreInfo->bundle_.lock())
                {
                    std::string message = "ServiceFactory returned an empty or nullptr interface map.";
                    bundle_->coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(
                        FrameworkEvent::Type::FRAMEWORK_ERROR,
                        MakeBundle(bundle->shared_from_this()),
                        message,
                        std::make_exception_ptr(ServiceException(message, ServiceException::Type::FACTORY_ERROR))));
                    return smap;
                }
            }
            {
                auto l = coreInfo->properties.Lock();
                US_UNUSED(l);
                for (auto const& clazz : ref_any_cast<std::vector<std::string>>(
                         coreInfo->properties.ValueByRef_unlocked(Constants::OBJECTCLASS)))
                {
                    if (smap->find(clazz) == smap->end() && clazz != "org.cppmicroservices.factory")
                    {
                        if (auto bundle_ = coreInfo->bundle_.lock())
                        {
                            std::string message("ServiceFactory produced an object that did not implement: " + clazz);
                            bundle_->coreCtx->listeners.SendFrameworkEvent(
                                FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING,
                                               MakeBundle(bundle->shared_from_this()),
                                               message,
                                               std::make_exception_ptr(std::logic_error(message.c_str()))));
                        }
                        return nullptr;
                    }
                }
            }
            return smap;
        }
        catch (cppmicroservices::SharedLibraryException const& ex)
        {
            if (auto bundle = coreInfo->bundle_.lock())
            {
                bundle->coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_ERROR,
                                                                             ex.GetBundle(),
                                                                             "Failed to load shared library",
                                                                             std::current_exception()));
            }
            throw;
        }
        catch (cppmicroservices::SecurityException const& ex)
        {
            if (bundle)
            {
                bundle->coreCtx->listeners.SendFrameworkEvent(
                    FrameworkEvent { FrameworkEvent::Type::FRAMEWORK_ERROR,
                                     ex.GetBundle(),
                                     std::string("Failed to load shared library due to a security exception"),
                                     std::current_exception() });
            }
            throw;
        }
        catch (std::exception const& ex)
        {
            std::string message = "ServiceFactory threw an unknown exception.";
            if (auto bundle_ = coreInfo->bundle_.lock())
            {
                bundle_->coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(
                    FrameworkEvent::Type::FRAMEWORK_ERROR,
                    MakeBundle(bundle->shared_from_this()),
                    message,
                    std::make_exception_ptr(ServiceException(ex.what(), ServiceException::Type::FACTORY_EXCEPTION))));
            }
        }
        return nullptr;
    }

    InterfaceMapConstPtr
    ServiceReferenceBasePrivate::GetPrototypeService(Bundle const& bundle)
    {
        InterfaceMapConstPtr s;
        {
            auto reg = registration.lock();
            if (coreInfo->available && reg)
            {
                auto factory
                    = std::static_pointer_cast<ServiceFactory>(reg->GetService("org.cppmicroservices.factory"));
                s = GetServiceFromFactory(GetPrivate(bundle).get(), factory);
                LockServiceRegistration(), coreInfo->prototypeServiceInstances[GetPrivate(bundle).get()].push_back(s);
            }
        }
        return s;
    }

    std::shared_ptr<void>
    ServiceReferenceBasePrivate::GetService(BundlePrivate* bundle)
    {
        return ExtractInterface(GetServiceInterfaceMap(bundle), interfaceId);
    }

    InterfaceMapConstPtr
    ServiceReferenceBasePrivate::GetServiceInterfaceMap(BundlePrivate* bundle)
    {
        /*
         * Detect recursive service factory calls. For each thread, this
         * map contains an entry for each bundle that tries to get a service
         * from a service factory.
         */
#ifdef US_HAVE_THREAD_LOCAL
        static thread_local ThreadMarksMapType threadMarks;
#elif !defined(US_ENABLE_THREADING_SUPPORT)
        static ThreadMarksMapType threadMarks;
#else
        // A non-static map will never lead to the detection of
        // recursive service factory calls.
        ThreadMarksMapType threadMarks;
#endif

        InterfaceMapConstPtr s;
        if (!coreInfo->available)
        {
            return s;
        }
        std::shared_ptr<ServiceFactory> serviceFactory;

        std::unordered_set<ServiceRegistrationBasePrivate*>* marks = nullptr;
        ServiceRegistrationBasePrivate* registrationPtr = registration.lock().get();
        struct Unmark
        {
            ~Unmark()
            {
                if (s)
                {
                    s->erase(r);
                }
            }
            std::unordered_set<ServiceRegistrationBasePrivate*>*& s;
            ServiceRegistrationBasePrivate* r;
        } unmark { marks, registrationPtr };
        US_UNUSED(unmark);

        {
            auto reg = registration.lock();
            if (!reg)
            {
                return s;
            }
            auto l = LockServiceRegistration();
            US_UNUSED(l);
            if (!coreInfo->available)
            {
                return s;
            }
            serviceFactory
                = std::static_pointer_cast<ServiceFactory>(reg->GetService_unlocked("org.cppmicroservices.factory"));

            auto res = coreInfo->dependents.insert(std::make_pair(bundle, 0));
            auto& depCounter = res.first->second;

            // No service factory, just return the registered service directly.
            if (!serviceFactory)
            {
                s = coreInfo->service;
                if (s && !s->empty())
                {
                    ++depCounter;
                }
                return s;
            }

            auto serviceIter = coreInfo->bundleServiceInstance.find(bundle);
            if (coreInfo->bundleServiceInstance.end() != serviceIter)
            {
                ++depCounter;
                return serviceIter->second;
            }

            marks = &threadMarks[bundle];
            if (marks->find(registrationPtr) != marks->end())
            {
                // Prevent recursive service factory calls from the same thread
                // for the same bundle.
                std::string msg = "Recursive call to ServiceFactory::GetService";
                auto fwEvent = FrameworkEvent(
                    FrameworkEvent::Type::FRAMEWORK_ERROR,
                    MakeBundle(bundle->shared_from_this()),
                    msg,
                    std::make_exception_ptr(ServiceException(msg, ServiceException::FACTORY_RECURSION)));

                if (auto bundle = coreInfo->bundle_.lock())
                {
                    bundle->coreCtx->listeners.SendFrameworkEvent(fwEvent);
                }

                return nullptr;
            }

            marks->insert(registrationPtr);
        }

        // Calling into a service factory could cause re-entrancy into the
        // framework and even, theoretically, into this function. Ensuring
        // we don't hold a lock while calling into the service factory eliminates
        // the possibility of a deadlock. It does not however eliminate the
        // possibility of infinite recursion.

        {
            auto l = LockServiceRegistration();
            US_UNUSED(l);

            if (coreInfo->bundleServiceInstance.end() != coreInfo->bundleServiceInstance.find(bundle))
            {
                ++coreInfo->dependents.at(bundle);
                return coreInfo->bundleServiceInstance.at(bundle);
            }
        }
        s = GetServiceFromFactory(bundle, serviceFactory);

        {
            auto l = LockServiceRegistration();
            US_UNUSED(l);

            coreInfo->dependents.insert(std::make_pair(bundle, 0));

            if (s && !s->empty())
            {
                // Insert a cached service object instance only if one isn't already cached. If another thread
                // already inserted a cached service object, discard the service object returned by
                // GetServiceFromFactory and return the cached one.
                auto insertResultPair = coreInfo->bundleServiceInstance.insert(std::make_pair(bundle, s));
                s = insertResultPair.first->second;
                ++coreInfo->dependents.at(bundle);
            }
        }
        return s;
    }

    bool
    ServiceReferenceBasePrivate::UngetPrototypeService(std::shared_ptr<BundlePrivate> const& bundle,
                                                       InterfaceMapConstPtr const& service)
    {
        std::list<InterfaceMapConstPtr> prototypeServiceMaps;
        std::shared_ptr<ServiceFactory> sf;

        {
            auto reg = registration.lock();
            auto l = LockServiceRegistration();
            US_UNUSED(l);
            auto iter = coreInfo->prototypeServiceInstances.find(bundle.get());
            if (iter == coreInfo->prototypeServiceInstances.end())
            {
                return false;
            }

            prototypeServiceMaps = iter->second;
            sf = std::static_pointer_cast<ServiceFactory>(reg->GetService_unlocked("org.cppmicroservices.factory"));
        }

        if (!sf)
        {
            return false;
        }

        for (auto& prototypeServiceMap : prototypeServiceMaps)
        {
            // compare the contents of the map
            if (*service.get() == *prototypeServiceMap.get())
            {
                try
                {
                    sf->UngetService(MakeBundle(bundle), ServiceRegistrationBase(registration.lock()), service);
                }
                catch (std::exception const& ex)
                {
                    if (auto bundle_ = coreInfo->bundle_.lock())
                    {
                        std::string message("ServiceFactory threw an exception");
                        bundle_->coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(
                            FrameworkEvent::Type::FRAMEWORK_ERROR,
                            MakeBundle(bundle->shared_from_this()),
                            message,
                            std::make_exception_ptr(
                                ServiceException(ex.what(), ServiceException::Type::FACTORY_EXCEPTION))));
                    }
                }

                auto l = LockServiceRegistration();
                US_UNUSED(l);
                auto iter = coreInfo->prototypeServiceInstances.find(bundle.get());
                if (iter == coreInfo->prototypeServiceInstances.end())
                {
                    return true;
                }

                auto serviceIter = std::find(iter->second.begin(), iter->second.end(), service);
                if (serviceIter != iter->second.end())
                {
                    iter->second.erase(serviceIter);
                }
                if (iter->second.empty())
                {
                    coreInfo->prototypeServiceInstances.erase(iter);
                }
                return true;
            }
        }

        return false;
    }

    bool
    ServiceReferenceBasePrivate::UngetService(std::shared_ptr<BundlePrivate> const& bundle, bool checkRefCounter)
    {
        bool hadReferences = false;
        bool removeService = false;
        InterfaceMapConstPtr sfi;
        std::shared_ptr<ServiceFactory> sf;

        {
            auto reg = registration.lock();
            auto l = LockServiceRegistration();
            US_UNUSED(l);
            auto depIter = coreInfo->dependents.find(bundle.get());
            if (coreInfo->dependents.end() == depIter)
            {
                return hadReferences && removeService;
            }

            int& count = depIter->second;
            if (count > 0)
            {
                hadReferences = true;
            }

            if (checkRefCounter)
            {
                if (count > 1)
                {
                    --count;
                }
                else if (count == 1)
                {
                    removeService = true;
                }
            }
            else
            {
                removeService = true;
            }

            if (removeService)
            {
                auto serviceIter = coreInfo->bundleServiceInstance.find(bundle.get());
                if (coreInfo->bundleServiceInstance.end() != serviceIter)
                {
                    sfi = serviceIter->second;
                }

                if (sfi && !sfi->empty() && reg)
                {
                    sf = std::static_pointer_cast<ServiceFactory>(
                        reg->GetService_unlocked("org.cppmicroservices.factory"));
                }
                coreInfo->bundleServiceInstance.erase(bundle.get());
                coreInfo->dependents.erase(bundle.get());
            }
        }

        if (sf && sfi && !sfi->empty())
        {
            try
            {
                if (auto reg = registration.lock(); reg)
                {
                    sf->UngetService(MakeBundle(bundle), ServiceRegistrationBase(registration.lock()), sfi);
                }
            }
            catch (std::exception const& ex)
            {
                if (auto bundle_ = coreInfo->bundle_.lock())
                {
                    std::string message("ServiceFactory threw an exception");
                    bundle_->coreCtx->listeners.SendFrameworkEvent(
                        FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_ERROR,
                                       MakeBundle(bundle->shared_from_this()),
                                       message,
                                       std::make_exception_ptr(
                                           ServiceException(ex.what(), ServiceException::Type::FACTORY_EXCEPTION))));
                }
            }
        }

        return hadReferences && removeService;
    }

    PropertiesHandle
    ServiceReferenceBasePrivate::GetProperties() const
    {
        return PropertiesHandle(coreInfo->properties, true);
    }

    bool
    ServiceReferenceBasePrivate::IsConvertibleTo(std::string const& interfaceId) const
    {
        if (auto reg = registration.lock(); reg)
        {
            auto l = LockServiceRegistration();
            US_UNUSED(l);
            return coreInfo->service ? coreInfo->service->find(interfaceId) != coreInfo->service->end() : false;
        }
        return false;
    }

    ServiceReferenceU
    ServiceReferenceFromService(std::shared_ptr<void> const& s)
    {
        auto deleter = std::get_deleter<CustomServiceDeleter>(s);
        if (!deleter)
        {
            throw std::runtime_error("The input is not a CppMicroServices managed ServiceObject");
        }
        return deleter->getServiceRef();
    }
} // namespace cppmicroservices
