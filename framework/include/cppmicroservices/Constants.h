/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

#ifndef CPPMICROSERVICES_CONSTANTS_H
#define CPPMICROSERVICES_CONSTANTS_H

#include "cppmicroservices/FrameworkConfig.h"

#include <string>

namespace cppmicroservices {

/**
 * Defines standard names for the CppMicroServices environment system properties,
 * service properties, and Manifest header attribute keys.
 *
 * <p>
 * The values associated with these keys are of type {@code std::string}, unless
 * otherwise indicated.
 *
 * \ingroup MicroServices
 */
namespace Constants {

/**
 * Location identifier of the OSGi <i>system bundle </i>, which is defined
 * to be &quot;System Bundle&quot;.
 */
US_Framework_EXPORT extern const std::string SYSTEM_BUNDLE_LOCATION; // = "System Bundle";

/**
 * Alias for the symbolic name of the OSGi <i>system bundle </i>. It is
 * defined to be &quot;system.bundle&quot;.
 */
US_Framework_EXPORT extern const std::string SYSTEM_BUNDLE_SYMBOLICNAME; // = "system_bundle";

/**
 * Manifest header identifying the bundle's category.
 * <p>
 * The header value may be retrieved via the {@code Bundle::GetProperty}
 * method.
 */
US_Framework_EXPORT extern const std::string BUNDLE_CATEGORY; // = "bundle.category";

/**
 * Manifest header identifying the bundle's copyright information.
 * <p>
 * The header value may be retrieved via the {@code Bundle::GetProperty}
 * method.
 */
US_Framework_EXPORT extern const std::string BUNDLE_COPYRIGHT; // = "bundle.copyright";

/**
 * Manifest header containing a brief description of the bundle's
 * functionality.
 * <p>
 * The header value may be retrieved via the {@code Bundle::GetProperty}
 * method.
 */
US_Framework_EXPORT extern const std::string BUNDLE_DESCRIPTION; // = "bundle.description";

/**
 * Manifest header identifying the bundle's menifest version.
 * <p>
 * The header value may be retrieved via the {@code Bundle::GetProperty}
 * method.
 */
US_Framework_EXPORT extern const std::string BUNDLE_MANIFESTVERSION; // = "bundle.manifest_version";

/**
 * Manifest header identifying the bundle's name.
 * <p>
 * The header value may be retrieved via the {@code Bundle::GetProperty}
 * method.
 */
US_Framework_EXPORT extern const std::string BUNDLE_NAME; // = "bundle.name";

/**
 * Manifest header identifying the bundle's vendor.
 *
 * <p>
 * The header value may be retrieved via the {@code Bundle::GetProperty}
 * method.
 */
US_Framework_EXPORT extern const std::string BUNDLE_VENDOR; // = "bundle.vendor";

/**
 * Manifest header identifying the bundle's version.
 *
 * <p>
 * The header value may be retrieved via the {@code Bundle::GetProperty}
 * method.
 */
US_Framework_EXPORT extern const std::string BUNDLE_VERSION; // = "bundle.version";

/**
 * Manifest header identifying the bundle's documentation URL, from which
 * further information about the bundle may be obtained.
 *
 * <p>
 * The header value may be retrieved via the {@code Bundle::GetProperty}
 * method.
 */
US_Framework_EXPORT extern const std::string BUNDLE_DOCURL; // = "bundle.doc_url";

/**
 * Manifest header identifying the contact address where problems with the
 * bundle may be reported; for example, an email address.
 *
 * <p>
 * The header value may be retrieved via the {@code Bundle::GetProperty}
 * method.
 */
US_Framework_EXPORT extern const std::string BUNDLE_CONTACTADDRESS; // = "bundle.contact_address";

/**
 * Manifest header identifying the bundle's symbolic name.
 *
 * <p>
 * The header value may be retrieved via the {@code Bundle::GetProperty}
 * method.
 */
US_Framework_EXPORT extern const std::string BUNDLE_SYMBOLICNAME; // = "bundle.symbolic_name";

/**
 * Manifest header identifying the bundle's activation policy.
 * <p>
 * The header value may be retrieved via the {@code Bundle::GetProperty}
 * method.
 *
 * @see #ACTIVATION_LAZY
 */
US_Framework_EXPORT extern const std::string BUNDLE_ACTIVATIONPOLICY; // = "bundle.activation_policy";

/**
 * Bundle activation policy declaring the bundle must be activated when the
 * library containing it is loaded into memory.
 * <p>
 * A bundle with the lazy activation policy that is started with the
 * {@link Bundle#START_ACTIVATION_POLICY START_ACTIVATION_POLICY} option
 * will wait in the {@link Bundle#STATE_STARTING STATE_STARTING} state until its
 * library is loaded. The bundle will then be activated.
 * <p>
 * The activation policy value is specified as in the
 * bundle.activation_policy manifest header like:
 *
 * <pre>
 *       bundle: { activation_policy: "lazy" }
 * </pre>
 *
 * @see #BUNDLE_ACTIVATIONPOLICY
 * @see Bundle#Start(uint32_t)
 * @see Bundle#START_ACTIVATION_POLICY
 */
US_Framework_EXPORT extern const std::string ACTIVATION_LAZY; // = "lazy";

/**
 * Framework environment property identifying the Framework version.
 *
 * <p>
 * The header value may be retrieved via the {@code Bundle::GetProperty}
 * method.
 */
US_Framework_EXPORT extern const std::string FRAMEWORK_VERSION; // = "org.cppmicroservices.framework.version";

/**
 * Framework environment property identifying the Framework implementation
 * vendor.
 *
 * <p>
 * The header value may be retrieved via the {@code Bundle::GetProperty}
 * method.
 */
US_Framework_EXPORT extern const std::string FRAMEWORK_VENDOR; // = "org.cppmicroservices.framework.vendor";

/**
 * Framework launching property specifying the persistent storage area used
 * by the framework. The value of this property must be a valid file path in
 * the file system to a directory. If the specified directory does not exist
 * then the framework will create the directory. If the specified path
 * exists but is not a directory or if the framework fails to create the
 * storage directory, then framework initialization fails. This area can not
 * be shared with anything else.
 * <p>
 * If this property is not set, the framework uses the "fwdir" directory in
 * the current working directory for the persistent storage area.
 */
US_Framework_EXPORT extern const std::string FRAMEWORK_STORAGE; // = "org.cppmicroservices.framework.storage";

/**
 * Framework launching property specifying if and when the persistent
 * storage area for the framework should be cleaned. If this property is not
 * set, then the framework storage area must not be cleaned.
 *
 * @see #FRAMEWORK_STORAGE_CLEAN_ONFIRSTINIT
 */
US_Framework_EXPORT extern const std::string FRAMEWORK_STORAGE_CLEAN; // = "org.cppmicroservices.framework.storage.clean";

/**
 * Specifies that the framework storage area must be cleaned before the
 * framework is initialized for the first time. Subsequent inits, starts or
 * updates of the framework will not result in cleaning the framework
 * storage area.
 */
US_Framework_EXPORT extern const std::string FRAMEWORK_STORAGE_CLEAN_ONFIRSTINIT; // = "onFirstInit";

/**
 * The framework's threading support property key name.
 * This property's default value is "single".
 * Valid key values are:
 * - "single" - The framework APIs are not thread-safe.
 * - "multi" - The framework APIs are thread-safe.
 *
 * @remarks This is a read-only property and cannot be altered at run-time.
 * The key's value is set at compile time by the US_ENABLE_THREADING_SUPPORT option.
 *
 * @see \ref BuildInstructions
 */
US_Framework_EXPORT extern const std::string FRAMEWORK_THREADING_SUPPORT; // = "org.cppmicroservices.framework.threading.support";

/**
 * The framework's log property key name.
 * This property's default value is off (boolean 'false').
 *
 * @see DIAG_LOG
 */
US_Framework_EXPORT extern const std::string FRAMEWORK_LOG; // = "org.cppmicroservices.framework.log";

/**
 * Framework environment property identifying the Framework's universally
 * unique identifier (UUID). A UUID represents a 128-bit value. A new UUID
 * is generated by the {@link Framework#Init()} method each time a framework
 * is initialized. The value of this property conforms to the UUID
 * string representation specified in <a
 * href="http://www.ietf.org/rfc/rfc4122.txt">RFC 4122</a>.
 *
 * <p>
 * The header value may be retrieved via the {@code Bundle::GetProperty}
 * method.
 */
US_Framework_EXPORT extern const std::string FRAMEWORK_UUID; // = "org.cppmicroservices.framework.uuid";


/*
 * Service properties.
 */

/**
 * Service property identifying all of the class names under which a service
 * was registered in the Framework. The value of this property must be of
 * type <code>std::vector&lt;std::string&gt;</code>.
 *
 * <p>
 * This property is set by the Framework when a service is registered.
 */
US_Framework_EXPORT extern const std::string OBJECTCLASS; // = "objectclass";

/**
 * Service property identifying a service's registration number. The value
 * of this property must be of type <code>long int</code>.
 *
 * <p>
 * The value of this property is assigned by the Framework when a service is
 * registered. The Framework assigns a unique value that is larger than all
 * previously assigned values since the Framework was started. These values
 * are NOT persistent across restarts of the Framework.
 */
US_Framework_EXPORT extern const std::string SERVICE_ID; // = "service.id";

/**
 * Service property identifying a service's persistent identifier.
 *
 * <p>
 * This property may be supplied in the {@code properties}
 * {@code ServiceProperties} object passed to the
 * {@code BundleContext#RegisterService} method. The value of this property
 * must be of type {@code std::string} or {@code std::vector<std::string>}.
 *
 * <p>
 * A service's persistent identifier uniquely identifies the service and
 * persists across multiple Framework invocations.
 *
 * <p>
 * By convention, every bundle has its own unique namespace, starting with
 * the bundle's identifier (see {@link Bundle#GetBundleId()}) and followed
 * by a dot (.). A bundle may use this as the prefix of the persistent
 * identifiers for the services it registers.
 */
US_Framework_EXPORT extern const std::string SERVICE_PID; // = "service.pid";

/**
 * Service property identifying a service's ranking number.
 *
 * <p>
 * This property may be supplied in the
 * <code>ServiceProperties</code> object passed to the
 * <code>BundleContext::RegisterService</code> method. The value of this
 * property must be of type <code>int</code>.
 *
 * <p>
 * The service ranking is used by the framework to determine the <i>natural
 * order</i> of services, see ServiceReference::operator<(const ServiceReference&),
 * and the <i>default</i> service to be returned from a call to the
 * {@link BundleContext::GetServiceReference} method.
 *
 * <p>
 * The default ranking is zero (0). A service with a ranking of
 * <code>std::numeric_limits<int>::max()</code> is very likely to be returned as the
 * default service, whereas a service with a ranking of
 * <code>std::numeric_limits<int>::min()</code> is very unlikely to be returned.
 *
 * <p>
 * If the supplied property value is not of type <code>int</code>, it is
 * deemed to have a ranking value of zero.
 */
US_Framework_EXPORT extern const std::string SERVICE_RANKING; // = "service.ranking";

/**
 * Service property identifying a service's vendor.
 *
 * <p>
 * This property may be supplied in the properties {@code ServiceProperties} object
 * passed to the {@code BundleContext#RegisterService} method.
 */
US_Framework_EXPORT extern const std::string SERVICE_VENDOR; //	= "service.vendor";

/**
 * Service property identifying a service's description.
 *
 * <p>
 * This property may be supplied in the properties {@code ServiceProperties} object
 * passed to the {@code BundleContext#RegisterService} method.
 */
US_Framework_EXPORT extern const std::string SERVICE_DESCRIPTION; // = "service.description";

/**
 * Service property identifying a service's scope.
 * This property is set by the framework when a service is registered. If the
 * registered object implements PrototypeServiceFactory, then the value of this
 * service property will be SCOPE_PROTOTYPE. Otherwise, if the registered
 * object implements ServiceFactory, then the value of this service property will
 * be SCOPE_BUNDLE. Otherwise, the value of this service property will be
 * SCOPE_SINGLETON.
 */
US_Framework_EXPORT extern const std::string SERVICE_SCOPE; // = "service.scope"

/**
 * Service scope is singleton. All bundles using the service receive the same
 * service object.
 *
 * @see SERVICE_SCOPE
 */
US_Framework_EXPORT extern const std::string SCOPE_SINGLETON; // = "singleton"

/**
 * Service scope is bundle. Each bundle using the service receives a distinct
 * service object.
 *
 * @see SERVICE_SCOPE
 */
US_Framework_EXPORT extern const std::string SCOPE_BUNDLE; // = "bundle"

/**
 * Service scope is prototype. Each bundle using the service receives either
 * a distinct service object or can request multiple distinct service objects
 * via ServiceObjects.
 *
 * @see SERVICE_SCOPE
 */
US_Framework_EXPORT extern const std::string SCOPE_PROTOTYPE; // = "prototype"

}

}

#endif // CPPMICROSERVICES_CONSTANTS_H
