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

#ifndef USMODULECONTEXT_H_
#define USMODULECONTEXT_H_

// TODO: Replace includes with forward directives!

#include "usListenerFunctors_p.h"
#include "usServiceInterface.h"
#include "usServiceEvent.h"
#include "usServiceRegistration.h"
#include "usServiceException.h"
#include "usModuleEvent.h"

US_BEGIN_NAMESPACE

typedef US_SERVICE_LISTENER_FUNCTOR ServiceListener;
typedef US_MODULE_LISTENER_FUNCTOR ModuleListener;

class ModuleContextPrivate;
class ServiceFactory;

template<class S> class ServiceObjects;

/**
 * \ingroup MicroServices
 *
 * A module's execution context within the framework. The context is used to
 * grant access to other methods so that this module can interact with the
 * Micro Services Framework.
 *
 * <p>
 * <code>ModuleContext</code> methods allow a module to:
 * <ul>
 * <li>Subscribe to events published by the framework.
 * <li>Register service objects with the framework service registry.
 * <li>Retrieve <code>ServiceReference</code>s from the framework service
 * registry.
 * <li>Get and release service objects for a referenced service.
 * <li>Get the list of modules loaded in the framework.
 * <li>Get the {@link Module} object for a module.
 * </ul>
 *
 * <p>
 * A <code>ModuleContext</code> object will be created and provided to the
 * module associated with this context when it is loaded using the
 * {@link ModuleActivator::Load} method. The same <code>ModuleContext</code>
 * object will be passed to the module associated with this context when it is
 * unloaded using the {@link ModuleActivator::Unload} method. A
 * <code>ModuleContext</code> object is generally for the private use of its
 * associated module and is not meant to be shared with other modules in the
 * module environment.
 *
 * <p>
 * The <code>Module</code> object associated with a <code>ModuleContext</code>
 * object is called the <em>context module</em>.
 *
 * <p>
 * The <code>ModuleContext</code> object is only valid during the execution of
 * its context module; that is, during the period when the context module
 * is loaded. If the <code>ModuleContext</code>
 * object is used subsequently, a <code>std::logic_error</code> is
 * thrown. The <code>ModuleContext</code> object is never reused after
 * its context module is unloaded.
 *
 * <p>
 * The framework is the only entity that can create <code>ModuleContext</code>
 * objects.
 *
 * @remarks This class is thread safe.
 */
class US_Core_EXPORT ModuleContext
{

public:

  ~ModuleContext();

  /**
   * Returns the <code>Module</code> object associated with this
   * <code>ModuleContext</code>. This module is called the context module.
   *
   * @return The <code>Module</code> object associated with this
   *         <code>ModuleContext</code>.
   * @throws std::logic_error If this ModuleContext is no
   *         longer valid.
   */
  Module* GetModule() const;

  /**
   * Returns the module with the specified identifier.
   *
   * @param id The identifier of the module to retrieve.
   * @return A <code>Module</code> object or <code>0</code> if the
   *         identifier does not match any previously loaded module.
   */
  Module* GetModule(long id) const;

  /**
   * Get the module that with the specified module name.
   *
   * @param name The name of the module to get.
   * @return The requested \c Module or \c NULL.
   */
  Module* GetModule(const std::string& name);

  /**
   * Returns a list of all known modules.
   * <p>
   * This method returns a list of all modules loaded in the module
   * environment at the time of the call to this method. This list will
   * also contain modules which might already have been unloaded.
   *
   * @return A std::vector of <code>Module</code> objects which
   *         will hold one object per known module.
   */
  std::vector<Module*> GetModules() const;

