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

#include "cppmicroservices/BundleContext.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/util/Error.h"
#include "cppmicroservices/util/FileSystem.h"

#include "BundleContextPrivate.h"
#include "BundlePrivate.h"
#include "BundleRegistry.h"
#include "CoreBundleContext.h"
#include "ServiceReferenceBasePrivate.h"
#include "ServiceRegistry.h"

#include <cstdio>
#include <memory>
#include <utility>

namespace cppmicroservices
{

    namespace
    {
        std::shared_ptr<BundlePrivate>
        GetAndCheckBundlePrivate(std::shared_ptr<BundleContextPrivate> const& d)
        {
            auto b = (d->Lock(), d->bundle.lock());
            if (!b)
            {
                throw std::runtime_error("The bundle context is no longer valid");
            }

            return b;
        }
    } // namespace

    BundleContext::BundleContext(std::shared_ptr<BundleContextPrivate> ctx) : d(std::move(ctx)) {}

    BundleContext::BundleContext() = default;

    bool
    BundleContext::operator==(BundleContext const& rhs) const
    {
        return *this ? (rhs ? d == rhs.d : false) : !rhs;
    }

    bool
    BundleContext::operator!=(BundleContext const& rhs) const
    {
        return !(*this == rhs);
    }

    bool
    BundleContext::operator<(BundleContext const& rhs) const
    {
        return *this ? (rhs ? (d < rhs.d) : true) : false;
    }

    BundleContext::
    operator bool() const
    {
        return d && d->IsValid();
    }

    BundleContext&
    BundleContext::operator=(std::nullptr_t)
    {
        d = nullptr;
        return *this;
    }

