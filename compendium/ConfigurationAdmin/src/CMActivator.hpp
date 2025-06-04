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

#ifndef CMACTIVATOR_HPP
#define CMACTIVATOR_HPP

#include <memory>
#include <mutex>
#include <unordered_map>
#include <shared_mutex>

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/ListenerToken.h"

#include "CMAsyncWorkService.hpp"
#include "CMBundleExtension.hpp"
#include "CMConstants.hpp"
#include "CMLogger.hpp"
#include "ConfigurationAdminImpl.hpp"

namespace cppmicroservices
{
    namespace cmimpl
    {
        using WriteLock = std::unique_lock<std::shared_mutex>;
        using ReadLock = std::shared_lock<std::shared_mutex>;

        class CMActivator final : public cppmicroservices::BundleActivator
        {
          public:
            CMActivator() = default;
            CMActivator(CMActivator const&) = delete;
            CMActivator(CMActivator&&) = delete;
            CMActivator& operator=(CMActivator const&) = delete;
            CMActivator& operator=(CMActivator&&) = delete;
            ~CMActivator() override = default;

            // callback methods for bundle lifecycle
            void Start(cppmicroservices::BundleContext context) override;
            void Stop(cppmicroservices::BundleContext context) override;

            // protected for pkgtests
          protected:
            /**
             * Bundle listener callback
             */
            void BundleChanged(cppmicroservices::BundleEvent const&);
            /*
             * This method creates the CMBundleExtension object for a bundle
             * with cm configuration metadata
             */
            void CreateExtension(cppmicroservices::Bundle const& bundle);
            /*
             * This method removes the CMBundleExtension object for a bundle
             * with cm configuration metadata
             */
            void RemoveExtension(cppmicroservices::Bundle const& bundle);

          private:
            cppmicroservices::BundleContext runtimeContext;
            std::shared_ptr<CMLogger> logger;
            std::shared_ptr<CMAsyncWorkService> asyncWorkService;
            std::shared_ptr<ConfigurationAdminImpl> configAdminImpl;
            std::mutex bundleRegMutex;
            std::shared_ptr<std::shared_mutex> notificationLock;
            std::shared_ptr<bool> activatorStopped;
            std::unordered_map<long, std::unique_ptr<CMBundleExtension>> bundleRegistry;
            cppmicroservices::ListenerToken bundleListenerToken;
            cppmicroservices::ServiceRegistration<cppmicroservices::service::cm::ConfigurationAdmin> configAdminReg;
        };
    } // namespace cmimpl
} // namespace cppmicroservices

#endif // CMACTIVATOR_HPP
