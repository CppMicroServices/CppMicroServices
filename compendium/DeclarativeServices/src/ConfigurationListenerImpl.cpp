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

#include "ConfigurationListenerImpl.hpp"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {

            ConfigurationListenerImpl::ConfigurationListenerImpl(
                cppmicroservices::BundleContext const& context,
                std::shared_ptr<cppmicroservices::logservice::LogService> logger,
                std::shared_ptr<cppmicroservices::scrimpl::ConfigurationNotifier> configNotifier)
                : bundleContext(context)
                , logger(std::move(logger))
                , configNotifier(std::move(configNotifier))
            {
                if (!bundleContext || !(this->logger) || !(this->configNotifier))
                {
                    throw std::invalid_argument("ConfigurationListenerImpl Constructor "
                                                "provided with invalid arguments");
                }
            }

            void
            ConfigurationListenerImpl::configurationEvent(ConfigurationEvent const& event)
            {
                try
                {
                    auto pid = (!event.getPid().empty()) ? event.getPid() : event.getFactoryPid();
                    if (pid.empty())
                    {
                        return;
                    }
                    auto configAdminRef = event.getReference();
                    if (!configAdminRef)
                    {
                        logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                            "configurationEvent error. ConfigurationAdmin service "
                            "reference is no longer valid");
                        return;
                    }
                    auto configAdmin
                        = bundleContext.GetService<cppmicroservices::service::cm::ConfigurationAdmin>(configAdminRef);
                    if (!configAdmin)
                    {
                        logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                            "configurationEvent error. ConfigurationAdmin service "
                            "reference is no longer valid");
                        return;
                    }
                    cppmicroservices::AnyMap properties;
                    auto type = event.getType();
                    unsigned long changeCount = 0;
                    if (type == ConfigurationEventType::CM_UPDATED)
                    {
                        auto configObject = configAdmin->GetConfiguration(pid);
                        if (configObject)
                        {
                            properties = configObject->GetProperties();
                            changeCount = configObject->GetChangeCount();
                        }
                    }
                    if (!configNotifier->AnyListenersForPid(pid, properties))
                    {
                        return;
                    }
                    auto ptr = std::make_shared<cppmicroservices::AnyMap>(properties);
                    configNotifier->NotifyAllListeners(pid, type, ptr, changeCount);
                }
                catch (cppmicroservices::SecurityException const&)
                {
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                "Security exception while executing "
                                "ConfigurationListener::configEvent method.",
                                std::current_exception());
                    throw;
                }
                catch (...)
                {
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                "Exception while executing ConfigurationListener::configEvent method.",
                                std::current_exception());
                }
            }
        } // namespace cm
    }     // namespace service
} // namespace cppmicroservices
