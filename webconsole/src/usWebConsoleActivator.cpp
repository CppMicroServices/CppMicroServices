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

#include <usModuleActivator.h>

#include "usSettingsPlugin_p.h"
#include "usServicesPlugin_p.h"

#include <usWebConsoleServlet_p.h>
#include <usModuleContext.h>

namespace us {

class WebConsoleActivator : public ModuleActivator
{
public:

  void Load(us::ModuleContext* context);
  void Unload(us::ModuleContext* context);

private:

  HttpServlet* m_WebConsoleServlet;

  SettingsPlugin m_SettingsPlugin;
  ServicesPlugin m_ServicesPlugin;
};

void WebConsoleActivator::Load(us::ModuleContext* context)
{
  m_WebConsoleServlet = new WebConsoleServlet();
  us::ServiceProperties props;
  props[HttpServlet::PROP_CONTEXT_ROOT()] = std::string("/console");
  context->RegisterService<HttpServlet>(m_WebConsoleServlet, props);

  std::cout << "****** Registering WebConsoleServlet at /console" << std::endl;

  m_SettingsPlugin.Register();
  m_ServicesPlugin.Register();

//  server->addHandler("/Console/modules/", new ModulesHtml(context));
//  server->addHandler("/Console/resources/", new ResourcesHtml(context));
//  server->addHandler("/Console/", new ConsoleHtmlHandler(context));
//  server->addHandler("/", new DefaultHandler(context));
}

void WebConsoleActivator::Unload(us::ModuleContext* /*context*/)
{
}

}

US_EXPORT_MODULE_ACTIVATOR(us::WebConsoleActivator)