  /**
   * Registers the specified service object with the specified properties
   * under the specified class names into the framework. A
   * <code>ServiceRegistration</code> object is returned. The
   * <code>ServiceRegistration</code> object is for the private use of the
   * module registering the service and should not be shared with other
   * modules. The registering module is defined to be the context module.
   * Other modules can locate the service by using either the
   * {@link #GetServiceReferences} or {@link #GetServiceReference} method.
   *
   * <p>
   * A module can register a service object that implements the
   * ServiceFactory or PrototypeServiceFactory interface to have more
   * flexibility in providing service objects to other modules.
   *
   * <p>
   * The following steps are taken when registering a service:
   * <ol>
   * <li>The framework adds the following service properties to the service
   * properties from the specified <code>ServiceProperties</code> (which may be
   * omitted): <br/>
   * A property named ServiceConstants#SERVICE_ID() identifying the
   * registration number of the service <br/>
   * A property named ServiceConstants#OBJECTCLASS() containing all the
   * specified classes. <br/>
   * A property named ServiceConstants#SERVICE_SCOPE() identifying the scope
   * of the service. <br/>
   * Properties with these names in the specified <code>ServiceProperties</code> will
   * be ignored.
   * <li>The service is added to the framework service registry and may now be
   * used by other modules.
   * <li>A service event of type ServiceEvent#REGISTERED is fired.
   * <li>A <code>ServiceRegistration</code> object for this registration is
   * returned.
   * </ol>
   *
   * @note This is a low-level method and should normally not be used directly.
   *       Use one of the templated RegisterService methods instead.
   *
   * @param service The service object, which is a map of interface identifiers
   *        to raw service pointers.
   * @param properties The properties for this service. The keys in the
   *        properties object must all be <code>std::string</code> objects. See
   *        {@link ServiceConstants} for a list of standard service property keys.
   *        Changes should not be made to this object after calling this
   *        method. To update the service's properties the
   *        {@link ServiceRegistration::SetProperties} method must be called.
   *        The set of properties may be omitted if the service has
   *        no properties.
   * @return A <code>ServiceRegistration</code> object for use by the module
   *         registering the service to update the service's properties or to
   *         unregister the service.
   *
   * @throws std::invalid_argument If one of the following is true:
   *         <ul>
   *         <li><code>service</code> is <code>0</code>.
   *         <li><code>properties</code> contains case variants of the same key name.
   *         </ul>
   * @throws std::logic_error If this ModuleContext is no longer valid.
   *
   * @see ServiceRegistration
   * @see ServiceFactory
   * @see PrototypeServiceFactory
   */
  ServiceRegistrationU RegisterService(const InterfaceMap& service,
                                       const ServiceProperties& properties = ServiceProperties());

  /**
   * Registers the specified service object with the specified properties
   * using the specified template argument with the framework.
   *
   * <p>
   * This method is provided as a convenience when <code>service</code> will only be registered under
   * a single class name whose type is available to the caller. It is otherwise identical to
   * RegisterService(const InterfaceMap&, const ServiceProperties&) but should be preferred
   * since it avoids errors in the string literal identifying the class name or interface identifier.
   *
   * Example usage:
   * \snippet uServices-registration/main.cpp 1-1
   * \snippet uServices-registration/main.cpp 1-2
   *
   * @tparam S The type under which the service can be located.
   * @param service The service object or a ServiceFactory object.
   * @param properties The properties for this service.
   * @return A ServiceRegistration object for use by the module
   *         registering the service to update the service's properties or to
   *         unregister the service.
   * @throws std::logic_error If this ModuleContext is no longer valid.
   * @throws ServiceException If the service type \c S is invalid or the
   *         \c service object is NULL.
   *
   * @see RegisterService(const InterfaceMap&, const ServiceProperties&)
   */
  template<class S>
  ServiceRegistration<S> RegisterService(S* service, const ServiceProperties& properties = ServiceProperties())
  {
    InterfaceMap servicePointers = MakeInterfaceMap<S>(service);
    return RegisterService(servicePointers, properties);
  }

