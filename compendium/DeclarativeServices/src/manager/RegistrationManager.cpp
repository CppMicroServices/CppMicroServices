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
#include <cassert>
#include "cppmicroservices/ServiceFactory.h"
#include "cppmicroservices/Constants.h"
#include "RegistrationManager.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"

using cppmicroservices::Constants::SERVICE_SCOPE;
using cppmicroservices::Constants::SCOPE_BUNDLE;
using cppmicroservices::Constants::SCOPE_SINGLETON;
using cppmicroservices::Constants::SCOPE_PROTOTYPE;

namespace cppmicroservices {
namespace scrimpl {

bool inline IsValidScope(const std::string& scope)
{
  return (scope == SCOPE_SINGLETON || scope == SCOPE_BUNDLE || scope == SCOPE_PROTOTYPE);
}

RegistrationManager::RegistrationManager(const cppmicroservices::BundleContext& bc,
                                         const std::vector<std::string>& services,
                                         const std::string& scope,
                                         const std::shared_ptr<cppmicroservices::logservice::LogService>& logger)
  : bundleContext(bc)
  , services(services)
  , scope(scope)
  , logger(logger)
{
  if(!bc || services.empty() || !IsValidScope(scope) || !logger)
  {
    throw std::invalid_argument("RegistrationManager: Invalid arguments passed to constructor");
  }
}

RegistrationManager::~RegistrationManager()
{
  if(IsServiceRegistered())
  {
    try
    {
      serviceReg.Unregister();
    }
    catch (...)
    {
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, "Service Unregistration failed with exception", std::current_exception());
    }
  }
}

bool RegistrationManager::IsServiceRegistered() const
{
  return static_cast<bool>(serviceReg); //(serviceReg ? true : false);
}

bool RegistrationManager::RegisterService(const std::shared_ptr<cppmicroservices::ServiceFactory>& factory,
                                          const cppmicroservices::ServiceProperties& props)
{
  if(IsServiceRegistered())
  {
    return true;
  }
  try
  {
    cppmicroservices::InterfaceMapPtr imap = MakeInterfaceMap<>(factory);
    std::shared_ptr<void> instance = std::static_pointer_cast<void>(factory);
    for(auto interface : services)
    {
      imap->emplace(interface, instance);
    }
    auto localProps = props;
    localProps.emplace(SERVICE_SCOPE, Any(scope));
    serviceReg = bundleContext.RegisterService(imap, localProps);
  }
  catch(...)
  {
    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, "Service Registration failed with exception", std::current_exception());
  }
  return IsServiceRegistered();
}

void RegistrationManager::UnregisterService()
{
  serviceReg.Unregister();
  // necessary because the bool operator of ServiceRegistration object
  // returns true even after the service has been unregistered.
  serviceReg = nullptr;
}

cppmicroservices::ServiceReferenceBase RegistrationManager::GetServiceReference() const
{
  return serviceReg ? serviceReg.GetReference() : ServiceReferenceU();
}
}
}
