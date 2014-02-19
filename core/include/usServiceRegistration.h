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
template<class I1, class I2 = void, class I3 = void>
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

  ///@{
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
  ServiceReference<I1> GetReference(InterfaceType<I1>) const
  {
    return this->ServiceRegistrationBase::GetReference(us_service_interface_iid<I1>());
  }
  ServiceReference<I2> GetReference(InterfaceType<I2>) const
  {
    return this->ServiceRegistrationBase::GetReference(us_service_interface_iid<I2>());
  }
  ServiceReference<I3> GetReference(InterfaceType<I3>) const
  {
    return this->ServiceRegistrationBase::GetReference(us_service_interface_iid<I3>());
  }
  ///@}

  using ServiceRegistrationBase::operator=;


private:

  friend class ModuleContext;

  ServiceRegistration(const ServiceRegistrationBase& base)
    : ServiceRegistrationBase(base)
  {
  }

};

/// \cond
template<class I1, class I2>
class ServiceRegistration<I1, I2, void> : public ServiceRegistrationBase
{

public:

  ServiceRegistration() : ServiceRegistrationBase()
  {
  }

  ServiceReference<I1> GetReference(InterfaceType<I1>) const
  {
    return ServiceReference<I1>(this->ServiceRegistrationBase::GetReference(us_service_interface_iid<I1>()));
  }

  ServiceReference<I2> GetReference(InterfaceType<I2>) const
  {
    return ServiceReference<I2>(this->ServiceRegistrationBase::GetReference(us_service_interface_iid<I2>()));
  }

  using ServiceRegistrationBase::operator=;


private:

  friend class ModuleContext;

  ServiceRegistration(const ServiceRegistrationBase& base)
    : ServiceRegistrationBase(base)
  {
  }

};

template<class I1>
class ServiceRegistration<I1, void, void> : public ServiceRegistrationBase
{

public:

  ServiceRegistration() : ServiceRegistrationBase()
  {
  }

  ServiceReference<I1> GetReference() const
  {
    return this->GetReference(InterfaceType<I1>());
  }

  ServiceReference<I1> GetReference(InterfaceType<I1>) const
  {
    return ServiceReference<I1>(this->ServiceRegistrationBase::GetReference(us_service_interface_iid<I1>()));
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
class ServiceRegistration<void, void, void> : public ServiceRegistrationBase
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
/// \endcond

/**
 * \ingroup MicroServices
 *
 * A service registration object of unknown type.
 */
typedef ServiceRegistration<void> ServiceRegistrationU;

US_END_NAMESPACE

#endif // USSERVICEREGISTRATION_H