    Any
    BundleContext::GetProperty(std::string const& key) const
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        auto iter = b->coreCtx->frameworkProperties.find(key);
        return iter == b->coreCtx->frameworkProperties.end() ? Any() : iter->second;
    }

    AnyMap
    BundleContext::GetProperties() const
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        return b->coreCtx->frameworkProperties;
    }

    Bundle
    BundleContext::GetBundle() const
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        return MakeBundle(b);
    }

    Bundle
    BundleContext::GetBundle(long id) const
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        return b->coreCtx->bundleHooks.FilterBundle(*this, MakeBundle(b->coreCtx->bundleRegistry.GetBundle(id)));
    }

    std::vector<Bundle>
    BundleContext::GetBundles(std::string const& location) const
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        std::vector<Bundle> res;
        for (auto bu : b->coreCtx->bundleRegistry.GetBundles(location))
        {
            res.emplace_back(MakeBundle(bu));
        }
        return res;
    }

    std::vector<Bundle>
    BundleContext::GetBundles() const
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        std::vector<Bundle> bus;
        for (auto bu : b->coreCtx->bundleRegistry.GetBundles())
        {
            bus.emplace_back(MakeBundle(bu));
        }
        b->coreCtx->bundleHooks.FilterBundles(*this, bus);
        return bus;
    }

    ServiceRegistrationU
    BundleContext::RegisterService(InterfaceMapConstPtr const& service, ServiceProperties const& properties)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        return b->coreCtx->services.RegisterService(b.get(), service, properties);
    }

    std::vector<ServiceReferenceU>
    BundleContext::GetServiceReferences(std::string const& clazz, std::string const& filter)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        std::vector<ServiceReferenceBase> refs;
        b->coreCtx->services.Get(clazz, filter, b.get(), refs);
        return std::vector<ServiceReferenceU>(refs.begin(), refs.end());
    }

    ServiceReferenceU
    BundleContext::GetServiceReference(std::string const& clazz)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        return b->coreCtx->services.Get(b.get(), clazz);
    }

    std::shared_ptr<void>
    BundleContext::GetService(ServiceReferenceBase const& reference)
    {
        if (!reference)
        {
            throw std::invalid_argument("Default constructed ServiceReference is not a "
                                        "valid input to GetService()");
        }

        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);
        std::cout << "MARK1" << std::endl;
        auto serviceHolder = new ServiceHolder<void>(b, reference, reference.d.Load()->GetService(b.get()), nullptr);
        std::cout << "MARK1.5" << std::endl;
        std::shared_ptr<ServiceHolder<void>> h(serviceHolder, CustomServiceDeleter { serviceHolder });
        return std::shared_ptr<void>(h, h->service.get());
    }

    InterfaceMapConstPtr
    BundleContext::GetService(ServiceReferenceU const& reference)
    {
        if (!reference)
        {
            throw std::invalid_argument("Default constructed ServiceReference is not a "
                                        "valid input to GetService()");
        }

        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        auto serviceInterfaceMap = reference.d.Load()->GetServiceInterfaceMap(b.get());
        std::shared_ptr<ServiceHolder<InterfaceMap const>> h(
            new ServiceHolder<InterfaceMap const>(b, reference, serviceInterfaceMap, nullptr));
        return InterfaceMapConstPtr(h, h->service.get());
    }

    ListenerToken
    BundleContext::AddServiceListener(ServiceListener const& delegate, std::string const& filter)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        return b->coreCtx->listeners.AddServiceListener(d, delegate, nullptr, filter);
    }

    void
    BundleContext::RemoveServiceListener(ServiceListener const& delegate)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        b->coreCtx->listeners.RemoveServiceListener(d, ListenerTokenId(0), delegate, nullptr);
    }

    ListenerToken
    BundleContext::AddBundleListener(BundleListener const& delegate)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        return b->coreCtx->listeners.AddBundleListener(d, delegate, nullptr);
    }

    void
    BundleContext::RemoveBundleListener(BundleListener const& delegate)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        b->coreCtx->listeners.RemoveBundleListener(d, delegate, nullptr);
    }

    ListenerToken
    BundleContext::AddFrameworkListener(FrameworkListener const& listener)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        return b->coreCtx->listeners.AddFrameworkListener(d, listener, nullptr);
    }

    void
    BundleContext::RemoveFrameworkListener(FrameworkListener const& listener)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        b->coreCtx->listeners.RemoveFrameworkListener(d, listener, nullptr);
    }

    ListenerToken
    BundleContext::AddServiceListener(ServiceListener const& delegate, void* data, std::string const& filter)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        return b->coreCtx->listeners.AddServiceListener(d, delegate, data, filter);
    }

    void
    BundleContext::RemoveServiceListener(ServiceListener const& delegate, void* data)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        b->coreCtx->listeners.RemoveServiceListener(d, ListenerTokenId(0), delegate, data);
    }

    ListenerToken
    BundleContext::AddBundleListener(BundleListener const& delegate, void* data)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        return b->coreCtx->listeners.AddBundleListener(d, delegate, data);
    }

    void
    BundleContext::RemoveBundleListener(BundleListener const& delegate, void* data)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        b->coreCtx->listeners.RemoveBundleListener(d, delegate, data);
    }

    void
    BundleContext::RemoveListener(ListenerToken token)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        b->coreCtx->listeners.RemoveListener(d, std::move(token));
    }

    std::string
    BundleContext::GetDataFile(std::string const& filename) const
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        std::string dataRoot = b->bundleDir;
        if (!dataRoot.empty())
        {
            if (!util::Exists(dataRoot))
            {
                util::MakePath(dataRoot);
            }
            return dataRoot + util::DIR_SEP + filename;
        }
        return std::string();
    }

    std::vector<Bundle>
    BundleContext::InstallBundles(std::string const& location, cppmicroservices::AnyMap const& bundleManifest)
    {
        if (!d)
        {
            throw std::runtime_error("The bundle context is no longer valid");
        }

        d->CheckValid();
        auto b = GetAndCheckBundlePrivate(d);

        return b->coreCtx->bundleRegistry.Install(location, b.get(), bundleManifest);
    }

} // namespace cppmicroservices