  /**
   * Registers the specified service object with the specified properties
   * using the specified template argument with the framework.
   *
   * <p>
   * This method is provided as a convenience when registering a service under
   * two interface classes whose type is available to the caller. It is otherwise identical to
   * RegisterService(const InterfaceMap&, const ServiceProperties&) but should be preferred
   * since it avoids errors in the string literal identifying the class name or interface identifier.
   *
   * Example usage:
   * \snippet uServices-registration/main.cpp 2-1
   * \snippet uServices-registration/main.cpp 2-2
   *
   * @tparam I1 The first interface type under which the service can be located.
   * @tparam I2 The second interface type under which the service can be located.
   * @param impl The service object or a ServiceFactory object.
   * @param properties The properties for this service.
   * @return A ServiceRegistration object for use by the module
   *         registering the service to update the service's properties or to
   *         unregister the service.
   * @throws std::logic_error If this ModuleContext is no longer valid.
   * @throws ServiceException If the service type \c S is invalid or the
   *         \c service object is NULL.
   *
   * @see RegisterService(const InterfaceMap&, const ServiceProperties&)
   */
  template<class I1, class I2, class Impl>
  ServiceRegistration<I1,I2> RegisterService(Impl* impl, const ServiceProperties& properties = ServiceProperties())
  {
    InterfaceMap servicePointers = MakeInterfaceMap<I1, I2>(impl);
    return RegisterService(servicePointers, properties);
  }

  /**
   * Registers the specified service object with the specified properties
   * using the specified template argument with the framework.
   *
   * <p>
   * This method is identical to the RegisterService<I1,I2,Impl>(Impl*, const ServiceProperties&)
   * method except that it supports three service interface types.
   *
   * @tparam I1 The first interface type under which the service can be located.
   * @tparam I2 The second interface type under which the service can be located.
   * @tparam I3 The third interface type under which the service can be located.
   * @param impl The service object or a ServiceFactory object.
   * @param properties The properties for this service.
   * @return A ServiceRegistration object for use by the module
   *         registering the service to update the service's properties or to
   *         unregister the service.
   * @throws std::logic_error If this ModuleContext is no longer valid.
   * @throws ServiceException If the service type \c S is invalid or the
   *         \c service object is NULL.
   *
   * @see RegisterService(const InterfaceMap&, const ServiceProperties&)
   */
  template<class I1, class I2, class I3, class Impl>
  ServiceRegistration<I1,I2,I3> RegisterService(Impl* impl, const ServiceProperties& properties = ServiceProperties())
  {
    InterfaceMap servicePointers = MakeInterfaceMap<I1, I2, I3>(impl);
    return RegisterService(servicePointers, properties);
  }

  /**
   * Registers the specified service factory as a service with the specified properties
   * using the specified template argument as service interface type with the framework.
   *
   * <p>
   * This method is provided as a convenience when <code>factory</code> will only be registered under
   * a single class name whose type is available to the caller. It is otherwise identical to
   * RegisterService(const InterfaceMap&, const ServiceProperties&) but should be preferred
   * since it avoids errors in the string literal identifying the class name or interface identifier.
   *
   * Example usage:
   * \snippet uServices-registration/main.cpp 1-1
   * \snippet uServices-registration/main.cpp f1
   *
   * @tparam S The type under which the service can be located.
   * @param factory The ServiceFactory or PrototypeServiceFactory object.
   * @param properties The properties for this service.
   * @return A ServiceRegistration object for use by the module
   *         registering the service to update the service's properties or to
   *         unregister the service.
   * @throws std::logic_error If this ModuleContext is no longer valid.
   * @throws ServiceException If the service type \c S is invalid or the
   *         \c service factory object is NULL.
   *
   * @see RegisterService(const InterfaceMap&, const ServiceProperties&)
   */
  template<class S>
  ServiceRegistration<S> RegisterService(ServiceFactory* factory, const ServiceProperties& properties = ServiceProperties())
  {
    InterfaceMap servicePointers = MakeInterfaceMap<S>(factory);
    return RegisterService(servicePointers, properties);
  }

