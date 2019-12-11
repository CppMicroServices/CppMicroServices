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

#include <memory>

#include "cppmicroservices/BundleActivator.h"

#include "BundlesPlugin.h"
#include "ServicesPlugin.h"
#include "SettingsPlugin.h"

#include "WebConsoleServlet.h"
#include "cppmicroservices/BundleContext.h"

namespace cppmicroservices {

class WebConsoleActivator : public BundleActivator
{
public:
  void Start(BundleContext context) override;
  void Stop(BundleContext context) override;

private:
  std::shared_ptr<HttpServlet> m_WebConsoleServlet;

  std::shared_ptr<SettingsPlugin> m_SettingsPlugin;
  std::shared_ptr<ServicesPlugin> m_ServicesPlugin;
  std::shared_ptr<BundlesPlugin> m_BundlesPlugin;
};

void WebConsoleActivator::Start(BundleContext context)
{
  m_SettingsPlugin = std::make_shared<SettingsPlugin>();
  m_ServicesPlugin = std::make_shared<ServicesPlugin>();
  m_BundlesPlugin = std::make_shared<BundlesPlugin>();
  m_WebConsoleServlet = std::make_shared<WebConsoleServlet>();
  cppmicroservices::ServiceProperties props;
  props[HttpServlet::PROP_CONTEXT_ROOT] = std::string("/console");
  context.RegisterService<HttpServlet>(m_WebConsoleServlet, props);

  std::cout << "****** Registering WebConsoleServlet at /console" << std::endl;

  m_SettingsPlugin->Register();
  m_ServicesPlugin->Register();
  m_BundlesPlugin->Register();

  //  server->addHandler("/Console/bundles/", new BundlesHtml(context));
  //  server->addHandler("/Console/resources/", new ResourcesHtml(context));
  //  server->addHandler("/Console/", new ConsoleHtmlHandler(context));
  //  server->addHandler("/", new DefaultHandler(context));
}

void WebConsoleActivator::Stop(BundleContext /*context*/) {}
}

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(cppmicroservices::WebConsoleActivator)
