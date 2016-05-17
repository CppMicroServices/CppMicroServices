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


#ifndef USSERVICEREFERENCEBASEPRIVATE_H
#define USSERVICEREFERENCEBASEPRIVATE_H

#include "usServiceInterface.h"

#include <string>
#include <atomic>

namespace us {

class Any;
class Bundle;
class ServicePropertiesHandle;
class ServiceRegistrationBasePrivate;
class ServiceReferenceBasePrivate;


/**
 * \ingroup MicroServices
 */
class ServiceReferenceBasePrivate
{
public:

  ServiceReferenceBasePrivate(const ServiceReferenceBasePrivate&) = delete;
  ServiceReferenceBasePrivate& operator=(const ServiceReferenceBasePrivate&) = delete;

  ServiceReferenceBasePrivate(ServiceRegistrationBasePrivate* reg);

  ~ServiceReferenceBasePrivate();

  /**
    * Get the service object.
    *
    * @param bundle requester of service.
    * @return Service requested or null in case of failure.
    */
  std::shared_ptr<void> GetService(const std::shared_ptr<Bundle>& bundle);

  InterfaceMapConstPtr GetServiceInterfaceMap(const std::shared_ptr<Bundle>& bundle);

  /**
    * Get new service instance.
    *
    * @param bundle requester of service.
    * @return Service requested or null in case of failure.
    */
  InterfaceMapConstPtr GetPrototypeService(const std::shared_ptr<Bundle>& bundle);

  /**
   * Unget the service object.
   *
   * @param bundle Bundle who wants remove service.
   * @param checkRefCounter If true decrement refence counter and remove service
   *                        if we reach zero. If false remove service without
   *                        checking refence counter.
   * @return True if service was removed or false if only reference counter was
   *         decremented.
   */
  bool UngetService(const std::shared_ptr<Bundle>& bundle, bool checkRefCounter);

  /**
   * Unget prototype scope service objects.
   *
   * @param bundle Bundle who wants to remove a prototype scope service.
   * @param service The prototype scope service pointer.
   * @return \c true if the service was removed, \c false otherwise.
   */
  bool UngetPrototypeService(const std::shared_ptr<Bundle>& bundle, const InterfaceMapConstPtr& service);

  /**
   * Get a handle to the locked service properties.
   *
   * @return A locked ServicePropertiesImpl handle object.
   */
  ServicePropertiesHandle GetProperties() const;

  bool IsConvertibleTo(const std::string& interfaceId) const;

  /**
   * Reference count for implicitly shared private implementation.
   */
  std::atomic<int> ref;

  /**
   * Link to registration object for this reference.
   */
  ServiceRegistrationBasePrivate* const registration;

  /**
   * The service interface id for this reference.
   */
  std::string interfaceId;

private:

  InterfaceMapConstPtr GetServiceFromFactory(const std::shared_ptr<Bundle>& bundle,
                                             const std::shared_ptr<ServiceFactory>& factory);

};

}

#endif // USSERVICEREFERENCEBASEPRIVATE_H
