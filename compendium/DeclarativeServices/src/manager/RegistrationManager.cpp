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
#include "RegistrationManager.hpp"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/ServiceFactory.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include <cassert>

using cppmicroservices::Constants::SCOPE_BUNDLE;
using cppmicroservices::Constants::SCOPE_PROTOTYPE;
using cppmicroservices::Constants::SCOPE_SINGLETON;
using cppmicroservices::Constants::SERVICE_SCOPE;

namespace cppmicroservices
{
    namespace scrimpl
    {

        bool inline IsValidScope(std::string const& scope)
        {
            return (scope == SCOPE_SINGLETON || scope == SCOPE_BUNDLE || scope == SCOPE_PROTOTYPE);
        }

        RegistrationManager::RegistrationManager(
            cppmicroservices::BundleContext const& bc,
            std::vector<std::string> const& services,
            std::string const& scope,
            std::shared_ptr<cppmicroservices::logservice::LogService> const& logger)
            : bundleContext(bc)
            , services(services)
            , scope(scope)
            , logger(logger)
        {
            if (!bc || services.empty() || !IsValidScope(scope) || !logger)
            {
                throw std::invalid_argument("RegistrationManager: Invalid arguments passed to constructor");
            }
        }

        RegistrationManager::~RegistrationManager()
        {
            if (IsServiceRegistered())
            {
                try
                {
                    serviceReg.Unregister();
                }
                catch (...)
                {
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                "Service Unregistration failed with exception",
                                std::current_exception());
                }
            }
        }

        bool
        RegistrationManager::IsServiceRegistered() const
        {
            return static_cast<bool>(serviceReg); //(serviceReg ? true : false);
        }

        bool
        RegistrationManager::RegisterService(std::shared_ptr<cppmicroservices::ServiceFactory> const& factory,
                                             cppmicroservices::ServiceProperties const& props)
        {
            if (IsServiceRegistered())
            {
                return true;
            }
            try
            {
                cppmicroservices::InterfaceMapPtr imap = MakeInterfaceMap<>(factory);
                std::shared_ptr<void> instance = std::static_pointer_cast<void>(factory);
                for (auto interface : services)
                {
                    imap->emplace(interface, instance);
                }
                auto localProps = props;
                localProps.emplace(SERVICE_SCOPE, Any(scope));
                serviceReg = bundleContext.RegisterService(imap, localProps);
            }
            catch (...)
            {
                logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                            "Service Registration failed with exception",
                            std::current_exception());
            }
            return IsServiceRegistered();
        }
        void
        RegistrationManager::SetProperties(cppmicroservices::ServiceProperties && properties)
        {
            try
            {
                if (IsServiceRegistered() && serviceReg)
                {
                    serviceReg.SetProperties(std::move(properties));
                }
            }
            catch (...)
            {
                logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                            "Service SetProperties failed with exception",
                            std::current_exception());
            }
        }

        void
        RegistrationManager::UnregisterService()
        {
            serviceReg.Unregister();
            // necessary because the bool operator of ServiceRegistration object
            // returns true even after the service has been unregistered.
            serviceReg = nullptr;
        }

        cppmicroservices::ServiceReferenceBase
        RegistrationManager::GetServiceReference() const
        {
            return serviceReg ? serviceReg.GetReference() : ServiceReferenceU();
        }
    } // namespace scrimpl
} // namespace cppmicroservices
