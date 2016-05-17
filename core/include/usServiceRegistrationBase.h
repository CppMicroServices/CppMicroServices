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

#ifndef USSERVICEREGISTRATIONBASE_H
#define USSERVICEREGISTRATIONBASE_H

#include "usServiceProperties.h"
#include "usServiceReference.h"

US_MSVC_PUSH_DISABLE_WARNING(4099) // type name first seen using 'struct' now seen using 'class'

namespace us {

class BundlePrivate;
class ServiceRegistrationBasePrivate;
class ServicePropertiesImpl;

/**
 * \ingroup MicroServices
 *
 * A registered service.
 *
 * <p>
 * The framework returns a <code>ServiceRegistrationBase</code> object when a
 * <code>BundleContext#RegisterService()</code> method invocation is successful.
 * The <code>ServiceRegistrationBase</code> object is for the private use of the
 * registering bundle and should not be shared with other bundles.
 * <p>
 * The <code>ServiceRegistrationBase</code> object may be used to update the
 * properties of the service or to unregister the service.
 *
 * \note This class is provided as public API for low-level service management only.
 *       In almost all cases you should use the template ServiceRegistration instead.
 *
 * @see BundleContext#RegisterService()
 */
class US_Core_EXPORT ServiceRegistrationBase
{

public:

  ServiceRegistrationBase(const ServiceRegistrationBase& reg);

  ServiceRegistrationBase(ServiceRegistrationBase&& reg);

  /**
   * A boolean conversion operator converting this ServiceRegistrationBase object
   * to \c true if it is valid and to \c false otherwise. A SeriveRegistration
   * object is invalid if it was default-constructed or was invalidated by
   * assigning 0 to it.
   *
   * \see operator=(std::nullptr_t)
   *
   * \return \c true if this ServiceRegistrationBase object is valid, \c false
   *         otherwise.
   */
  explicit operator bool() const;

  /**
   * Releases any resources held or locked by this
   * <code>ServiceRegistrationBase</code> and renders it invalid.
   *
   * \return This ServiceRegistrationBase object.
   */
  ServiceRegistrationBase& operator=(std::nullptr_t);

  ~ServiceRegistrationBase();

  /**
   * Returns a <code>ServiceReference</code> object for a service being
   * registered.
   * <p>
   * The <code>ServiceReference</code> object may be shared with other
   * bundles.
   *
   * @throws std::logic_error If this
   *         <code>ServiceRegistrationBase</code> object has already been
   *         unregistered or if it is invalid.
   * @return <code>ServiceReference</code> object.
   */
  ServiceReferenceBase GetReference(const std::string& interfaceId = std::string()) const;

  /**
   * Updates the properties associated with a service.
   *
   * <p>
   * The ServiceConstants#OBJECTCLASS and ServiceConstants#SERVICE_ID keys
   * cannot be modified by this method. These values are set by the framework
   * when the service is registered in the environment.
   *
   * <p>
   * The following steps are taken to modify service properties:
   * <ol>
   * <li>The service's properties are replaced with the provided properties.
   * <li>A service event of type ServiceEvent#MODIFIED is fired.
   * </ol>
   *
   * @param properties The properties for this service. See {@link ServiceProperties}
   *        for a list of standard service property keys. Changes should not
   *        be made to this object after calling this method. To update the
   *        service's properties this method should be called again.
   *
   * @throws std::logic_error If this <code>ServiceRegistrationBase</code>
   *         object has already been unregistered or if it is invalid.
   * @throws std::invalid_argument If <code>properties</code> contains
   *         case variants of the same key name.
   */
  void SetProperties(const ServiceProperties& properties);

  /**
   * Unregisters a service. Remove a <code>ServiceRegistrationBase</code> object
   * from the framework service registry. All <code>ServiceRegistrationBase</code>
   * objects associated with this <code>ServiceRegistrationBase</code> object
   * can no longer be used to interact with the service once unregistration is
   * complete.
   *
   * <p>
   * The following steps are taken to unregister a service:
   * <ol>
   * <li>The service is removed from the framework service registry so that
   * it can no longer be obtained.
   * <li>A service event of type ServiceEvent#UNREGISTERING is fired
   * so that bundles using this service can release their use of the service.
   * Once delivery of the service event is complete, the
   * <code>ServiceRegistrationBase</code> objects for the service may no longer be
   * used to get a service object for the service.
   * <li>For each bundle whose use count for this service is greater than
   * zero: <br>
   * The bundle's use count for this service is set to zero. <br>
   * If the service was registered with a ServiceFactory object, the
   * <code>ServiceFactory#UngetService</code> method is called to release
   * the service object for the bundle.
   * </ol>
   *
   * @throws std::logic_error If this
   *         <code>ServiceRegistrationBase</code> object has already been
   *         unregistered or if it is invalid.
   */
  void Unregister();

  /**
   * Compare two ServiceRegistrationBase objects.
   *
   * If both ServiceRegistrationBase objects are valid, the comparison is done
   * using the underlying ServiceReference object. Otherwise, this ServiceRegistrationBase
   * object is less than the other object if and only if this object is invalid and
   * the other object is valid.
   *
   * @param o The ServiceRegistrationBase object to compare with.
   * @return \c true if this ServiceRegistrationBase object is less than the other object.
   */
  bool operator<(const ServiceRegistrationBase& o) const;

  bool operator==(const ServiceRegistrationBase& registration) const;

  ServiceRegistrationBase& operator=(const ServiceRegistrationBase& registration);
  ServiceRegistrationBase& operator=(ServiceRegistrationBase&& registration);


private:

  friend class ServiceRegistry;
  friend class ServiceReferenceBasePrivate;

  template<class I1, class ...Interfaces> friend class ServiceRegistration;

  friend class ::std::hash<ServiceRegistrationBase>;

  /**
   * Creates an invalid ServiceRegistrationBase object. You can use
   * this object in boolean expressions and it will evaluate to
   * <code>false</code>.
   */
  ServiceRegistrationBase();

  ServiceRegistrationBase(ServiceRegistrationBasePrivate* registrationPrivate);

  ServiceRegistrationBase(BundlePrivate* bundle, const InterfaceMapConstPtr& service,
                          ServicePropertiesImpl&& props);

  ServiceRegistrationBasePrivate* d;

};

inline std::ostream& operator<<(std::ostream& os, const ServiceRegistrationBase& /*reg*/)
{
  return os << "us::ServiceRegistrationBase object";
}

}

US_MSVC_POP_WARNING

US_HASH_FUNCTION_BEGIN(us::ServiceRegistrationBase)
  return std::hash<us::ServiceRegistrationBasePrivate*>()(arg.d);
US_HASH_FUNCTION_END

#endif // USSERVICEREGISTRATIONBASE_H
