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

#ifndef SCRACTIVATOR_HPP
#define SCRACTIVATOR_HPP
#include "ComponentRegistry.hpp"
#include "SCRAsyncWorkService.hpp"
#include "SCRBundleExtension.hpp"
#include "SCRExtensionRegistry.hpp"
#include "SCRLogger.hpp"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/cm/ConfigurationListener.hpp"
#include "cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp"
#include "manager/ConfigurationNotifier.hpp"
#include <map>
#include <vector>
#include <shared_mutex>

using cppmicroservices::service::component::runtime::ServiceComponentRuntime;

namespace cppmicroservices
{
    namespace scrimpl
    {
        using WriteLock = std::unique_lock<std::shared_mutex>;
        using ReadLock = std::shared_lock<std::shared_mutex>;

        class SCRActivator : public cppmicroservices::BundleActivator
        {
          public:
            SCRActivator() = default;
            SCRActivator(SCRActivator const&) = delete;
            SCRActivator(SCRActivator&&) = delete;
            SCRActivator& operator=(SCRActivator const&) = delete;
            SCRActivator& operator=(SCRActivator&&) = delete;
            ~SCRActivator() override = default;

            // callback methods for bundle lifecycle
            void Start(cppmicroservices::BundleContext context) override;
            void Stop(cppmicroservices::BundleContext context) override;

          protected:
            /**
             * bundle listener callback
             */
            void BundleChanged(cppmicroservices::BundleEvent const&);
            /*
             * This method creates the BundleExtension object for a bundle
             * with declarative services metadata
             */
            void CreateExtension(cppmicroservices::Bundle const& bundle);
            /*
             * This method removes the BundleExtension object for a bundle
             * with declarative services metadata
             */
            void DisposeExtension(cppmicroservices::Bundle const& bundle);

          private:
            cppmicroservices::BundleContext runtimeContext;
            cppmicroservices::ServiceRegistration<ServiceComponentRuntime> scrServiceReg;
            std::shared_ptr<ComponentRegistry> componentRegistry;
            std::shared_ptr<SCRLogger> logger;
            std::shared_mutex notificationLock;
            std::shared_ptr<SCRExtensionRegistry> bundleRegistry;
            ListenerToken bundleListenerToken;
            std::shared_ptr<SCRAsyncWorkService> asyncWorkService;
            cppmicroservices::ServiceRegistration<cppmicroservices::service::cm::ConfigurationListener>
                configListenerReg;
            std::shared_ptr<ConfigurationNotifier> configNotifier;
        };
    } // namespace scrimpl
} // namespace cppmicroservices
#endif // SCRACTIVATOR_HPP
