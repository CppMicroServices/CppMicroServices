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

#ifndef CPPMICROSERVICES_SERVICEHOOKS_H
#define CPPMICROSERVICES_SERVICEHOOKS_H

#include "cppmicroservices/ServiceTracker.h"
#include "cppmicroservices/detail/WaitCondition.h"

#include "ServiceListeners.h"

namespace cppmicroservices
{

    struct ServiceListenerHook;

    class ServiceHooks
        : public detail::MultiThreaded<>
        , public ServiceTrackerCustomizer<ServiceListenerHook>
    {

      private:
        CoreBundleContext* coreCtx;
        std::unique_ptr<ServiceTracker<ServiceListenerHook>> listenerHookTracker;

        std::atomic<bool> bOpen;

        virtual std::shared_ptr<ServiceListenerHook> AddingService(
            ServiceReference<ServiceListenerHook> const& reference);
        virtual void ModifiedService(ServiceReference<ServiceListenerHook> const& reference,
                                     std::shared_ptr<ServiceListenerHook> const& service);
        virtual void RemovedService(ServiceReference<ServiceListenerHook> const& reference,
                                    std::shared_ptr<ServiceListenerHook> const& service);

      public:
        ServiceHooks(CoreBundleContext* coreCtx);
        ~ServiceHooks();

        void Open();

        void Close();

        bool IsOpen() const;

        void FilterServiceReferences(BundleContextPrivate* context,
                                     std::string const& service,
                                     std::string const& filter,
                                     std::vector<ServiceReferenceBase>& refs);

        void FilterServiceEventReceivers(ServiceEvent const& evt, ServiceListeners::ServiceListenerEntries& receivers);

        void HandleServiceListenerReg(ServiceListenerEntry const& sle);

        void HandleServiceListenerUnreg(ServiceListenerEntry const& sle);

        void HandleServiceListenerUnreg(std::vector<ServiceListenerEntry> const& set);
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_SERVICEHOOKS_H
