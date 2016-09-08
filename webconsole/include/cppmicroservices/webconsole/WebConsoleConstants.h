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
   * the OSGi Manager (value is "javax.servlet.Servlet").
   */
  static std::string SERVICE_NAME();

  /**
   * The URI address label under which the OSGi Manager plugin is called by
   * the OSGi Manager (value is "org.cppmicroservices.webconsole.label").
   * <p>
   * This service registration property must be set to a single non-empty
   * String value. Otherwise the {@link #SERVICE_NAME Servlet} services will
   * be ignored by the Felix Web Console and not be used as a plugin.
   */
  static std::string PLUGIN_LABEL(); // = "org.cppmicroservices.webconsole.label"

  /**
   * The title under which the OSGi Manager plugin is called by
   * the OSGi Manager (value is "org.cppmicroservices.webconsole.title").
   * <p>
   * For {@link #SERVICE_NAME Servlet} services not extending the
   * {@link AbstractWebConsolePlugin} this property is required for the
   * service to be used as a plugin. Otherwise the service is just ignored
   * by the Felix Web Console.
   * <p>
   * For {@link #SERVICE_NAME Servlet} services extending from the
   * {@link AbstractWebConsolePlugin} abstract class this property is not
   * technically required. To support lazy service access, e.g. for plugins
   * implemented using the OSGi <i>Service Factory</i> pattern, the use
   * of this service registration property is encouraged.
   */
  static std::string PLUGIN_TITLE(); // = "org.cppmicroservices.webconsole.title"

  /**
   * The category under which the OSGi Manager plugin is listed in the top
   * navigation by the OSGi Manager (value is "org.cppmicroservices.webconsole.category").
   * <p>
   * For {@link #SERVICE_NAME Servlet} services not extending the
   * {@link AbstractWebConsolePlugin} this property is required to declare a
   * specific category. Otherwise the plugin is put into the default category.
   * <p>
   * For {@link #SERVICE_NAME Servlet} services extending from the
   * {@link AbstractWebConsolePlugin} abstract class this property is not
   * technically required. To support lazy service access with categorization,
   * e.g. for plugins implemented using the OSGi <i>Service Factory</i>
   * pattern, the use of this service registration property is strongly
   * encouraged. If the property is missing the
   * {@link AbstractWebConsolePlugin#getCategory()} is called which should be
   * overritten.
   */
  static std::string PLUGIN_CATEGORY(); // = "org.cppmicroservices.webconsole.category"

  /**
   * The name of the service registration properties providing references
   * to addition CSS files that should be loaded when rendering the header
   * for a registered plugin.
   * <p>
   * This property is expected to be a single string value, array of string
   * values or a Collection (or Vector) of string values.
   * <p>
   * This service registration property is only used for plugins registered
   * as {@link #SERVICE_NAME} services which do not extend the
   * {@link AbstractWebConsolePlugin}. Extensions of the
   * {@link AbstractWebConsolePlugin} should overwrite the
   * {@link AbstractWebConsolePlugin#getCssReferences()} method to provide
   * additional CSS resources.
   */
  static std::string PLUGIN_CSS_REFERENCES(); // = "org.cppmicroservices.webconsole.css"

  /**
   * The name of the request attribute providing the absolute path of the
   * Web Console root (value is "org.cppmicroservices.webconsole.appRoot"). This consists of
   * the servlet context path (from <code>HttpServletRequest.getContextPath()</code>)
   * and the Web Console servlet path (from
   * <code>HttpServletRequest.getServletPath()</code>,
   * <code>/system/console</code> by default).
   * <p>
   * The type of this request attribute is <code>String</code>.
   */
  static std::string ATTR_APP_ROOT(); // = "org.cppmicroservices.webconsole.appRoot"

  /**
   * The name of the request attribute providing the absolute path of the
   * current plugin (value is "org.cppmicroservices.webconsole.pluginRoot"). This consists of
   * the servlet context path (from <code>ServletRequest.getContextPath()</code>),
   * the configured path of the web console root (<code>/system/console</code>
   * by default) and the plugin label {@link #PLUGIN_LABEL}.
   * <p>
   * The type of this request attribute is <code>String</code>.
   */
  static std::string ATTR_PLUGIN_ROOT(); // = "org.cppmicroservices.webconsole.pluginRoot"

  /**
   * The name of the request attribute providing a mapping of labels to page
   * titles of registered console plugins (value is "org.cppmicroservices.webconsole.labelMap").
   * This map may be used to render a navigation of the console plugins as the
   * {@link AbstractWebConsolePlugin#renderTopNavigation(javax.servlet.http.HttpServletRequest, java.io.PrintWriter)}
   * method does.
   * <p>
   * The type of this request attribute is <code>Map<String, String></code>.
   */
  static std::string ATTR_LABEL_MAP(); // = "org.cppmicroservices.webconsole.labelMap"

  /**
   * The name of the request attribute holding the {@link VariableResolver}
   * for the request (value is "org.cppmicroservices.webconsole.variable.resolver").
   *
   * @see VariableResolver
   * @see WebConsoleUtil#getVariableResolver(javax.servlet.ServletRequest)
   * @see WebConsoleUtil#setVariableResolver(javax.servlet.ServletRequest, VariableResolver)
   */
  static std::string ATTR_CONSOLE_VARIABLE_RESOLVER(); // = "org.cppmicroservices.webconsole.variable.resolver"

  /**
   * The name of the request attribute holding the language {@link java.util.Map}
   * for the request (value is "org.cppmicroservices.webconsole.langMap").
   *
   * This map contains the web console supported languages, which are automatically detected.
   * The keys of the map are the language codes, like "en", "en_US" .. and so-on.
   * The value for each key is the locale user-friendly name - exactly the same as
   * returned by {@link java.util.Locale#getDisplayLanguage()}.
   *
   * The automatic detection of languages is very simple. It relies on having a
   * 'res/flags/[lang].gif' file in the bundle. So translators should not only provide
   * localized l10n/bundle.properties but also a flag image.
   *
   * The image should be obtained from http://famfamfam.com/lab/icons/flags/ and eventually
   * renamed to the correct locale.
   */
  static std::string ATTR_LANG_MAP(); // = "org.cppmicroservices.webconsole.langMap"
};

}

#endif // CPPMICROSERVICES_WEBCONSOLECONSTANTS_H
