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


#ifndef USSERVICEREGISTRATIONBASEPRIVATE_H
#define USSERVICEREGISTRATIONBASEPRIVATE_H

#include "usServiceInterface.h"
#include "usServiceReference.h"
#include "usServicePropertiesImpl_p.h"
#include "usAtomicInt_p.h"

US_BEGIN_NAMESPACE

class ModulePrivate;
class ServiceRegistrationBase;

/**
 * \ingroup MicroServices
 */
class ServiceRegistrationBasePrivate
{

protected:

  friend class ServiceRegistrationBase;

  // The ServiceReferenceBasePrivate class holds a pointer to a
  // ServiceRegistrationBasePrivate instance and needs to manipulate
  // its reference count. This way it can keep the ServiceRegistrationBasePrivate
  // instance alive and keep returning service properties for
  // unregistered service instances.
  friend class ServiceReferenceBasePrivate;

  /**
   * Reference count for implicitly shared private implementation.
   */
  AtomicInt ref;

  /**
   * Service or ServiceFactory object.
   */
  InterfaceMap service;

public:

  typedef US_UNORDERED_MAP_TYPE<Module*,int> ModuleToRefsMap;
  typedef US_UNORDERED_MAP_TYPE<Module*, InterfaceMap> ModuleToServiceMap;
  typedef US_UNORDERED_MAP_TYPE<Module*, std::list<InterfaceMap> > ModuleToServicesMap;

  /**
   * Modules dependent on this service. Integer is used as
   * reference counter, counting number of unbalanced getService().
   */
  ModuleToRefsMap dependents;

  /**
   * Object instances that a prototype factory has produced.
   */
  ModuleToServicesMap prototypeServiceInstances;

  /**
   * Object instance with module scope that a factory may have produced.
   */
  ModuleToServiceMap moduleServiceInstance;

  /**
   * Module registering this service.
   */
  ModulePrivate* module;

  /**
   * Reference object to this service registration.
   */
  ServiceReferenceBase reference;

  /**
   * Service properties.
   */
  ServicePropertiesImpl properties;

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
  Mutex eventLock;

  // needs to be recursive
  Mutex propsLock;

  ServiceRegistrationBasePrivate(ModulePrivate* module, const InterfaceMap& service,
                                 const ServicePropertiesImpl& props);

  ~ServiceRegistrationBasePrivate();

  /**
   * Check if a module uses this service
   *
   * @param p Module to check
   * @return true if module uses this service
   */
  bool IsUsedByModule(Module* m) const;

  const InterfaceMap& GetInterfaces() const;

  void* GetService(const std::string& interfaceId) const;

private:

  // purposely not implemented
  ServiceRegistrationBasePrivate(const ServiceRegistrationBasePrivate&);
  ServiceRegistrationBasePrivate& operator=(const ServiceRegistrationBasePrivate&);

};

US_END_NAMESPACE


#endif // USSERVICEREGISTRATIONBASEPRIVATE_H