  /**
   * Registers the specified service factory as a service with the specified properties
   * using the specified template argument as service interface type with the framework.
   *
   * <p>
   * This method is identical to the RegisterService<S>(ServiceFactory*, const ServiceProperties&)
   * method except that it supports two service interface types.
   *
   * Example usage:
   * \snippet uServices-registration/main.cpp 2-1
   * \snippet uServices-registration/main.cpp f2
   *
   * @tparam I1 The first interface type under which the service can be located.
   * @tparam I2 The second interface type under which the service can be located.
   * @param factory The ServiceFactory or PrototypeServiceFactory object.
   * @param properties The properties for this service.
   * @return A ServiceRegistration object for use by the module
   *         registering the service to update the service's properties or to
   *         unregister the service.
   * @throws std::logic_error If this ModuleContext is no longer valid.
   * @throws ServiceException If the service type \c S is invalid or the
   *         \c service factory object is NULL.
   *
   * @see RegisterService(const InterfaceMap&, const ServiceProperties&)
   */
  template<class I1, class I2>
  ServiceRegistration<I1,I2> RegisterService(ServiceFactory* factory, const ServiceProperties& properties = ServiceProperties())
  {
    InterfaceMap servicePointers = MakeInterfaceMap<I1,I2>(factory);
    return RegisterService(servicePointers, properties);
  }

  /**
   * Registers the specified service factory as a service with the specified properties
   * using the specified template argument as service interface type with the framework.
   *
   * <p>
   * This method is identical to the RegisterService<S>(ServiceFactory*, const ServiceProperties&)
   * method except that it supports three service interface types.
   *
   * @tparam I1 The first interface type under which the service can be located.
   * @tparam I2 The second interface type under which the service can be located.
   * @tparam I3 The third interface type under which the service can be located.
   * @param factory The ServiceFactory or PrototypeServiceFactory object.
   * @param properties The properties for this service.
   * @return A ServiceRegistration object for use by the module
   *         registering the service to update the service's properties or to
   *         unregister the service.
   * @throws std::logic_error If this ModuleContext is no longer valid.
   * @throws ServiceException If the service type \c S is invalid or the
   *         \c service factory object is NULL.
   *
   * @see RegisterService(const InterfaceMap&, const ServiceProperties&)
   */
  template<class I1, class I2, class I3>
  ServiceRegistration<I1,I2,I3> RegisterService(ServiceFactory* factory, const ServiceProperties& properties = ServiceProperties())
  {
    InterfaceMap servicePointers = MakeInterfaceMap<I1,I2,I3>(factory);
    return RegisterService(servicePointers, properties);
  }

  /**
   * Returns a list of <code>ServiceReference</code> objects. The returned
   * list contains services that
   * were registered under the specified class and match the specified filter
   * expression.
   *
   * <p>
   * The list is valid at the time of the call to this method. However since
   * the Micro Services framework is a very dynamic environment, services can be modified or
   * unregistered at any time.
   *
   * <p>
   * The specified <code>filter</code> expression is used to select the
   * registered services whose service properties contain keys and values
   * which satisfy the filter expression. See LDAPFilter for a description
   * of the filter syntax. If the specified <code>filter</code> is
   * empty, all registered services are considered to match the
   * filter. If the specified <code>filter</code> expression cannot be parsed,
   * an <code>std::invalid_argument</code> will be thrown with a human readable
   * message where the filter became unparsable.
   *
   * <p>
   * The result is a list of <code>ServiceReference</code> objects for all
   * services that meet all of the following conditions:
   * <ul>
   * <li>If the specified class name, <code>clazz</code>, is not
   * empty, the service must have been registered with the
   * specified class name. The complete list of class names with which a
   * service was registered is available from the service's
   * {@link ServiceConstants#OBJECTCLASS() objectClass} property.
   * <li>If the specified <code>filter</code> is not empty, the
   * filter expression must match the service.
   * </ul>
   *
   * @param clazz The class name with which the service was registered or
   *        an empty string for all services.
   * @param filter The filter expression or empty for all
   *        services.
   * @return A list of <code>ServiceReference</code> objects or
   *         an empty list if no services are registered which satisfy the
   *         search.
   * @throws std::invalid_argument If the specified <code>filter</code>
   *         contains an invalid filter expression that cannot be parsed.
   * @throws std::logic_error If this ModuleContext is no longer valid.
   */
  std::vector<ServiceReferenceU> GetServiceReferences(const std::string& clazz, const std::string& filter = std::string());

