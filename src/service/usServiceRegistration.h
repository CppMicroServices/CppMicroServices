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

#ifndef USSERVICEREGISTRATION_H
#define USSERVICEREGISTRATION_H

#include "usServiceRegistrationBase.h"

US_BEGIN_NAMESPACE

/**
 * \ingroup MicroServices
 *
 * A registered service.
 *
 * <p>
 * The framework returns a <code>ServiceRegistration</code> object when a
 * <code>ModuleContext#RegisterService()</code> method invocation is successful.
 * The <code>ServiceRegistration</code> object is for the private use of the
 * registering module and should not be shared with other modules.
 * <p>
 * The <code>ServiceRegistration</code> object may be used to update the
 * properties of the service or to unregister the service.
 *
 * @tparam S Class tyoe of the service interface
 * @see ModuleContext#RegisterService()
 * @remarks This class is thread safe.
 */
template<class S, class T = void>
class ServiceRegistration : public ServiceRegistrationBase
{

public:

  /**
   * Creates an invalid ServiceRegistration object. You can use
   * this object in boolean expressions and it will evaluate to
   * <code>false</code>.
   */
  ServiceRegistration() : ServiceRegistrationBase()
  {
  }

  /**
   * Returns a <code>ServiceReference</code> object for a service being
   * registered.
   * <p>
   * The <code>ServiceReference</code> object may be shared with other
   * modules.
   *
   * @throws std::logic_error If this
   *         <code>ServiceRegistration</code> object has already been
   *         unregistered or if it is invalid.
   * @return <code>ServiceReference</code> object.
   */
  ServiceReference<S> GetReference(InterfaceT<S>) const
  {
    return ServiceReference<S>(this->GetReferenceBase(us_service_interface_iid<S>()));
  }

  ServiceReference<T> GetReference(InterfaceT<T>) const
  {
    return ServiceReference<T>(this->GetReference(us_service_interface_iid<T>()));
  }

  using ServiceRegistrationBase::operator=;


private:

  friend class ModuleContext;

  ServiceRegistration(const ServiceRegistrationBase& base)
    : ServiceRegistrationBase(base)
  {
  }

};

template<class S>
class ServiceRegistration<S, void> : public ServiceRegistrationBase
{

public:

  ServiceRegistration() : ServiceRegistrationBase()
  {
  }

  ServiceReference<S> GetReference() const
  {
    return this->GetReference(InterfaceT<S>());
  }

  ServiceReference<S> GetReference(InterfaceT<S>) const
  {
    return ServiceReference<S>(this->GetReference(us_service_interface_iid<S>()));
  }

  using ServiceRegistrationBase::operator=;


private:

  friend class ModuleContext;

  ServiceRegistration(const ServiceRegistrationBase& base)
    : ServiceRegistrationBase(base)
  {
  }

};

template<>
class ServiceRegistration<void, void> : public ServiceRegistrationBase
{
public:

  /**
   * Creates an invalid ServiceReference object. You can use
   * this object in boolean expressions and it will evaluate to
   * <code>false</code>.
   */
  ServiceRegistration() : ServiceRegistrationBase()
  {
  }

  ServiceRegistration(const ServiceRegistrationBase& base)
    : ServiceRegistrationBase(base)
  {
  }

  using ServiceRegistrationBase::operator=;
};

typedef ServiceRegistration<void,void> ServiceRegistrationU;

US_END_NAMESPACE

#endif // USSERVICEREGISTRATION_H
