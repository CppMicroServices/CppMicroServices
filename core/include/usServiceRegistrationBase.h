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

#ifndef USSERVICEREGISTRATIONBASE_H
#define USSERVICEREGISTRATIONBASE_H

#include "usServiceProperties.h"
#include "usServiceReference.h"

US_MSVC_PUSH_DISABLE_WARNING(4396)

US_BEGIN_NAMESPACE

class ModulePrivate;
class ServiceRegistrationBasePrivate;
class ServicePropertiesImpl;

/**
 * \ingroup MicroServices
 *
 * A registered service.
 *
 * <p>
 * The framework returns a <code>ServiceRegistrationBase</code> object when a
 * <code>ModuleContext#RegisterService()</code> method invocation is successful.
 * The <code>ServiceRegistrationBase</code> object is for the private use of the
 * registering module and should not be shared with other modules.
 * <p>
 * The <code>ServiceRegistrationBase</code> object may be used to update the
 * properties of the service or to unregister the service.
 *
 * \note This class is provided as public API for low-level service management only.
 *       In almost all cases you should use the template ServiceRegistration instead.
 *
 * @see ModuleContext#RegisterService()
 * @remarks This class is thread safe.
 */
class US_Core_EXPORT ServiceRegistrationBase
{

private:

  typedef ServiceRegistrationBasePrivate* ServiceRegistrationBase::*bool_type;

public:

  ServiceRegistrationBase(const ServiceRegistrationBase& reg);

  /**
   * A boolean conversion operator converting this ServiceRegistrationBase object
   * to \c true if it is valid and to \c false otherwise. A SeriveRegistration
   * object is invalid if it was default-constructed or was invalidated by
   * assigning 0 to it.
   *
   * \see operator=(int)
   *
   * \return \c true if this ServiceRegistrationBase object is valid, \c false
   *         otherwise.
   */
  operator bool_type() const;

  /**
   * Releases any resources held or locked by this
   * <code>ServiceRegistrationBase</code> and renders it invalid.
   *
   * \return This ServiceRegistrationBase object.
   */
  ServiceRegistrationBase& operator=(int null);

  ~ServiceRegistrationBase();

  /**
   * Returns a <code>ServiceReference</code> object for a service being
   * registered.
   * <p>
   * The <code>ServiceReference</code> object may be shared with other
   * modules.
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
   * so that modules using this service can release their use of the service.
   * Once delivery of the service event is complete, the
   * <code>ServiceRegistrationBase</code> objects for the service may no longer be
   * used to get a service object for the service.
   * <li>For each module whose use count for this service is greater than
   * zero: <br>
   * The module's use count for this service is set to zero. <br>
   * If the service was registered with a ServiceFactory object, the
   * <code>ServiceFactory#UngetService</code> method is called to release
   * the service object for the module.
   * </ol>
   *
   * @throws std::logic_error If this
   *         <code>ServiceRegistrationBase</code> object has already been
   *         unregistered or if it is invalid.
   * @see ModuleContext#UngetService
   * @see ServiceFactory#UngetService
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


private:

  friend class ServiceRegistry;
  friend class ServiceReferenceBasePrivate;

  template<class I1, class I2, class I3> friend class ServiceRegistration;

  US_HASH_FUNCTION_FRIEND(ServiceRegistrationBase);

  /**
   * Creates an invalid ServiceRegistrationBase object. You can use
   * this object in boolean expressions and it will evaluate to
   * <code>false</code>.
   */
  ServiceRegistrationBase();

  ServiceRegistrationBase(ServiceRegistrationBasePrivate* registrationPrivate);

  ServiceRegistrationBase(ModulePrivate* module, const InterfaceMap& service,
                          const ServicePropertiesImpl& props);

  ServiceRegistrationBasePrivate* d;

};

US_END_NAMESPACE

US_MSVC_POP_WARNING

US_HASH_FUNCTION_NAMESPACE_BEGIN
US_HASH_FUNCTION_BEGIN(US_PREPEND_NAMESPACE(ServiceRegistrationBase))
  return US_HASH_FUNCTION(US_PREPEND_NAMESPACE(ServiceRegistrationBasePrivate)*, arg.d);
US_HASH_FUNCTION_END
US_HASH_FUNCTION_NAMESPACE_END


inline std::ostream& operator<<(std::ostream& os, const US_PREPEND_NAMESPACE(ServiceRegistrationBase)& /*reg*/)
{
  return os << "US_PREPEND_NAMESPACE(ServiceRegistrationBase) object";
}

#endif // USSERVICEREGISTRATIONBASE_H