  /**
   * Returns a list of <code>ServiceReference</code> objects. The returned
   * list contains services that
   * were registered under the interface id of the template argument <code>S</code>
   * and match the specified filter expression.
   *
   * <p>
   * This method is identical to GetServiceReferences(const std::string&, const std::string&) except that
   * the class name for the service object is automatically deduced from the template argument.
   *
   * @tparam S The type under which the requested service objects must have been registered.
   * @param filter The filter expression or empty for all
   *        services.
   * @return A list of <code>ServiceReference</code> objects or
   *         an empty list if no services are registered which satisfy the
   *         search.
   * @throws std::invalid_argument If the specified <code>filter</code>
   *         contains an invalid filter expression that cannot be parsed.
   * @throws std::logic_error If this ModuleContext is no longer valid.
   * @throws ServiceException If the service type \c S is invalid.
   *
   * @see GetServiceReferences(const std::string&, const std::string&)
   */
  template<class S>
  std::vector<ServiceReference<S> > GetServiceReferences(const std::string& filter = std::string())
  {
    std::string clazz = us_service_interface_iid<S>();
    if (clazz.empty()) throw ServiceException("The service interface class has no US_DECLARE_SERVICE_INTERFACE macro");
    typedef std::vector<ServiceReferenceU> BaseVectorT;
    BaseVectorT serviceRefs = GetServiceReferences(clazz, filter);
    std::vector<ServiceReference<S> > result;
    for(BaseVectorT::const_iterator i = serviceRefs.begin(); i != serviceRefs.end(); ++i)
    {
      result.push_back(ServiceReference<S>(*i));
    }
    return result;
  }

  /**
   * Returns a <code>ServiceReference</code> object for a service that
   * implements and was registered under the specified class.
   *
   * <p>
   * The returned <code>ServiceReference</code> object is valid at the time of
   * the call to this method. However as the Micro Services framework is a very dynamic
   * environment, services can be modified or unregistered at any time.
   *
   * <p>
   * This method is the same as calling
   * {@link ModuleContext::GetServiceReferences(const std::string&, const std::string&)} with an
   * empty filter expression. It is provided as a convenience for
   * when the caller is interested in any service that implements the
   * specified class.
   * <p>
   * If multiple such services exist, the service with the highest ranking (as
   * specified in its ServiceConstants::SERVICE_RANKING() property) is returned.
   * <p>
   * If there is a tie in ranking, the service with the lowest service ID (as
   * specified in its ServiceConstants::SERVICE_ID() property); that is, the
   * service that was registered first is returned.
   *
   * @param clazz The class name with which the service was registered.
   * @return A <code>ServiceReference</code> object, or an invalid <code>ServiceReference</code> if
   *         no services are registered which implement the named class.
   * @throws std::logic_error If this ModuleContext is no longer valid.
   * @throws ServiceException If no service was registered under the given class name.
   *
   * @see #GetServiceReferences(const std::string&, const std::string&)
   */
  ServiceReferenceU GetServiceReference(const std::string& clazz);

  /**
   * Returns a <code>ServiceReference</code> object for a service that
   * implements and was registered under the specified template class argument.
   *
   * <p>
   * This method is identical to GetServiceReference(const std::string&) except that
   * the class name for the service object is automatically deduced from the template argument.
   *
   * @tparam S The type under which the requested service must have been registered.
   * @return A <code>ServiceReference</code> object, or an invalid <code>ServiceReference</code> if
   *         no services are registered which implement the type <code>S</code>.
   * @throws std::logic_error If this ModuleContext is no longer valid.
   * @throws ServiceException It no service was registered under the given class name.
   * @see #GetServiceReference(const std::string&)
   * @see #GetServiceReferences(const std::string&)
   */
  template<class S>
  ServiceReference<S> GetServiceReference()
  {
    std::string clazz = us_service_interface_iid<S>();
    if (clazz.empty()) throw ServiceException("The service interface class has no US_DECLARE_SERVICE_INTERFACE macro");
    return ServiceReference<S>(GetServiceReference(clazz));
  }

