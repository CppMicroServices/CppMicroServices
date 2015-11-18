/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

#ifndef USSERVICEOBJECTS_H
#define USSERVICEOBJECTS_H

#include <usCoreExport.h>

#include <usBundleContext.h>
#include <usPrototypeServiceFactory.h>
#include <usServiceReference.h>
#include <usServiceProperties.h>

namespace us {

class ServiceObjectsBasePrivate;

class US_Core_EXPORT ServiceObjectsBase
{

private:

  ServiceObjectsBasePrivate* d;

protected:

  ServiceObjectsBase(BundleContext* context, const ServiceReferenceBase& reference);

  ServiceObjectsBase(const ServiceObjectsBase& other);

  ~ServiceObjectsBase();

  ServiceObjectsBase& operator=(const ServiceObjectsBase& other);

  // Called by ServiceObjects<S> with S != void
  std::shared_ptr<void> GetService() const;

  // Called by the ServiceObjects<void> specialization
  InterfaceMapConstPtr GetServiceInterfaceMap() const;

  ServiceReferenceBase GetReference() const;

};

/**
 * @ingroup MicroServices
 *
 * Allows multiple service objects for a service to be obtained.
 *
 * For services with \link ServiceConstants::SCOPE_PROTOTYPE prototype\endlink scope,
 * multiple service objects for the service can be obtained. For services with
 * \link ServiceConstants::SCOPE_SINGLETON singleton\endlink or
 * \link ServiceConstants::SCOPE_BUNDLE bundle \endlink scope, only one, use-counted
 * service object is available. Any unreleased service objects obtained from this
 * ServiceObjects object are automatically released by the framework when the bundles
 * associated with the BundleContext used to create this ServiceObjects object is
 * stopped.
 *
 * @tparam S Type of Service.
 */
template<class S>
class ServiceObjects : private ServiceObjectsBase
{

public:

  /**
   * Returns a service object for the referenced service.
   *
   * This ServiceObjects object can be used to obtain multiple service objects for
   * the referenced service if the service has \link ServiceConstants::SCOPE_PROTOTYPE prototype\endlink
   * scope. If the referenced service has \link ServiceConstants::SCOPE_SINGLETON singleton\endlink
   * or \link ServiceConstants::SCOPE_BUNDLE bundle\endlink scope, this method
   * behaves the same as calling the BundleContext::GetService(const ServiceReferenceBase&)
   * method for the referenced service. That is, only one, use-counted service object
   * is available from this ServiceObjects object.
   *
   * This method will always return \c NULL when the referenced service has been unregistered.
   *
   * For a prototype scope service, the following steps are taken to get the service object:
   *
   * <ol>
   *   <li>If the referenced service has been unregistered, \c NULL is returned.</li>
   *   <li>The PrototypeServiceFactory::GetService(const std::shared_ptr<Bundle>&, const ServiceRegistrationBase&)
   *       method is called to create a service object for the caller.</li>
   *   <li>If the service object (an instance of InterfaceMap) returned by the
   *       PrototypeServiceFactory object is empty, does not contain all the interfaces
   *       named when the service was registered or the PrototypeServiceFactory object
   *       throws an exception, \c NULL is returned and a warning message is issued.</li>
   *   <li>The service object is returned.</li>
   * </ol>
   *
   * @return A \c shared_ptr to the service object.The returned \c shared_ptr
   *         is empty if the service is not registered, the service object returned by a
   *         ServiceFactory does not contain all the classes under which it was registered 
   *         or the ServiceFactory threw an exception.
   *
   * @throw std::logic_error If the BundleContext used to create this ServiceObjects object
   *        is no longer valid.
   *
   */
  std::shared_ptr<S> GetService() const
  {
    return std::static_pointer_cast<S>(this->ServiceObjectsBase::GetService());
  }

  /**
  * Returns the ServiceReference for this ServiceObjects object.
  *
  * @return The ServiceReference for this ServiceObjects object.
  */
  ServiceReference<S> GetServiceReference() const
  {
    return this->ServiceObjectsBase::GetReference();
  }

private:

  friend class BundleContext;

  ServiceObjects(BundleContext* context, const ServiceReference<S>& reference)
    : ServiceObjectsBase(context, reference)
  {}

};

/**
 * @ingroup MicroServices
 *
 * Allows multiple service objects for a service to be obtained.
 *
 * This is a specialization of the ServiceObjects class template for
 * "void", which maps to all service interface types.
 *
 * @see ServiceObjects
 */
template<>
class US_Core_EXPORT ServiceObjects<void> : private ServiceObjectsBase
{

public:

  /**
   * Returns a service object as a InterfaceMap instance for the referenced service.
   *
   * This method is the same as ServiceObjects<S>::GetService() except for the
   * return type. Further, this method will always return an empty InterfaeMap
   * object when the referenced service has been unregistered.
   *
   * @return A InterfaceMapConstPtr object for the referenced service, which is empty if
   *         the service is not registered, the InterfaceMap returned by a ServiceFactory
   *         does not contain all the classes under which the service object was
   *         registered or the ServiceFactory threw an exception.
   *
   * @throw std::logic_error If the BundleContext used to create this ServiceObjects object
   *        is no longer valid.
   *
   * @see ServiceObjects<S>::GetService()
   */
  InterfaceMapConstPtr GetService() const;

  /**
   * Returns the ServiceReference for this ServiceObjects object.
   *
   * @return The ServiceReference for this ServiceObjects object.
   */
  ServiceReferenceU GetServiceReference() const;

private:

  friend class BundleContext;

  ServiceObjects(BundleContext* context, const ServiceReferenceU& reference);

};

}

#endif // USSERVICEOBJECTS_H
