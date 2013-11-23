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


#ifndef USSERVICEREGISTRY_H
#define USSERVICEREGISTRY_H

#include "usServiceInterface.h"
#include "usServiceRegistration.h"

#include "usThreads_p.h"

US_BEGIN_NAMESPACE

class CoreModuleContext;
class ModulePrivate;
class ServicePropertiesImpl;


/**
 * Here we handle all the CppMicroServices services that are registered.
 */
class ServiceRegistry
{

public:

  typedef Mutex MutexType;

  mutable MutexType mutex;

  /**
   * Creates a new ServiceProperties object containing <code>in</code>
   * with the keys converted to lower case.
   *
   * @param classes A list of class names which will be added to the
   *        created ServiceProperties object under the key
   *        ModuleConstants::OBJECTCLASS.
   * @param sid A service id which will be used instead of a default one.
   */
  static ServicePropertiesImpl CreateServiceProperties(const ServiceProperties& in,
                                                       const std::vector<std::string>& classes = std::vector<std::string>(),
                                                       bool isFactory = false, bool isPrototypeFactory = false, long sid = -1);

  typedef US_UNORDERED_MAP_TYPE<ServiceRegistrationBase, std::vector<std::string> > MapServiceClasses;
  typedef US_UNORDERED_MAP_TYPE<std::string, std::vector<ServiceRegistrationBase> > MapClassServices;

  /**
   * All registered services in the current framework.
   * Mapping of registered service to class names under which
   * the service is registerd.
   */
  MapServiceClasses services;

  std::vector<ServiceRegistrationBase> serviceRegistrations;

  /**
   * Mapping of classname to registered service.
   * The List of registered services are ordered with the highest
   * ranked service first.
   */
  MapClassServices classServices;

  CoreModuleContext* core;

  ServiceRegistry(CoreModuleContext* coreCtx);

  ~ServiceRegistry();

  void Clear();

  /**
   * Register a service in the framework wide register.
   *
   * @param module The module registering the service.
   * @param classes The class names under which the service can be located.
   * @param service The service object.
   * @param properties The properties for this service.
   * @return A ServiceRegistration object.
   * @exception std::invalid_argument If one of the following is true:
   * <ul>
   * <li>The service object is 0.</li>
   * <li>The service parameter is not a ServiceFactory or an
   * instance of all the named classes in the classes parameter.</li>
   * </ul>
   */
  ServiceRegistrationBase RegisterService(ModulePrivate* module,
                                          const InterfaceMap& service,
                                          const ServiceProperties& properties);

  /**
   * Service ranking changed, reorder registered services
   * according to ranking.
   *
   * @param serviceRegistration The ServiceRegistrationPrivate object.
   * @param rank New rank of object.
   */
  void UpdateServiceRegistrationOrder(const ServiceRegistrationBase& sr,
                                      const std::vector<std::string>& classes);

  /**
   * Get all services implementing a certain class.
   * Only used internally by the framework.
   *
   * @param clazz The class name of the requested service.
   * @return A sorted list of {@link ServiceRegistrationPrivate} objects.
   */
  void Get(const std::string& clazz, std::vector<ServiceRegistrationBase>& serviceRegs) const;

  /**
   * Get a service implementing a certain class.
   *
   * @param module The module requesting reference
   * @param clazz The class name of the requested service.
   * @return A {@link ServiceReference} object.
   */
  ServiceReferenceBase Get(ModulePrivate* module, const std::string& clazz) const;

  /**
   * Get all services implementing a certain class and then
   * filter these with a property filter.
   *
   * @param clazz The class name of requested service.
   * @param filter The property filter.
   * @param module The module requesting reference.
   * @return A list of {@link ServiceReference} object.
   */
  void Get(const std::string& clazz, const std::string& filter,
           ModulePrivate* module, std::vector<ServiceReferenceBase>& serviceRefs) const;

  /**
   * Remove a registered service.
   *
   * @param sr The ServiceRegistration object that is registered.
   */
  void RemoveServiceRegistration(const ServiceRegistrationBase& sr) ;

  /**
   * Get all services that a module has registered.
   *
   * @param p The module
   * @return A set of {@link ServiceRegistration} objects
   */
  void GetRegisteredByModule(ModulePrivate* m, std::vector<ServiceRegistrationBase>& serviceRegs) const;

  /**
   * Get all services that a module uses.
   *
   * @param p The module
   * @return A set of {@link ServiceRegistration} objects
   */
  void GetUsedByModule(Module* m, std::vector<ServiceRegistrationBase>& serviceRegs) const;

private:

  friend class ServiceHooks;

  void Get_unlocked(const std::string& clazz, std::vector<ServiceRegistrationBase>& serviceRegs) const;

  void Get_unlocked(const std::string& clazz, const std::string& filter,
                    ModulePrivate* module, std::vector<ServiceReferenceBase>& serviceRefs) const;

  // purposely not implemented
  ServiceRegistry(const ServiceRegistry&);
  ServiceRegistry& operator=(const ServiceRegistry&);

};

US_END_NAMESPACE

#endif // USSERVICEREGISTRY_H