  /**
   * Returns the service object referenced by the specified
   * <code>ServiceReferenceBase</code> object.
   * <p>
   * A module's use of a service is tracked by the module's use count of that
   * service. Each time a service's service object is returned by
   * {@link #GetService(const ServiceReference<S>&)} the context module's use count for
   * that service is incremented by one. Each time the service is released by
   * {@link #UngetService(const ServiceReferenceBase&)} the context module's use count
   * for that service is decremented by one.
   * <p>
   * When a module's use count for a service drops to zero, the module should
   * no longer use that service.
   *
   * <p>
   * This method will always return <code>0</code> when the service
   * associated with this <code>reference</code> has been unregistered.
   *
   * <p>
   * The following steps are taken to get the service object:
   * <ol>
   * <li>If the service has been unregistered, <code>0</code> is returned.
   * <li>The context module's use count for this service is incremented by
   * one.
   * <li>If the context module's use count for the service is currently one
   * and the service was registered with an object implementing the
   * <code>ServiceFactory</code> interface, the
   * {@link ServiceFactory::GetService} method is
   * called to create a service object for the context module. This service
   * object is cached by the framework. While the context module's use count
   * for the service is greater than zero, subsequent calls to get the
   * services's service object for the context module will return the cached
   * service object. <br>
   * If the <code>ServiceFactory</code> object throws an
   * exception, <code>0</code> is returned and a warning is logged.
   * <li>The service object for the service is returned.
   * </ol>
   *
   * @param reference A reference to the service.
   * @return A service object for the service associated with
   *         <code>reference</code> or <code>0</code> if the service is not
   *         registered or the <code>ServiceFactory</code> threw
   *         an exception.
   * @throws std::logic_error If this ModuleContext is no
   *         longer valid.
   * @throws std::invalid_argument If the specified
   *         <code>ServiceReferenceBase</code> is invalid (default constructed).
   * @see #UngetService(const ServiceReferenceBase&)
   * @see ServiceFactory
   */
  void* GetService(const ServiceReferenceBase& reference);

  InterfaceMap GetService(const ServiceReferenceU& reference);

  /**
   * Returns the service object referenced by the specified
   * <code>ServiceReference</code> object.
   * <p>
   * This is a convenience method which is identical to void* GetService(const ServiceReferenceBase&)
   * except that it casts the service object to the supplied template argument type
   *
   * @tparam S The type the service object will be cast to.
   * @return A service object for the service associated with
   *         <code>reference</code> or <code>0</code> if the service is not
   *         registered, the <code>ServiceFactory</code> threw
   *         an exception or the service could not be casted to the desired type.
   * @throws std::logic_error If this ModuleContext is no
   *         longer valid.
   * @throws std::invalid_argument If the specified
   *         <code>ServiceReference</code> is invalid (default constructed).
   * @see #GetService(const ServiceReferenceBase&)
   * @see #UngetService(const ServiceReferenceBase&)
   * @see ServiceFactory
   */
  template<class S>
  S* GetService(const ServiceReference<S>& reference)
  {
    const ServiceReferenceBase& baseRef = reference;
    return reinterpret_cast<S*>(GetService(baseRef));
  }

  /**
   * Returns the ServiceObjects object for the service referenced by the specified
   * ServiceReference object. The ServiceObjects object can be used to obtain
   * multiple service objects for services with prototype scope. For services with
   * singleton or module scope, the ServiceObjects::GetService() method behaves
   * the same as the GetService(const ServiceReference<S>&) method and the
   * ServiceObjects::UngetService(const ServiceReferenceBase&) method behaves the
   * same as the UngetService(const ServiceReferenceBase&) method. That is, only one,
   * use-counted service object is available from the ServiceObjects object.
   *
   * @tparam S Type of Service.
   * @param reference A reference to the service.
   * @return A ServiceObjects object for the service associated with the specified
   * reference or an invalid instance if the service is not registered.
   * @throws std::logic_error If this ModuleContext is no longer valid.
   * @throws std::invalid_argument If the specified ServiceReference is invalid
   * (default constructed or the service has been unregistered)
   *
   * @see PrototypeServiceFactory
   */
  template<class S>
  ServiceObjects<S> GetServiceObjects(const ServiceReference<S>& reference)
  {
    return ServiceObjects<S>(this, reference);
  }

