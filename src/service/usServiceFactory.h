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


#include "usServiceRegistration.h"

US_BEGIN_NAMESPACE

/**
 * \ingroup MicroServices
 *
 * Allows services to provide customized service objects in the module
 * environment.
 *
 * <p>
 * When registering a service, a <code>ServiceFactory</code> object can be
 * used instead of a service object, so that the module developer can gain
 * control of the specific service object granted to a module that is using the
 * service.
 *
 * <p>
 * When this happens, the
 * <code>ModuleContext::GetService(const ServiceReference&)</code> method calls the
 * <code>ServiceFactory::GetService</code> method to create a service object
 * specifically for the requesting module. The service object returned by the
 * <code>ServiceFactory</code> is cached by the framework until the module
 * releases its use of the service.
 *
 * <p>
 * When the module's use count for the service equals zero (including the module
 * unloading or the service being unregistered), the
 * <code>ServiceFactory::UngetService</code> method is called.
 *
 * <p>
 * <code>ServiceFactory</code> objects are only used by the framework and are
 * not made available to other modules in the module environment. The framework
 * may concurrently call a <code>ServiceFactory</code>.
 *
 * @see ModuleContext#GetService
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
   * <code>module</code> requests a service object using the
   * <code>ModuleContext::GetService(const ServiceReference&)</code> method. The
   * service factory can then return a specific service object for each
   * module.
   *
   * <p>
   * The framework caches the value returned (unless it is 0),
   * and will return the same service object on any future call to
   * <code>ModuleContext::GetService</code> for the same modules. This means the
   * framework must not allow this method to be concurrently called for the
   * same module.
   *
   * @param module The module using the service.
   * @param registration The <code>ServiceRegistration</code> object for the
   *        service.
   * @return A service object that <strong>must</strong> be an instance of all
   *         the classes named when the service was registered.
   * @see ModuleContext#GetService
   */
  virtual US_BASECLASS_NAME* GetService(Module* module, const ServiceRegistration& registration) = 0;

  /**
   * Releases a service object.
   *
   * <p>
   * The framework invokes this method when a service has been released by a
   * module. The service object may then be destroyed.
   *
   * @param module The Module releasing the service.
   * @param registration The <code>ServiceRegistration</code> object for the
   *        service.
   * @param service The service object returned by a previous call to the
   *        <code>ServiceFactory::getService</code> method.
   * @see ModuleContext#UngetService
   */
  virtual void UngetService(Module* module, const ServiceRegistration& registration,
                            US_BASECLASS_NAME* service) = 0;
};

US_END_NAMESPACE
