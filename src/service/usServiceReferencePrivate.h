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


#ifndef USSERVICEREFERENCEPRIVATE_H
#define USSERVICEREFERENCEPRIVATE_H

#include "usAtomicInt.h"

#include "usServiceProperties.h"


US_BEGIN_NAMESPACE

class Module;
class ServiceRegistrationPrivate;
class ServiceReferencePrivate;


/**
 * \ingroup MicroServices
 */
class ServiceReferencePrivate
{
public:

  ServiceReferencePrivate(ServiceRegistrationPrivate* reg);

  ~ServiceReferencePrivate();

  /**
    * Get the service object.
    *
    * @param module requester of service.
    * @return Service requested or null in case of failure.
    */
  US_BASECLASS_NAME* GetService(Module* module);

  /**
   * Unget the service object.
   *
   * @param module Module who wants remove service.
   * @param checkRefCounter If true decrement refence counter and remove service
   *                        if we reach zero. If false remove service without
   *                        checking refence counter.
   * @return True if service was remove or false if only refence counter was
   *         decremented.
   */
  bool UngetService(Module* module, bool checkRefCounter);

  /**
   * Get all properties registered with this service.
   *
   * @return A ServiceProperties object containing properties or being empty
   *         if service has been removed.
   */
  ServiceProperties GetProperties() const;

  /**
   * Returns the property value to which the specified property key is mapped
   * in the properties <code>ServiceProperties</code> object of the service
   * referenced by this <code>ServiceReference</code> object.
   *
   * <p>
   * Property keys are case-insensitive.
   *
   * <p>
   * This method must continue to return property values after the service has
   * been unregistered. This is so references to unregistered services can
   * still be interrogated.
   *
   * @param key The property key.
   * @param lock If <code>true</code>, access of the properties of the service
   * referenced by this <code>ServiceReference</code> object will be
   * synchronized.
   * @return The property value to which the key is mapped; an invalid Any
   * if there is no property named after the key.
   */
  Any GetProperty(const std::string& key, bool lock) const;

  /**
   * Reference count for implicitly shared private implementation.
   */
  AtomicInt ref;

  /**
   * Link to registration object for this reference.
   */
  ServiceRegistrationPrivate* const registration;

private:

  // purposely not implemented
  ServiceReferencePrivate(const ServiceReferencePrivate&);
  ServiceReferencePrivate& operator=(const ServiceReferencePrivate&);
};

US_END_NAMESPACE

#endif // USSERVICEREFERENCEPRIVATE_H
