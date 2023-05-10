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
#include "Utils.h"
#include "ServiceReferenceBasePrivate.h"
#include "ServiceRegistrationBasePrivate.h"
#include "LockSet.h"
#include <cassert>

namespace cppmicroservices
{

    ServiceReferenceBase::ServiceReferenceBase()
        : d(std::make_shared<ServiceReferenceBasePrivate>(std::weak_ptr<ServiceRegistrationBasePrivate>()))
    {
    }

    ServiceReferenceBase::ServiceReferenceBase(ServiceReferenceBase const& ref) : d(atomic_load_tSafe(ref.d)) {}

    ServiceReferenceBase::ServiceReferenceBase(std::shared_ptr<ServiceRegistrationBasePrivate> reg)
        : d(std::make_shared<ServiceReferenceBasePrivate>(reg))
    {
    }

    void
    ServiceReferenceBase::SetInterfaceId(std::string const& interfaceId)
    {
        d = std::make_shared<ServiceReferenceBasePrivate>(atomic_load_tSafe(d)->registration);
        atomic_load_tSafe(d)->interfaceId = interfaceId;
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
        auto l = atomic_load_tSafe(d)->coreInfo->properties.Lock();
        US_UNUSED(l);
        return atomic_load_tSafe(d)->coreInfo->properties.Value_unlocked(key).first;
    }

    void
    ServiceReferenceBase::GetPropertyKeys(std::vector<std::string>& keys) const
    {
        keys = GetPropertyKeys();
    }

    std::vector<std::string>
    ServiceReferenceBase::GetPropertyKeys() const
    {
        auto l = atomic_load_tSafe(d)->coreInfo->properties.Lock();
        US_UNUSED(l);
        return atomic_load_tSafe(d)->coreInfo->properties.Keys_unlocked();
    }

    Bundle
    ServiceReferenceBase::GetBundle() const
    {
        auto p = atomic_load_tSafe(d);
        auto reg = p->registration.lock();
        if (!reg)
        {
            return Bundle();
        }

        auto l = p->GetLocks();
        US_UNUSED(l);
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
        auto l = atomic_load_tSafe(d)->GetLocks();
        US_UNUSED(l);
        for (auto const& iter : atomic_load_tSafe(d)->coreInfo->dependents)
        {
            bundles.push_back(MakeBundle(iter.first->shared_from_this()));
        }
        return bundles;
    }

    bool
    ServiceReferenceBase::operator<(ServiceReferenceBase const& reference) const
    {
        if (atomic_load_tSafe(d) == atomic_load_tSafe(reference.d))
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

        if (atomic_load_tSafe(d)->registration.lock() == atomic_load_tSafe(reference.d)->registration.lock())
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
            auto l1 = atomic_load_tSafe(d)->coreInfo->properties.Lock();
            US_UNUSED(l1);
            anyR1 = atomic_load_tSafe(d)->coreInfo->properties.Value_unlocked(Constants::SERVICE_RANKING).first;
            assert(anyR1.Empty() || anyR1.Type() == typeid(int));
            anyId1 = atomic_load_tSafe(d)->coreInfo->properties.Value_unlocked(Constants::SERVICE_ID).first;
            assert(anyId1.Empty() || anyId1.Type() == typeid(long int));
        }

        Any anyR2;
        Any anyId2;
        {
            auto l2 = atomic_load_tSafe(reference.d)->coreInfo->properties.Lock();
            US_UNUSED(l2);
            anyR2
                = atomic_load_tSafe(reference.d)->coreInfo->properties.Value_unlocked(Constants::SERVICE_RANKING).first;
            assert(anyR2.Empty() || anyR2.Type() == typeid(int));
            anyId2 = atomic_load_tSafe(reference.d)->coreInfo->properties.Value_unlocked(Constants::SERVICE_ID).first;
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
        return atomic_load_tSafe(d)->registration.lock() == atomic_load_tSafe(reference.d)->registration.lock();
    }

    ServiceReferenceBase&
    ServiceReferenceBase::operator=(ServiceReferenceBase const& reference)
    {
        if (d == atomic_load_tSafe(reference.d))
            return *this;

        d = reference.d;

        return *this;
    }

    bool
    ServiceReferenceBase::IsConvertibleTo(std::string const& interfaceId) const
    {
        return atomic_load_tSafe(d)->IsConvertibleTo(interfaceId);
    }

    std::string
    ServiceReferenceBase::GetInterfaceId() const
    {
        return atomic_load_tSafe(d)->interfaceId;
    }

    std::size_t
    ServiceReferenceBase::Hash() const
    {
        return std::hash<std::shared_ptr<ServiceRegistrationBasePrivate>>()(
            atomic_load_tSafe(this->d)->registration.lock());
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
