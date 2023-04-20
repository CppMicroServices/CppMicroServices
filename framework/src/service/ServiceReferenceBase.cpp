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

#include "cppmicroservices/ServiceReferenceBase.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/Constants.h"

#include "BundlePrivate.h"
#include "ServiceReferenceBasePrivate.h"
#include "ServiceRegistrationBasePrivate.h"
#include <cassert>

namespace cppmicroservices
{

    ServiceReferenceBase::ServiceReferenceBase()
        : d(new ServiceReferenceBasePrivate(std::weak_ptr<ServiceRegistrationBasePrivate>()))
    {
    }

    ServiceReferenceBase::ServiceReferenceBase(ServiceReferenceBase const& ref) : d(std::atomic_load(&ref.d)) {}

    ServiceReferenceBase::ServiceReferenceBase(std::shared_ptr<ServiceRegistrationBasePrivate> reg)
        : d(new ServiceReferenceBasePrivate(reg))
    {
    }

    void
    ServiceReferenceBase::SetInterfaceId(std::string const& interfaceId)
    {
        d = std::make_shared<ServiceReferenceBasePrivate>(std::atomic_load(&d)->registration);
        std::atomic_load(&d)->interfaceId = interfaceId;
    }

    ServiceReferenceBase::operator bool() const { return static_cast<bool>(GetBundle()); }

    ServiceReferenceBase&
    ServiceReferenceBase::operator=(std::nullptr_t)
    {
        d = std::make_shared<ServiceReferenceBasePrivate>(std::weak_ptr<ServiceRegistrationBasePrivate>());
        return *this;
    }

    ServiceReferenceBase::~ServiceReferenceBase() {}

    Any
    ServiceReferenceBase::GetProperty(std::string const& key) const
    {
        auto l = std::atomic_load(&d)->coreInfo->properties.Lock();
        US_UNUSED(l);
        return std::atomic_load(&d)->coreInfo->properties.Value_unlocked(key).first;
    }

    void
    ServiceReferenceBase::GetPropertyKeys(std::vector<std::string>& keys) const
    {
        keys = GetPropertyKeys();
    }

    std::vector<std::string>
    ServiceReferenceBase::GetPropertyKeys() const
    {
        auto l = std::atomic_load(&d)->coreInfo->properties.Lock();
        US_UNUSED(l);
        return std::atomic_load(&d)->coreInfo->properties.Keys_unlocked();
    }

    Bundle
    ServiceReferenceBase::GetBundle() const
    {
        auto p = std::atomic_load(&d);
        if (p->registration.expired())
        {
            return Bundle();
        }

        auto l = p->registration.lock()->Lock();
        US_UNUSED(l);
        auto l1 = p->coreInfo->Lock();
        US_UNUSED(l1);
        if (p->coreInfo->bundle.lock() == nullptr)
        {
            return Bundle();
        }
        return MakeBundle(p->coreInfo->bundle.lock()->shared_from_this());
    }

    std::vector<Bundle>
    ServiceReferenceBase::GetUsingBundles() const
    {
        std::vector<Bundle> bundles;
        for (auto const& iter : (std::atomic_load(&d)->coreInfo->dependents))
        {
            bundles.push_back(MakeBundle(iter.first->shared_from_this()));
        }
        return bundles;
    }

    bool
    ServiceReferenceBase::operator<(ServiceReferenceBase const& reference) const
    {
        if (std::atomic_load(&d) == std::atomic_load(&reference.d))
        {
            return false;
        }

        if (!(*this))
        {
            return true;
        }

        if (!reference)
        {
            return false;
        }

        if (std::atomic_load(&d)->registration.lock() == std::atomic_load(&reference.d)->registration.lock())
        {
            return false;
        }

        /// A deadlock caused by mutex order locking will happen if these two scoped blocks
        /// are combined into one. Multiple threads can enter this function as a result of
        /// adding/removing ServiceReferenceBase objects from STL containers. If that occurs
        /// AND these two scoped blocks are combined into one such that both "Lock()" calls
        /// happen in the same scoped block, sporadic deadlocks will occur.
        Any anyR1;
        Any anyId1;
        {
            auto l1 = std::atomic_load(&d)->coreInfo->properties.Lock();
            US_UNUSED(l1);
            anyR1 = std::atomic_load(&d)->coreInfo->properties.Value_unlocked(Constants::SERVICE_RANKING).first;
            assert(anyR1.Empty() || anyR1.Type() == typeid(int));
            anyId1 = std::atomic_load(&d)->coreInfo->properties.Value_unlocked(Constants::SERVICE_ID).first;
            assert(anyId1.Empty() || anyId1.Type() == typeid(long int));
        }

        Any anyR2;
        Any anyId2;
        {
            auto l2 = std::atomic_load(&reference.d)->coreInfo->properties.Lock();
            US_UNUSED(l2);
            anyR2
                = std::atomic_load(&reference.d)->coreInfo->properties.Value_unlocked(Constants::SERVICE_RANKING).first;
            assert(anyR2.Empty() || anyR2.Type() == typeid(int));
            anyId2 = std::atomic_load(&reference.d)->coreInfo->properties.Value_unlocked(Constants::SERVICE_ID).first;
            assert(anyId2.Empty() || anyId2.Type() == typeid(long int));
        }

        int const r1 = anyR1.Empty() ? 0 : *any_cast<int>(&anyR1);
        int const r2 = anyR2.Empty() ? 0 : *any_cast<int>(&anyR2);

        if (r1 != r2)
        {
            // use ranking if ranking differs
            return r1 < r2;
        }
        else
        {
            long int const id1 = *any_cast<long int>(&anyId1);
            long int const id2 = *any_cast<long int>(&anyId2);

            // otherwise compare using IDs,
            // is less than if it has a higher ID.
            return id2 < id1;
        }
    }

    bool
    ServiceReferenceBase::operator==(ServiceReferenceBase const& reference) const
    {
        return std::atomic_load(&d)->registration.lock() == std::atomic_load(&reference.d)->registration.lock();
    }

    ServiceReferenceBase&
    ServiceReferenceBase::operator=(ServiceReferenceBase const& reference)
    {
        if (d == std::atomic_load(&reference.d))
            return *this;

        d = reference.d;

        return *this;
    }

    bool
    ServiceReferenceBase::IsConvertibleTo(std::string const& interfaceId) const
    {
        return std::atomic_load(&d)->IsConvertibleTo(interfaceId);
    }

    std::string
    ServiceReferenceBase::GetInterfaceId() const
    {
        return std::atomic_load(&d)->interfaceId;
    }

    std::size_t
    ServiceReferenceBase::Hash() const
    {
        return std::hash<std::shared_ptr<ServiceRegistrationBasePrivate>>()(
            std::atomic_load(&this->d)->registration.lock());
    }

    std::ostream&
    operator<<(std::ostream& os, ServiceReferenceBase const& serviceRef)
    {
        if (serviceRef)
        {
            assert(serviceRef.GetBundle());

            os << "Reference for service object registered from " << serviceRef.GetBundle().GetSymbolicName() << " "
               << serviceRef.GetBundle().GetVersion() << " (";
            std::vector<std::string> keys = serviceRef.GetPropertyKeys();
            size_t keySize = keys.size();
            for (size_t i = 0; i < keySize; ++i)
            {
                os << keys[i] << "=" << serviceRef.GetProperty(keys[i]).ToString();
                if (i < keySize - 1)
                    os << ",";
            }
            os << ")";
        }
        else
        {
            os << "Invalid service reference";
        }

        return os;
    }
} // namespace cppmicroservices
