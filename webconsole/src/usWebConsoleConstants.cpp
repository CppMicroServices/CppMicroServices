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

#include "usWebConsoleConstants.h"

#include "usHttpServlet.h"

namespace us {

std::string WebConsoleConstants::SERVICE_NAME()
{
  return us_service_interface_iid<HttpServlet>();
}

std::string WebConsoleConstants::PLUGIN_LABEL()
{
  static std::string s = "org.cppmicroservices.webconsole.label";
  return s;
}

std::string WebConsoleConstants::PLUGIN_TITLE()
{
  static std::string s = "org.cppmicroservices.webconsole.title";
  return s;
}

std::string WebConsoleConstants::PLUGIN_CATEGORY()
{
  static std::string s = "org.cppmicroservices.webconsole.category";
  return s;
}

std::string WebConsoleConstants::PLUGIN_CSS_REFERENCES()
{
  static std::string s = "org.cppmicroservices.webconsole.css";
  return s;
}

std::string WebConsoleConstants::ATTR_APP_ROOT()
{
  static std::string s = "org.cppmicroservices.webconsole.appRoot";
  return s;
}

std::string WebConsoleConstants::ATTR_PLUGIN_ROOT()
{
  static std::string s = "org.cppmicroservices.webconsole.pluginRoot";
  return s;
}

std::string WebConsoleConstants::ATTR_LABEL_MAP()
{
  static std::string s = "org.cppmicroservices.webconsole.labelMap";
  return s;
}

std::string WebConsoleConstants::ATTR_CONSOLE_VARIABLE_RESOLVER()
{
  static std::string s = "org.cppmicroservices.webconsole.variable.resolver";
  return s;
}

std::string WebConsoleConstants::ATTR_LANG_MAP()
{
  static std::string s = "org.cppmicroservices.webconsole.langMap";
  return s;
}

}
