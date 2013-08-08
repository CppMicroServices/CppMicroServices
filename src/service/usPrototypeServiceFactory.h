/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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

#ifndef USPROTOTYPESERVICEFACTORY_H
#define USPROTOTYPESERVICEFACTORY_H

#include <usServiceFactory.h>

US_BEGIN_NAMESPACE

/**
 * @ingroup MicroServices
 *
 * A factory for \link ServiceConstants::SCOPE_PROTOTYPE prototype scope\endlink services.
 * The factory can provide multiple, unique service objects.
 *
 * When registering a service, a PrototypeServiceFactory object can be used
 * instead of a service object, so that the module developer can create a
 * unique service object for each caller that is using the service.
 * When a caller uses a ServiceObjects to request a service instance, the
 * framework calls the GetService method to return a service object specifically
 * for the requesting caller. The caller can release the returned service object
 * and the framework will call the UngetService method with the service object.
 * When a module uses the ModuleContext::GetService(const ServiceReferenceBase&)
 * method to obtain a service object, the framework acts as if the service
 * has module scope. That is, the framework will call the GetService method to
 * obtain a module-scoped instance which will be cached and have a use count.
 * See ServiceFactory.
 *
 * A module can use both ServiceObjects and ModuleContext::GetService(const ServiceReferenceBase&)
 * to obtain a service object for a service. ServiceObjects::GetService() will always
 * return an instance provided by a call to GetService(Module*, const ServiceRegistrationBase&)
 * and ModuleContext::GetService(const ServiceReferenceBase&) will always
 * return the module-scoped instance.
 * PrototypeServiceFactory objects are only used by the framework and are not made
 * available to other modules. The framework may concurrently call a PrototypeServiceFactory.
 *
 * @see ModuleContext::GetServiceObjects()
 * @see ServiceObjects
 */
struct PrototypeServiceFactory : public ServiceFactory
{

  /**
   * Returns a service object for a caller.
   *
   * The framework invokes this method for each caller requesting a service object using
   * ServiceObjects::GetService(). The factory can then return a specific service object for the caller.
   * The framework checks that the returned service object is valid. If the returned service
   * object is empty or does not contain entries for all the interfaces named when the service
   * was registered, a warning is issued and NULL is returned to the caller. If this
   * method throws an exception, a warning is issued and NULL is returned to the caller.
   *
   * @param module The module requesting the service.
   * @param registration The ServiceRegistrationBase object for the requested service.
   * @return A service object that must contain entries for all the interfaces named when
   *         the service was registered.
   *
   * @see ServiceObjects#GetService()
   * @see InterfaceMap
   */
  virtual InterfaceMap GetService(Module* module, const ServiceRegistrationBase& registration) = 0;

  /**
   * Releases a service object created for a caller.
   *
   * The framework invokes this method when a service has been released by a modules such as
   * by calling ServiceObjects::UngetService(). The service object may then be destroyed.
   * If this method throws an exception, a warning is issued.
   *
   * @param module The module releasing the service.
   * @param registration The ServiceRegistrationBase object for the service being released.
   * @param service The service object returned by a previous call to the GetService method.
   *
   * @see ServiceObjects::UngetService()
   */
  virtual void UngetService(Module* module, const ServiceRegistrationBase& registration,
                            const InterfaceMap& service) = 0;

};

US_END_NAMESPACE

#endif // USPROTOTYPESERVICEFACTORY_H
