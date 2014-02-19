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


#ifndef US_SERVICE_PROPERTIES_H
#define US_SERVICE_PROPERTIES_H


#include <usCoreConfig.h>

#include <cctype>

#include "usAny.h"

US_BEGIN_NAMESPACE

/**
 * \ingroup MicroServices
 *
 * A hash table with std::string as the key type and Any as the value
 * type. It is typically used for passing service properties to
 * ModuleContext::RegisterService.
 */
typedef US_UNORDERED_MAP_TYPE<std::string, Any> ServiceProperties;

/**
 * \ingroup MicroServices
 */
namespace ServiceConstants {

/**
 * Service property identifying all of the class names under which a service
 * was registered in the framework. The value of this property must be of
 * type <code>std::vector&lt;std::string&gt;</code>.
 *
 * <p>
 * This property is set by the framework when a service is registered.
 */
US_Core_EXPORT const std::string& OBJECTCLASS(); // = "objectclass"

/**
 * Service property identifying a service's registration number. The value
 * of this property must be of type <code>long int</code>.
 *
 * <p>
 * The value of this property is assigned by the framework when a service is
 * registered. The framework assigns a unique value that is larger than all
 * previously assigned values since the framework was started. These values
 * are NOT persistent across restarts of the framework.
 */
US_Core_EXPORT const std::string& SERVICE_ID(); // = "service.id"

/**
 * Service property identifying a service's ranking number.
 *
 * <p>
 * This property may be supplied in the
 * <code>ServiceProperties</code> object passed to the
 * <code>ModuleContext::RegisterService</code> method. The value of this
 * property must be of type <code>int</code>.
 *
 * <p>
 * The service ranking is used by the framework to determine the <i>natural
 * order</i> of services, see ServiceReference::operator<(const ServiceReference&),
 * and the <i>default</i> service to be returned from a call to the
 * {@link ModuleContext::GetServiceReference} method.
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
US_Core_EXPORT const std::string& SERVICE_RANKING(); // = "service.ranking"

/**
 * Service property identifying a service's scope.
 * This property is set by the framework when a service is registered. If the
 * registered object implements PrototypeServiceFactory, then the value of this
 * service property will be SCOPE_PROTOTYPE(). Otherwise, if the registered
 * object implements ServiceFactory, then the value of this service property will
 * be SCOPE_MODULE(). Otherwise, the value of this service property will be
 * SCOPE_SINGLETON().
 */
US_Core_EXPORT const std::string& SERVICE_SCOPE(); // = "service.scope"

/**
 * Service scope is singleton. All modules using the service receive the same
 * service object.
 *
 * @see SERVICE_SCOPE()
 */
US_Core_EXPORT const std::string& SCOPE_SINGLETON(); // = "singleton"

/**
 * Service scope is module. Each module using the service receives a distinct
 * service object.
 *
 * @see SERVICE_SCOPE()
 */
US_Core_EXPORT const std::string& SCOPE_MODULE(); // = "module"

/**
 * Service scope is prototype. Each module using the service receives either
 * a distinct service object or can request multiple distinct service objects
 * via ServiceObjects.
 *
 * @see SERVICE_SCOPE()
 */
US_Core_EXPORT const std::string& SCOPE_PROTOTYPE(); // = "prototype"

}

US_END_NAMESPACE

#endif // US_SERVICE_PROPERTIES_H
