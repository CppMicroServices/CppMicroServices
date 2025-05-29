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

#ifndef cppmicroservices_service_cm_ConfigurationListenerImpl_HPP
#define cppmicroservices_service_cm_ConfigurationListenerImpl_HPP

#include "SCRLogger.hpp"
#include "cppmicroservices/cm/ConfigurationListener.hpp"
#include "manager/ConfigurationNotifier.hpp"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {

            /**
             * ConfigChangeNotification
             * This class is used by ConfigurationListener to notify ComponentConfigurationImpl
             * about changes to Configuration Objects.
             */
            struct ConfigChangeNotification final
            {
                ConfigChangeNotification(std::string pid,
                                         std::shared_ptr<cppmicroservices::AnyMap> properties,
                                         ConfigurationEventType evt)
                    : pid(std::move(pid))
                    , event(std::move(evt))
                    , newProperties(std::move(properties))
                {
                }

                std::string pid;
                ConfigurationEventType event;
                std::shared_ptr<cppmicroservices::AnyMap> newProperties;
            };
            class ConfigurationListenerImpl final : public ConfigurationListener
            {

              public:
                /*
                 *@throws std::invalid_argument exception if any of the params is a nullptr
                 */
                ConfigurationListenerImpl(
                    cppmicroservices::BundleContext const& context,
                    std::shared_ptr<cppmicroservices::logservice::LogService> logger,
                    std::shared_ptr<cppmicroservices::scrimpl::ConfigurationNotifier> configNotifier);
                ConfigurationListenerImpl(ConfigurationListenerImpl const&) = delete;
                ConfigurationListenerImpl(ConfigurationListenerImpl&&) = delete;
                ConfigurationListenerImpl& operator=(ConfigurationListenerImpl const&) = delete;
                ConfigurationListenerImpl& operator=(ConfigurationListenerImpl&&) = delete;
                ~ConfigurationListenerImpl() = default;

                /*
                 * configurationEvent is the method called by Configuration Admin whenever a
                 * a configuraton object is updated or removed.
                 *
                 */
                void configurationEvent(ConfigurationEvent const& event) override;

              private:
                cppmicroservices::BundleContext bundleContext;
                std::shared_ptr<cppmicroservices::logservice::LogService> logger;
                std::shared_ptr<cppmicroservices::scrimpl::ConfigurationNotifier> configNotifier;
            };

        } // namespace cm
    }     // namespace service
} // namespace cppmicroservices
#endif // cppmicroservices_service_cm_ConfigurationListenerImpl_HPP
