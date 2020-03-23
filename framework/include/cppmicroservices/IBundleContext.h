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

#ifndef CPPMICROSERVICES_IBUNDLECONTEXT_H
#define CPPMICROSERVICES_IBUNDLECONTEXT_H

#include "cppmicroservices/GlobalConfig.h"
#include "cppmicroservices/ListenerFunctors.h"
#include "cppmicroservices/ListenerToken.h"
#include "cppmicroservices/ServiceInterface.h"
#include "cppmicroservices/ServiceRegistration.h"

#include <memory>

namespace cppmicroservices {

class AnyMap;
class Bundle;
class BundleContext;
class ServiceFactory;

/**
  * \ingroup MicroServices
  *
  * Minimal interface for a BundleContext
  * 
  * \see BundleContext for complete implementation documentation
  */
class US_Framework_EXPORT IBundleContext
{

public:
  virtual ~IBundleContext() = default;

  /**
    * Returns a list of <code>ServiceReference</code> objects. The returned
    * list contains services that were registered under the specified class
    * and match the specified filter expression.
    *
    * <p>
    * The list is valid at the time of the call to this method. However, since
    * the framework is a very dynamic environment, services can be modified or
    * unregistered at any time.
    *
    * <p>
    * The specified <code>filter</code> expression is used to select the
    * registered services whose service properties contain keys and values
    * that satisfy the filter expression. See LDAPFilter for a description
    * of the filter syntax. If the specified <code>filter</code> is
    * empty, all registered services are considered to match the
    * filter. If the specified <code>filter</code> expression cannot be parsed,
    * an <code>std::invalid_argument</code> will be thrown with a human-readable
    * message where the filter became unparsable.
    *
    * <p>
    * The result is a list of <code>ServiceReference</code> objects for all
    * services that meet all of the following conditions:
    * <ul>
    * <li>If the specified class name, <code>clazz</code>, is not
    * empty, the service must have been registered with the
    * specified class name. The complete list of class names with which a
    * service was registered is available from the service's
    * {@link Constants#OBJECTCLASS objectClass} property.
    * <li>If the specified <code>filter</code> is not empty, the
    * filter expression must match the service.
    * </ul>
    *
    * @param clazz The class name with which the service was registered or
    *        an empty string for all services.
    * @param filter The filter expression or empty for all
    *        services.
    * @return A list of <code>ServiceReference</code> objects or
    *         an empty list if no services are registered that satisfy the
    *         search.
    * @throws std::invalid_argument If the specified <code>filter</code>
    *         contains an invalid filter expression that cannot be parsed.
    * @throws std::runtime_error If this BundleContext is no longer valid.
    * @throws std::logic_error If the ServiceRegistrationBase object is invalid,
    *         or if the service is unregistered.
    */
  virtual std::vector<ServiceReferenceU> GetServiceReferences(
    const std::string& clazz,
    const std::string& filter = std::string()) = 0;

  /**
    * Returns a <code>ServiceReference</code> object for a service that
    * implements and was registered under the specified class.
    *
    * <p>
    * The returned <code>ServiceReference</code> object is valid at the time of
    * the call to this method. However as the Micro Services framework is a very dynamic
    * environment, services can be modified or unregistered at any time.
    *
    * <p>
    * This method is the same as calling
    * {@link BundleContext::GetServiceReferences(const std::string&, const std::string&)} with an
    * empty filter expression. It is provided as a convenience for
    * when the caller is interested in any service that implements the
    * specified class.
    * <p>
    * If multiple such services exist, the service with the highest ranking (as
    * specified in its Constants::SERVICE_RANKING property) is returned.
    * <p>
    * If there is a tie in ranking, the service with the lowest service ID (as
    * specified in its Constants::SERVICE_ID property); that is, the
    * service that was registered first is returned.
    *
    * @param clazz The class name with which the service was registered.
    * @return A <code>ServiceReference</code> object, or an invalid <code>ServiceReference</code> if
    *         no services are registered which implement the named class.
    * @throws std::runtime_error If this BundleContext is no longer valid.
    *
    * @see #GetServiceReferences(const std::string&, const std::string&)
    */
  virtual ServiceReferenceU GetServiceReference(const std::string& clazz) = 0;

  /**
    * Returns the service object referenced by the specified
    * <code>ServiceReferenceBase</code> object.
    * <p>
    * A bundle's use of a service is tracked by the bundle's use count of that
    * service. Each call to {@link #GetService(const ServiceReference<S>&)} increments
    * the context bundle's use count by one. The deleter function of the returned shared_ptr
    * object is responsible for decrementing the context bundle's use count.
    * <p>
    * When a bundle's use count for a service drops to zero, the bundle should
    * no longer use that service.
    *
    * <p>
    * This method will always return an empty object when the service
    * associated with this <code>reference</code> has been unregistered.
    *
    * <p>
    * The following steps are taken to get the service object:
    * <ol>
    * <li>If the service has been unregistered, empty object is returned.
    * <li>The context bundle's use count for this service is incremented by
    * one.
    * <li>If the context bundle's use count for the service is currently one
    * and the service was registered with an object implementing the
    * <code>ServiceFactory</code> interface, the
    * {@link ServiceFactory::GetService} method is
    * called to create a service object for the context bundle. This service
    * object is cached by the framework. While the context bundle's use count
    * for the service is greater than zero, subsequent calls to get the
    * services's service object for the context bundle will return the cached
    * service object. <br>
    * If the <code>ServiceFactory</code> object throws an
    * exception, empty object is returned and a warning is logged.
    * <li>A shared_ptr to the service object is returned.
    * </ol>
    *
    * @param reference A reference to the service.
    * @return A shared_ptr to the service object associated with <code>reference</code>.
    *         An empty shared_ptr is returned if the service is not registered or the
    *         <code>ServiceFactory</code> threw an exception
    * @throws std::runtime_error If this BundleContext is no longer valid.
    * @throws std::invalid_argument If the specified
    *         <code>ServiceReferenceBase</code> is invalid (default constructed).
    * @see ServiceFactory
    */
  virtual std::shared_ptr<void> GetService(
    const ServiceReferenceBase& reference) = 0;

  virtual InterfaceMapConstPtr GetService(
    const ServiceReferenceU& reference) = 0;
};

} // namespace cppmicroservices

#endif /* CPPMICROSERVICES_IBUNDLECONTEXT_H */
