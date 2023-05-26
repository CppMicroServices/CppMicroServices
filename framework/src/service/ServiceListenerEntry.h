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

#ifndef CPPMICROSERVICES_SERVICELISTENERENTRY_H
#define CPPMICROSERVICES_SERVICELISTENERENTRY_H

#include "cppmicroservices/ListenerFunctors.h"
#include "cppmicroservices/ListenerToken.h"
#include "cppmicroservices/ServiceListenerHook.h"

#include "LDAPExpr.h"
#include "Utils.h"

namespace cppmicroservices
{

    class BundleContextPrivate;
    class ServiceListenerEntryData;

    /**
     * Data structure for saving service listener info. Contains
     * the optional service listener filter, in addition to the info
     * in ListenerEntry.
     */
    class ServiceListenerEntry : public ServiceListenerHook::ListenerInfo
    {

      public:
        ServiceListenerEntry();
        ServiceListenerEntry(ServiceListenerEntry const& other);
        ServiceListenerEntry(ServiceListenerHook::ListenerInfo const& info);

        ~ServiceListenerEntry();
        ServiceListenerEntry& operator=(ServiceListenerEntry const& other);

        void SetRemoved(bool removed) const;

        ServiceListenerEntry(std::shared_ptr<BundleContextPrivate> const& context,
                             ServiceListener const& l,
                             void* data,
                             ListenerTokenId tokenId,
                             std::string const& filter = "");

        LDAPExpr const& GetLDAPExpr() const;

        LDAPExpr::LocalCache& GetLocalCache() const;

        void CallDelegate(ServiceEvent const& event) const;

        bool operator==(ServiceListenerEntry const& other) const;
        bool operator<(ServiceListenerEntry const& other) const;

        bool Contains(std::shared_ptr<BundleContextPrivate> const& context,
                      ServiceListener const& listener,
                      void* data) const;

        ListenerTokenId Id() const;

        std::size_t Hash() const;
    };
} // namespace cppmicroservices

US_HASH_FUNCTION_BEGIN(cppmicroservices::ServiceListenerEntry)
return arg.Hash();
US_HASH_FUNCTION_END

#endif // CPPMICROSERVICES_SERVICELISTENERENTRY_H
