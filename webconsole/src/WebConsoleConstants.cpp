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

#include "cppmicroservices/webconsole/WebConsoleConstants.h"

#include "cppmicroservices/httpservice/HttpServlet.h"

namespace cppmicroservices {

std::string WebConsoleConstants::SERVICE_NAME =
  us_service_interface_iid<HttpServlet>();
std::string WebConsoleConstants::PLUGIN_LABEL =
  "org.cppmicroservices.webconsole.label";
std::string WebConsoleConstants::PLUGIN_TITLE =
  "org.cppmicroservices.webconsole.title";
std::string WebConsoleConstants::PLUGIN_CATEGORY =
  "org.cppmicroservices.webconsole.category";
std::string WebConsoleConstants::PLUGIN_CSS_REFERENCES =
  "org.cppmicroservices.webconsole.css";
std::string WebConsoleConstants::ATTR_APP_ROOT =
  "org.cppmicroservices.webconsole.appRoot";
std::string WebConsoleConstants::ATTR_PLUGIN_ROOT =
  "org.cppmicroservices.webconsole.pluginRoot";
std::string WebConsoleConstants::ATTR_LABEL_MAP =
  "org.cppmicroservices.webconsole.labelMap";
std::string WebConsoleConstants::ATTR_CONSOLE_VARIABLE_RESOLVER =
  "org.cppmicroservices.webconsole.variable.resolver";
}
