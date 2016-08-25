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

#ifndef CPPMICROSERVICES_SERVICEFACTORY_H
#define CPPMICROSERVICES_SERVICEFACTORY_H

#include "cppmicroservices/ServiceInterface.h"
#include "cppmicroservices/ServiceRegistration.h"

namespace cppmicroservices {

/**
 * \ingroup MicroServices
 *
 * A factory for \link Constants::SCOPE_BUNDLE bundle scope\endlink services.
 * The factory can provide service objects unique to each bundle.
 *
 * <p>
 * When registering a service, a <code>ServiceFactory</code> object can be
 * used instead of a service object, so that the bundle developer can gain
 * control of the specific service object granted to a bundle that is using the
 * service.
 *
 * <p>
 * When this happens, the
 * <code>BundleContext::GetService(const ServiceReference&)</code> method calls the
 * <code>ServiceFactory::GetService</code> method to create a service object
 * specifically for the requesting bundle. The service object returned by the
 * <code>ServiceFactory</code> is cached by the framework until the bundle
 * releases its use of the service.
 *
 * <p>
 * When the bundle's use count for the service equals zero (including the bundle
 * stopping or the service being unregistered), the
 * <code>ServiceFactory::UngetService</code> method is called.
 *
 * <p>
 * <code>ServiceFactory</code> objects are only used by the framework and are
 * not made available to other bundles in the bundle environment. The framework
 * may concurrently call a <code>ServiceFactory</code>.
 *
 * @see BundleContext#GetService
 * @see PrototypeServiceFactory
 * @remarks This class is thread safe.
 */
class ServiceFactory
{

public:

  virtual ~ServiceFactory() {}

  /**
   * Creates a new service object.
   *
   * <p>
   * The Framework invokes this method the first time the specified
   * <code>bundle</code> requests a service object using the
   * <code>BundleContext::GetService(const ServiceReferenceBase&)</code> method. The
   * service factory can then return a specific service object for each
   * bundle.
   *
   * <p>
   * The framework caches the value returned (unless the InterfaceMap is empty),
   * and will return the same service object on any future call to
   * <code>BundleContext::GetService</code> for the same bundles. This means the
   * framework does not allow this method to be concurrently called for the
   * same bundle.
   *
   * @param bundle The bundle using the service.
   * @param registration The <code>ServiceRegistrationBase</code> object for the
   *        service.
   * @return A service object that <strong>must</strong> contain entries for all
   *         the interfaces named when the service was registered.
   * @see BundleContext#GetService
   * @see InterfaceMapConstPtr
   */
  virtual InterfaceMapConstPtr GetService(const Bundle& bundle,
                                          const ServiceRegistrationBase& registration) = 0;

  /**
   * Releases a service object.
   *
   * <p>
   * The framework invokes this method when a service has been released by a
   * bundle.
   *
   * @param bundle The Bundle releasing the service.
   * @param registration The <code>ServiceRegistration</code> object for the
   *        service.
   * @param service The service object returned by a previous call to the
   *        <code>ServiceFactory::GetService</code> method.
   * @see InterfaceMapConstPtr
   */
  virtual void UngetService(const Bundle& bundle,
                            const ServiceRegistrationBase& registration,
                            const InterfaceMapConstPtr& service) = 0;
};

}

#endif // CPPMICROSERVICES_SERVICEFACTORY_H
