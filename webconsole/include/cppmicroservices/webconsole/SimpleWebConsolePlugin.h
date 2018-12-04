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

#ifndef CPPMICROSERVICES_SIMPLEWEBCONSOLEPLUGIN_H
#define CPPMICROSERVICES_SIMPLEWEBCONSOLEPLUGIN_H

#include "cppmicroservices/webconsole/AbstractWebConsolePlugin.h"

#include "cppmicroservices/ServiceRegistration.h"
#include "cppmicroservices/webconsole/WebConsoleExport.h"

namespace cppmicroservices {

/**
 * SimpleWebConsolePlugin is a utility class that provides a default
 * implementation of the AbstractWebConsolePlugin and supports the
 * following features:
 *
 *   - Methods for (un)registering the web console plugin service
 *   - Default implementation for resource loading
 *
 */
class US_WebConsole_EXPORT SimpleWebConsolePlugin
  : public AbstractWebConsolePlugin
{

public:
  /**
   * Creates new Simple Web Console Plugin with the given category.
   *
   * @param label the front label. See AbstractWebConsolePlugin#GetLabel()
   * @param title the plugin title . See AbstractWebConsolePlugin#GetTitle()
   * @param category the plugin's navigation category. See
   *        AbstractWebConsolePlugin#GetCategory()
   * @param css the additional plugin CSS. See
   *        AbstractWebConsolePlugin#GetCssReferences()
   */
  SimpleWebConsolePlugin(
    const std::string& label,
    const std::string& title,
    std::string  category = std::string(),
    std::vector<std::string>  css = std::vector<std::string>());

  /**
   * @see AbstractWebConsolePlugin#GetLabel()
   */
  std::string GetLabel() const;

  /**
   * @see AbstractWebConsolePlugin#GetTitle()
   */
  std::string GetTitle() const;

  /**
   * @see AbstractWebConsolePlugin#GetCategory()
   */
  std::string GetCategory() const;

  /**
   * This is an utility method. It is used to register the plugin service. Don't
   * forget to call #Unregister() when the plugin is no longer
   * needed.
   *
   * @param context the bundle context used for service registration.
   * @return A shared pointer to this plugin.
   */
  std::shared_ptr<SimpleWebConsolePlugin> Register(
    const BundleContext& context = GetBundleContext());

  /**
   * An utility method that removes the service, registered by the
   * #Register(const BundleContext&) method.
   */
  void Unregister();

protected:
  /**
   * @see AbstractWebConsolePlugin#GetCssReferences()
   */
  std::vector<std::string> GetCssReferences() const;

  BundleContext GetContext() const;

private:
  /**
   * Called internally by AbstractWebConsolePlugin to load resources.
   *
   * This particular implementation depends on the label. As example, if the
   * plugin is accessed as <code>/us/console/abc</code>, and the plugin
   * resources are accessed like <code>/us/console/abc/res/logo.gif</code>,
   * the code here will try load resource <code>/res/logo.gif</code> from the
   * bundle, providing the plugin.
   *
   * @param path the path to read.
   * @return the URL of the resource or <code>null</code> if not found.
   */
  BundleResource GetResource(const std::string& path) const;

  // used for standard AbstractWebConsolePlugin implementation
  std::string m_Label;
  std::string m_Title;
  std::string m_Category;
  std::vector<std::string> m_Css;
  std::string m_LabelRes;
  std::size_t m_LabelResLen;

  // used for service registration
  ServiceRegistration<HttpServlet> m_Reg;
  BundleContext m_Context;
};
}

#endif // CPPMICROSERVICES_SIMPLEWEBCONSOLEPLUGIN_H