  /**
   * Releases the service object referenced by the specified
   * <code>ServiceReference</code> object. If the context module's use count
   * for the service is zero, this method returns <code>false</code>.
   * Otherwise, the context modules's use count for the service is decremented
   * by one.
   *
   * <p>
   * The service's service object should no longer be used and all references
   * to it should be destroyed when a module's use count for the service drops
   * to zero.
   *
   * <p>
   * The following steps are taken to unget the service object:
   * <ol>
   * <li>If the context module's use count for the service is zero or the
   * service has been unregistered, <code>false</code> is returned.
   * <li>The context module's use count for this service is decremented by
   * one.
   * <li>If the context module's use count for the service is currently zero
   * and the service was registered with a <code>ServiceFactory</code> object,
   * the ServiceFactory#UngetService
   * method is called to release the service object for the context module.
   * <li><code>true</code> is returned.
   * </ol>
   *
   * @param reference A reference to the service to be released.
   * @return <code>false</code> if the context module's use count for the
   *         service is zero or if the service has been unregistered;
   *         <code>true</code> otherwise.
   * @throws std::logic_error If this ModuleContext is no
   *         longer valid.
   * @see #GetService
   * @see ServiceFactory
   */
  bool UngetService(const ServiceReferenceBase& reference);

  void AddServiceListener(const ServiceListener& delegate,
                          const std::string& filter = std::string());
  void RemoveServiceListener(const ServiceListener& delegate);

  void AddModuleListener(const ModuleListener& delegate);
  void RemoveModuleListener(const ModuleListener& delegate);

  /**
   * Adds the specified <code>callback</code> with the
   * specified <code>filter</code> to the context modules's list of listeners.
   * See LDAPFilter for a description of the filter syntax. Listeners
   * are notified when a service has a lifecycle state change.
   *
   * <p>
   * You must take care to remove registered listeners befor the <code>receiver</code>
   * object is destroyed. However, the Micro Services framework takes care
   * of removing all listeners registered by this context module's classes
   * after the module is unloaded.
   *
   * <p>
   * If the context module's list of listeners already contains a pair <code>(r,c)</code>
   * of <code>receiver</code> and <code>callback</code> such that
   * <code>(r == receiver && c == callback)</code>, then this
   * method replaces that callback's filter (which may be empty)
   * with the specified one (which may be empty).
   *
   * <p>
   * The callback is called if the filter criteria is met. To filter based
   * upon the class of the service, the filter should reference the
   * ServiceConstants#OBJECTCLASS() property. If <code>filter</code> is
   * empty, all services are considered to match the filter.
   *
   * <p>
   * When using a <code>filter</code>, it is possible that the
   * <code>ServiceEvent</code>s for the complete lifecycle of a service
   * will not be delivered to the callback. For example, if the
   * <code>filter</code> only matches when the property <code>x</code> has
   * the value <code>1</code>, the callback will not be called if the
   * service is registered with the property <code>x</code> not set to the
   * value <code>1</code>. Subsequently, when the service is modified
   * setting property <code>x</code> to the value <code>1</code>, the
   * filter will match and the callback will be called with a
   * <code>ServiceEvent</code> of type <code>MODIFIED</code>. Thus, the
   * callback will not be called with a <code>ServiceEvent</code> of type
   * <code>REGISTERED</code>.
   *
   * @tparam R The type of the receiver (containing the member function to be called)
   * @param receiver The object to connect to.
   * @param callback The member function pointer to call.
   * @param filter The filter criteria.
   * @throws std::invalid_argument If <code>filter</code> contains an
   *         invalid filter string that cannot be parsed.
   * @throws std::logic_error If this ModuleContext is no
   *         longer valid.
   * @see ServiceEvent
   * @see RemoveServiceListener()
   */
  template<class R>
  void AddServiceListener(R* receiver, void(R::*callback)(const ServiceEvent),
                          const std::string& filter = std::string())
  {
    AddServiceListener(ServiceListenerMemberFunctor(receiver, callback),
                       static_cast<void*>(receiver), filter);
  }

