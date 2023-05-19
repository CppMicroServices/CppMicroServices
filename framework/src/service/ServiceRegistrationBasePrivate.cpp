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

#include "ServiceRegistrationBasePrivate.h"
#include "BundlePrivate.h"

#include <utility>

#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4355)
#endif

namespace cppmicroservices
{

    ServiceRegistrationBasePrivate::ServiceRegistrationBasePrivate(BundlePrivate* bundle,
                                                                   InterfaceMapConstPtr service,
                                                                   Properties&& props)
        : coreInfo(std::make_shared<ServiceRegistrationCoreInfo>(bundle, service, std::move(props)))
    {
    }

    ServiceRegistrationBasePrivate::~ServiceRegistrationBasePrivate() = default;

    // Need to first create shared_ptr to registration before duplicating for reference
    void
    ServiceRegistrationBasePrivate::CreateReference()
    {
        reference = shared_from_this();
    }

    bool
    ServiceRegistrationBasePrivate::IsUsedByBundle(BundlePrivate* bundle) const
    {
        auto l = this->Lock();
        US_UNUSED(l);
        auto l1 = coreInfo->Lock();
        US_UNUSED(l1);
        return (coreInfo->dependents.find(bundle) != coreInfo->dependents.end())
               || (coreInfo->prototypeServiceInstances.find(bundle) != coreInfo->prototypeServiceInstances.end());
    }

    InterfaceMapConstPtr
    ServiceRegistrationBasePrivate::GetInterfaces() const
    {
        return (this->Lock(), coreInfo->Lock(), coreInfo->service);
    }

    std::shared_ptr<void>
    ServiceRegistrationBasePrivate::GetService(std::string const& interfaceId) const
    {
        return this->Lock(), coreInfo->Lock(), GetService_unlocked(interfaceId);
    }

    std::shared_ptr<void>
    ServiceRegistrationBasePrivate::GetService_unlocked(std::string const& interfaceId) const
    {
        return ExtractInterface(coreInfo->service, interfaceId);
    }
} // namespace cppmicroservices

#ifdef _MSC_VER
#    pragma warning(pop)
#endif
