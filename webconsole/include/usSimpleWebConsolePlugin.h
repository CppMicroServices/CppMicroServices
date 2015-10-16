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

#ifndef USSIMPLEWEBCONSOLEPLUGIN_H
#define USSIMPLEWEBCONSOLEPLUGIN_H

#include "usAbstractWebConsolePlugin.h"

#include <usWebConsoleExport.h>
#include <usServiceRegistration.h>

namespace us {

/**
 * SimpleWebConsolePlugin is a utility class that provides a default
 * implementation of the AbstractWebConsolePlugin and supports the
 * following features:
 *
 *   - Methods for (un)registering the web console plugin service
 *   - Default implementation for resource loading
 *
 */
class US_WebConsole_EXPORT SimpleWebConsolePlugin : public AbstractWebConsolePlugin
{

public:

  /**
   * Creates new Simple Web Console Plugin with the given category.
   *
   * @param label the front label. See
   *            {@link AbstractWebConsolePlugin#getLabel()}
   * @param title the plugin title . See
   *            {@link AbstractWebConsolePlugin#getTitle()}
   * @param category the plugin's navigation category. See
   *            {@link AbstractWebConsolePlugin#getCategory()}
   * @param css the additional plugin CSS. See
   *            {@link AbstractWebConsolePlugin#getCssReferences()}
   */
  SimpleWebConsolePlugin(const std::string& label, const std::string& title,
                         const std::string& category = std::string(),
                         const std::vector<std::string>& css = std::vector<std::string>());

  /**
   * @see org.apache.felix.webconsole.AbstractWebConsolePlugin#getLabel()
   */
  std::string GetLabel() const;

  /**
   * @see org.apache.felix.webconsole.AbstractWebConsolePlugin#getTitle()
   */
   std::string GetTitle() const;

  /**
   * @see org.apache.felix.webconsole.AbstractWebConsolePlugin#getCategory()
   */
  std::string GetCategory() const;

  /**
   * This is an utility method. It is used to register the plugin service. Don't
   * forget to call the {@link #unregister()} when the plugin is no longer
   * needed.
   *
   * @param bc the bundle context used for service registration.
   * @return self
   */
  SimpleWebConsolePlugin* Register(BundleContext* context = GetBundleContext());


  /**
   * An utility method that removes the service, registered by the
   * {@link #register(BundleContext)} method.
   */
  void Unregister();

protected:

  /**
   * @see org.apache.felix.webconsole.AbstractWebConsolePlugin#getCssReferences()
   */
  std::vector<std::string> GetCssReferences() const;

  BundleContext* GetContext() const;

private:

  /**
   * Called internally by {@link AbstractWebConsolePlugin} to load resources.
   *
   * This particular implementation depends on the label. As example, if the
   * plugin is accessed as <code>/system/console/abc</code>, and the plugin
   * resources are accessed like <code>/system/console/abc/res/logo.gif</code>,
   * the code here will try load resource <code>/res/logo.gif</code> from the
   * bundle, providing the plugin.
   *
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
  BundleContext* m_Context;
};

}

#endif // USSIMPLEWEBCONSOLEPLUGIN_H
