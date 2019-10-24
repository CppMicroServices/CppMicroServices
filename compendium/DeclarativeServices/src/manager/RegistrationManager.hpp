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

#ifndef __REGISTRATION_MANAGER_HPP__
#define __REGISTRATION_MANAGER_HPP__
#include "gtest/gtest_prod.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceFactory.h"
#include "cppmicroservices/ServiceProperties.h"
#include "cppmicroservices/ServiceRegistration.h"
#include "cppmicroservices/logservice/LogService.hpp"

namespace cppmicroservices {
namespace scrimpl {
/**
 * This is a helper class used by {@link ComponentConfiguration} to manage
 * the registration of the services it provides
 *
 * \note: This class is not thread safe. Caller is responsible for thread safety
 */
class RegistrationManager final
{
public:
  /**
   * Constructor
   * \param bc - the {@link BundleContext} of the bundle containing the Service implementation
   * \param services - a \c vector containing service interface names used for service registration
   * \param scope - the scope of the service registered by this object
   * \param logger - the logger object used to log information from this class.
   *
   * \throws \c std::invalid_argument if any of the following is true
   *             \c bc is inavlid bundle context
   *             \c services is empty
   *             \c scope is not one of "singleton", "bundle" or "prototype"
   *             \c logger is nullptr
   */
  RegistrationManager(const cppmicroservices::BundleContext& bc,
                      const std::vector<std::string>& services,
                      const std::string& scope,
                      const std::shared_ptr<cppmicroservices::logservice::LogService>& logger);
  RegistrationManager(const RegistrationManager&) = delete;
  RegistrationManager(RegistrationManager&&) = delete;
  RegistrationManager& operator=(const RegistrationManager&) = delete;
  RegistrationManager& operator=(RegistrationManager&&) = delete;
  ~RegistrationManager();

  /**
   * This method returns \c true if a service is registered using this object, \c false otherwise.
   */
  bool IsServiceRegistered() const;

  /**
   * This method returns a {@link ServiceReferenceBase} object from the stored {@link ServiceRegistration}
   * object.
   *
   * \return a valid {@link ServiceReferenceBase} object if the service is
   * registered, invalid object if no service is registered by this object.
   */
  cppmicroservices::ServiceReferenceBase GetServiceReference() const;

  /**
   * This method registers a service using the service interface names and scope
   * stored in this object along with the factory object and the properties passed
   * as parameters.
   *
   * \param factory - a {@link ServiceFactory} object used for service registration
   * \param props - a map with properties used for service registration
   * \return \c true if registration succeeded, \c false otherwise
   */
  bool RegisterService(const std::shared_ptr<cppmicroservices::ServiceFactory>& factory,
                       const cppmicroservices::ServiceProperties& props);

  /**
   * This method unregisters the service from the framework service registry. The
   * {@link ServiceRegistration} member of this object is set to null before returning
   * from this method.
   *
   * \throws std::logic_error if the service was never registered or has already been unregistered
   */
  void UnregisterService();
private:
  FRIEND_TEST(RegistrationManagerTest, VerifyUnregister);

  cppmicroservices::ServiceRegistrationU serviceReg;
  cppmicroservices::BundleContext bundleContext;
  std::vector<std::string> services;
  std::string scope;
  std::shared_ptr<cppmicroservices::logservice::LogService> logger;
};
}
}

#endif // __REGISTRATION_MANAGER_HPP__
