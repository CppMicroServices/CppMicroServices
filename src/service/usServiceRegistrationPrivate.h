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


#ifndef USSERVICEREGISTRATIONPRIVATE_H
#define USSERVICEREGISTRATIONPRIVATE_H

#include "usServiceReference.h"
#include "usServiceProperties.h"
#include "usAtomicInt_p.h"

US_BEGIN_NAMESPACE

class ModulePrivate;
class ServiceRegistration;

/**
 * \ingroup MicroServices
 */
class ServiceRegistrationPrivate
{

protected:

  friend class ServiceRegistration;

  // The ServiceReferencePrivate class holds a pointer to a
  // ServiceRegistrationPrivate instance and needs to manipulate
  // its reference count. This way it can keep the ServiceRegistrationPrivate
  // instance alive and keep returning service properties for
  // unregistered service instances.
  friend class ServiceReferencePrivate;

  /**
   * Reference count for implicitly shared private implementation.
   */
  AtomicInt ref;

  /**
   * Service or ServiceFactory object.
   */
  US_BASECLASS_NAME* service;

public:

  typedef Mutex MutexType;
  typedef MutexLock<MutexType> MutexLocker;

  typedef US_UNORDERED_MAP_TYPE<Module*,int> ModuleToRefsMap;
  typedef US_UNORDERED_MAP_TYPE<Module*, US_BASECLASS_NAME*> ModuleToServicesMap;

  /**
   * Modules dependent on this service. Integer is used as
   * reference counter, counting number of unbalanced getService().
   */
  ModuleToRefsMap dependents;

  /**
   * Object instances that factory has produced.
   */
  ModuleToServicesMap serviceInstances;

  /**
   * Module registering this service.
   */
  ModulePrivate* module;

  /**
   * Reference object to this service registration.
   */
  ServiceReference reference;

  /**
   * Service properties.
   */
  ServiceProperties properties;

  /**
   * Is service available. I.e., if <code>true</code> then holders
   * of a ServiceReference for the service are allowed to get it.
   */
  volatile bool available;

  /**
   * Avoid recursive unregistrations. I.e., if <code>true</code> then
   * unregistration of this service has started but is not yet
   * finished.
   */
  volatile bool unregistering;

  /**
   * Lock object for synchronous event delivery.
   */
  MutexType eventLock;

  // needs to be recursive
  MutexType propsLock;

  ServiceRegistrationPrivate(ModulePrivate* module, US_BASECLASS_NAME* service,
                             const ServiceProperties& props);

  ~ServiceRegistrationPrivate();

  /**
   * Check if a module uses this service
   *
   * @param p Module to check
   * @return true if module uses this service
   */
  bool IsUsedByModule(Module* m);

  US_BASECLASS_NAME* GetService();

private:

  // purposely not implemented
  ServiceRegistrationPrivate(const ServiceRegistrationPrivate&);
  ServiceRegistrationPrivate& operator=(const ServiceRegistrationPrivate&);

};

US_END_NAMESPACE


#endif // USSERVICEREGISTRATIONPRIVATE_H
