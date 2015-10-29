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

#include <usBundleActivator.h>

#include "usSettingsPlugin_p.h"
#include "usServicesPlugin_p.h"

#include <usWebConsoleServlet_p.h>
#include <usBundleContext.h>

namespace us {

class WebConsoleActivator : public BundleActivator
{
public:

  void Start(us::BundleContext* context);
  void Stop(us::BundleContext* context);

private:

  std::shared_ptr<HttpServlet> m_WebConsoleServlet;

  std::shared_ptr<SettingsPlugin> m_SettingsPlugin;
  std::shared_ptr<ServicesPlugin> m_ServicesPlugin;
};

void WebConsoleActivator::Start(us::BundleContext* context)
{
  m_SettingsPlugin.reset(new SettingsPlugin);
  m_ServicesPlugin.reset(new ServicesPlugin);
  m_WebConsoleServlet.reset(new WebConsoleServlet());
  us::ServiceProperties props;
  props[HttpServlet::PROP_CONTEXT_ROOT()] = std::string("/console");
  context->RegisterService<HttpServlet>(m_WebConsoleServlet, props);

  std::cout << "****** Registering WebConsoleServlet at /console" << std::endl;

  m_SettingsPlugin->Register();
  m_ServicesPlugin->Register();

//  server->addHandler("/Console/bundles/", new BundlesHtml(context));
//  server->addHandler("/Console/resources/", new ResourcesHtml(context));
//  server->addHandler("/Console/", new ConsoleHtmlHandler(context));
//  server->addHandler("/", new DefaultHandler(context));
}

void WebConsoleActivator::Stop(us::BundleContext* /*context*/)
{
}

}

US_EXPORT_BUNDLE_ACTIVATOR(us::WebConsoleActivator)
