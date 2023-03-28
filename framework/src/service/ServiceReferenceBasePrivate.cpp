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
        : ref(1)
        , registration(reg)
    {
        // std::cout << "constructor: " << static_cast<void*>(this) << std::endl;
    }

    ServiceReferenceBasePrivate::~ServiceReferenceBasePrivate()
    {
        // std::cout << "destructor: " << static_cast<void*>(this) << std::endl;
        long use_count = registration.use_count();
        US_UNUSED(use_count);
    }

    InterfaceMapConstPtr
    ServiceReferenceBasePrivate::GetServiceFromFactory(BundlePrivate* bundle,
                                                       std::shared_ptr<ServiceFactory> const& factory)
    {
        assert(factory && "Factory service pointer is nullptr");
        InterfaceMapConstPtr s;
        try
        {
            InterfaceMapConstPtr smap
                = factory->GetService(MakeBundle(bundle->shared_from_this()), ServiceRegistrationBase(registration.lock()));
            if (!smap || smap->empty())
            {
                if (auto bundle_ = registration.lock()->bundle.lock())
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
            std::vector<std::string> classes
                = (registration.lock()->properties.Lock(),
                   any_cast<std::vector<std::string>>(
                       registration.lock()->properties.Value_unlocked(Constants::OBJECTCLASS).first));
            for (auto clazz : classes)
            {
                if (smap->find(clazz) == smap->end() && clazz != "org.cppmicroservices.factory")
                {
                    if (auto bundle_ = registration.lock()->bundle.lock())
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
            s = smap;
        }
        catch (cppmicroservices::SharedLibraryException const& ex)
        {
            if (auto bundle = registration.lock()->bundle.lock())
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
            s.reset();
            std::string message = "ServiceFactory threw an unknown exception.";
            if (auto bundle_ = registration.lock()->bundle.lock())
            {
                bundle_->coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(
                    FrameworkEvent::Type::FRAMEWORK_ERROR,
                    MakeBundle(bundle->shared_from_this()),
                    message,
                    std::make_exception_ptr(ServiceException(ex.what(), ServiceException::Type::FACTORY_EXCEPTION))));
            }
        }
        return s;
    }

    InterfaceMapConstPtr
    ServiceReferenceBasePrivate::GetPrototypeService(Bundle const& bundle)
    {
        InterfaceMapConstPtr s;
        {
            if (registration.lock()->available)
            {
                auto factory = std::static_pointer_cast<ServiceFactory>(
                    registration.lock()->GetService("org.cppmicroservices.factory"));
                s = GetServiceFromFactory(GetPrivate(bundle).get(), factory);
                registration.lock()->Lock(), registration.lock()->prototypeServiceInstances[GetPrivate(bundle).get()].push_back(s);
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
        if (!registration.lock()->available)
            return s;
        std::shared_ptr<ServiceFactory> serviceFactory;

        std::unordered_set<ServiceRegistrationBasePrivate*>* marks = nullptr;
        ServiceRegistrationBasePrivate* registrationPtr = registration.lock().get();
        struct Unmark
        {
            ~Unmark()
            {
                if (s)
                    s->erase(r);
            }
            std::unordered_set<ServiceRegistrationBasePrivate*>*& s;
            ServiceRegistrationBasePrivate* r;
        } unmark { marks, registrationPtr };
        US_UNUSED(unmark);

        {
            auto l = registration.lock()->Lock();
            US_UNUSED(l);
            if (!registration.lock()->available)
                return s;
            serviceFactory = std::static_pointer_cast<ServiceFactory>(
                registration.lock()->GetService_unlocked("org.cppmicroservices.factory"));

            auto res = registration.lock()->dependents.insert(std::make_pair(bundle, 0));
            auto& depCounter = res.first->second;

            // No service factory, just return the registered service directly.
            if (!serviceFactory)
            {
                s = registration.lock()->service;
                if (s && !s->empty())
                {
                    ++depCounter;
                }
                return s;
            }

            auto serviceIter = registration.lock()->bundleServiceInstance.find(bundle);
            if (registration.lock()->bundleServiceInstance.end() != serviceIter)
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

                if (auto bundle = registration.lock()->bundle.lock())
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
        s = GetServiceFromFactory(bundle, serviceFactory);

        auto l = registration.lock()->Lock();
        US_UNUSED(l);

        registration.lock()->dependents.insert(std::make_pair(bundle, 0));

        if (s && !s->empty())
        {
            // Insert a cached service object instance only if one isn't already cached. If another thread
            // already inserted a cached service object, discard the service object returned by
            // GetServiceFromFactory and return the cached one.
            auto insertResultPair = registration.lock()->bundleServiceInstance.insert(std::make_pair(bundle, s));
            s = insertResultPair.first->second;
            ++registration.lock()->dependents.at(bundle);
        }
        else
        {
            // If the service factory returned an invalid service object check the cache and return a valid one
            // if it exists.
            if (registration.lock()->bundleServiceInstance.end() != registration.lock()->bundleServiceInstance.find(bundle))
            {
                s = registration.lock()->bundleServiceInstance.at(bundle);
                ++registration.lock()->dependents.at(bundle);
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
            auto l = registration.lock()->Lock();
            US_UNUSED(l);
            auto iter = registration.lock()->prototypeServiceInstances.find(bundle.get());
            if (iter == registration.lock()->prototypeServiceInstances.end())
            {
                return false;
            }

            prototypeServiceMaps = iter->second;
            sf = std::static_pointer_cast<ServiceFactory>(
                registration.lock()->GetService_unlocked("org.cppmicroservices.factory"));
        }

        if (!sf)
            return false;

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
                    if (auto bundle_ = registration.lock()->bundle.lock())
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

                auto l = registration.lock()->Lock();
                US_UNUSED(l);
                auto iter = registration.lock()->prototypeServiceInstances.find(bundle.get());
                if (iter == registration.lock()->prototypeServiceInstances.end())
                    return true;

                auto serviceIter = std::find(iter->second.begin(), iter->second.end(), service);
                if (serviceIter != iter->second.end())
                    iter->second.erase(serviceIter);
                if (iter->second.empty())
                {
                    registration.lock()->prototypeServiceInstances.erase(iter);
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
            auto l = registration.lock()->Lock();
            US_UNUSED(l);
            auto depIter = registration.lock()->dependents.find(bundle.get());
            if (registration.lock()->dependents.end() == depIter)
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
                auto serviceIter = registration.lock()->bundleServiceInstance.find(bundle.get());
                if (registration.lock()->bundleServiceInstance.end() != serviceIter)
                {
                    sfi = serviceIter->second;
                }

                if (sfi && !sfi->empty())
                {
                    sf = std::static_pointer_cast<ServiceFactory>(
                        registration.lock()->GetService_unlocked("org.cppmicroservices.factory"));
                }
                registration.lock()->bundleServiceInstance.erase(bundle.get());
                registration.lock()->dependents.erase(bundle.get());
            }
        }

        if (sf && sfi && !sfi->empty())
        {
            try
            {
                sf->UngetService(MakeBundle(bundle), ServiceRegistrationBase(registration.lock()), sfi);
            }
            catch (std::exception const& ex)
            {
                if (auto bundle_ = registration.lock()->bundle.lock())
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
        return PropertiesHandle(registration.lock()->properties, true);
    }

    bool
    ServiceReferenceBasePrivate::IsConvertibleTo(std::string const& interfaceId) const
    {
        if (registration.lock())
        {
            auto l = registration.lock()->Lock();
            US_UNUSED(l);
            return registration.lock()->service ? registration.lock()->service->find(interfaceId) != registration.lock()->service->end()
                                         : false;
        }
        return false;
    }
} // namespace cppmicroservices