  /**
   * Removes the specified <code>callback</code> from the context module's
   * list of listeners.
   *
   * <p>
   * If the <code>(receiver,callback)</code> pair is not contained in this
   * context module's list of listeners, this method does nothing.
   *
   * @tparam R The type of the receiver (containing the member function to be removed)
   * @param receiver The object from which to disconnect.
   * @param callback The member function pointer to remove.
   * @throws std::logic_error If this ModuleContext is no
   *         longer valid.
   * @see AddServiceListener()
   */
  template<class R>
  void RemoveServiceListener(R* receiver, void(R::*callback)(const ServiceEvent))
  {
    RemoveServiceListener(ServiceListenerMemberFunctor(receiver, callback),
                          static_cast<void*>(receiver));
  }

  /**
   * Adds the specified <code>callback</code> to the context modules's list
   * of listeners. Listeners are notified when a module has a lifecycle
   * state change.
   *
   * <p>
   * If the context module's list of listeners already contains a pair <code>(r,c)</code>
   * of <code>receiver</code> and <code>callback</code> such that
   * <code>(r == receiver && c == callback)</code>, then this method does nothing.
   *
   * @tparam R The type of the receiver (containing the member function to be called)
   * @param receiver The object to connect to.
   * @param callback The member function pointer to call.
   * @throws std::logic_error If this ModuleContext is no
   *         longer valid.
   * @see ModuleEvent
   */
  template<class R>
  void AddModuleListener(R* receiver, void(R::*callback)(const ModuleEvent))
  {
    AddModuleListener(ModuleListenerMemberFunctor(receiver, callback),
                      static_cast<void*>(receiver));
  }

  /**
   * Removes the specified <code>callback</code> from the context module's
   * list of listeners.
   *
   * <p>
   * If the <code>(receiver,callback)</code> pair is not contained in this
   * context module's list of listeners, this method does nothing.
   *
   * @tparam R The type of the receiver (containing the member function to be removed)
   * @param receiver The object from which to disconnect.
   * @param callback The member function pointer to remove.
   * @throws std::logic_error If this ModuleContext is no
   *         longer valid.
   * @see AddModuleListener()
   */
  template<class R>
  void RemoveModuleListener(R* receiver, void(R::*callback)(const ModuleEvent))
  {
    RemoveModuleListener(ModuleListenerMemberFunctor(receiver, callback),
                         static_cast<void*>(receiver));
  }

  /**
   * Get the absolute path for a file or directory in the persistent
   * storage area provided for the module. The returned path
   * might be empty if no storage path has been set previously.
   * If the path is non-empty, it is safe to assume that the path is writable.
   *
   * @see ModuleSettings::SetStoragePath(const std::string&)
   *
   * @param filename A relative name to the file or directory to be accessed.
   * @return The absolute path to the persistent storage area for the given file name.
   */
  std::string GetDataFile(const std::string& filename) const;


private:

  friend class Module;
  friend class ModulePrivate;

  ModuleContext(ModulePrivate* module);

  // purposely not implemented
  ModuleContext(const ModuleContext&);
  ModuleContext& operator=(const ModuleContext&);

  void AddServiceListener(const ServiceListener& delegate, void* data,
                          const std::string& filter);
  void RemoveServiceListener(const ServiceListener& delegate, void* data);

  void AddModuleListener(const ModuleListener& delegate, void* data);
  void RemoveModuleListener(const ModuleListener& delegate, void* data);

  ModuleContextPrivate * const d;
};

US_END_NAMESPACE

#endif /* USMODULECONTEXT_H_ */
