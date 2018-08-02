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

#ifndef CPPMICROSERVICES_WEBCONSOLECONSTANTS_H
#define CPPMICROSERVICES_WEBCONSOLECONSTANTS_H

#include <string>

#include "cppmicroservices/webconsole/WebConsoleExport.h"

namespace cppmicroservices {

/**
 * WebConsoleConstants provides some common constants that are used by plugin
 * developers.
 */
struct US_WebConsole_EXPORT WebConsoleConstants
{

  /**
   * The name of the service to register as to be used as a "plugin" for
   * the web console (value is "cppmicroservices::HttpServlet").
   */
  static std::string SERVICE_NAME;

  /**
   * The URI address label under which the Web Console plugin is called
   * (value is "org.cppmicroservices.webconsole.label").
   *
   * This service registration property must be set to a single non-empty
   * string value. Otherwise the {@link #SERVICE_NAME Servlet} services will
   * be ignored by the Web Console and not be used as a plugin.
   */
  static std::string PLUGIN_LABEL; // = "org.cppmicroservices.webconsole.label"

  /**
   * The title under which the Web Console plugin is called
   * (value is "org.cppmicroservices.webconsole.title").
   *
   * For {@link #SERVICE_NAME Servlet} services not extending the
   * {@link AbstractWebConsolePlugin} this property is required for the
   * service to be used as a plugin. Otherwise the service is just ignored
   * by the Web Console.
   *
   * For {@link #SERVICE_NAME Servlet} services extending from the
   * AbstractWebConsolePlugin abstract class this property is not
   * technically required. To support lazy service access, e.g. for plugins
   * implemented using the <i>Service Factory</i> pattern, the use
   * of this service registration property is encouraged.
   */
  static std::string PLUGIN_TITLE; // = "org.cppmicroservices.webconsole.title"

  /**
   * The category under which the Web Console plugin is listed in the top
   * navigation (value is "org.cppmicroservices.webconsole.category").
   *
   * For {@link #SERVICE_NAME Servlet} services not extending the
   * AbstractWebConsolePlugin this property is required to declare a
   * specific category. Otherwise the plugin is put into the default category.
   *
   * For {@link #SERVICE_NAME Servlet} services extending from the
   * AbstractWebConsolePlugin abstract class this property is not
   * technically required. To support lazy service access with categorization,
   * e.g. for plugins implemented using the <i>Service Factory</i>
   * pattern, the use of this service registration property is strongly
   * encouraged. If the property is missing the
   * AbstractWebConsolePlugin#GetCategory() is called which should be
   * overwritten.
   */
  static std::string
    PLUGIN_CATEGORY; // = "org.cppmicroservices.webconsole.category"

  /**
   * The name of the service registration properties providing references
   * to addition CSS files that should be loaded when rendering the header
   * for a registered plugin.
   *
   * This property is expected to be a single string value or a vector of
   * string values.
   *
   * This service registration property is only used for plugins registered
   * as #SERVICE_NAME services which do not extend the AbstractWebConsolePlugin.
   * Extensions of the AbstractWebConsolePlugin should overwrite the
   * AbstractWebConsolePlugin#GetCssReferences() method to provide
   * additional CSS resources.
   */
  static std::string
    PLUGIN_CSS_REFERENCES; // = "org.cppmicroservices.webconsole.css"

  /**
   * The name of the request attribute providing the absolute path of the
   * Web Console root (value is "org.cppmicroservices.webconsole.appRoot"). This consists of
   * the servlet context path (from <code>HttpServletRequest#GetContextPath()</code>)
   * and the Web Console servlet path (from
   * <code>HttpServletRequest::GetServletPath()</code>,
   * <code>/us/console</code> by default).
   *
   * The type of this request attribute is <code>std::string</code>.
   */
  static std::string
    ATTR_APP_ROOT; // = "org.cppmicroservices.webconsole.appRoot"

  /**
   * The name of the request attribute providing the absolute path of the
   * current plugin (value is "org.cppmicroservices.webconsole.pluginRoot"). This consists of
   * the servlet context path (from <code>HttpServletRequest::GetContextPath()</code>),
   * the configured path of the web console root (<code>/us/console</code>
   * by default) and the plugin label #PLUGIN_LABEL.
   *
   * The type of this request attribute is <code>std::string</code>.
   */
  static std::string
    ATTR_PLUGIN_ROOT; // = "org.cppmicroservices.webconsole.pluginRoot"

  /**
   * The name of the request attribute providing a mapping of labels to page
   * titles of registered console plugins (value is "org.cppmicroservices.webconsole.labelMap").
   * This map may be used to render a navigation of the console plugins as the
   * AbstractWebConsolePlugin#RenderTopNavigation(HttpServletRequest&, std::ostream&)
   * method does.
   *
   * The type of this request attribute is <code>AnyMap</code>.
   */
  static std::string
    ATTR_LABEL_MAP; // = "org.cppmicroservices.webconsole.labelMap"

  /**
   * The name of the request attribute holding the WebConsoleVariableResolver
   * for the request (value is "org.cppmicroservices.webconsole.variable.resolver").
   *
   * @see WebConsoleVariableResolver
   * @see AbstractWebConsolePlugin#GetVariableResolver
   * @see AbstractWebConsolePlugin#SetVariableResolver
   */
  static std::string
    ATTR_CONSOLE_VARIABLE_RESOLVER; // = "org.cppmicroservices.webconsole.variable.resolver"
};
}

#endif // CPPMICROSERVICES_WEBCONSOLECONSTANTS_H
