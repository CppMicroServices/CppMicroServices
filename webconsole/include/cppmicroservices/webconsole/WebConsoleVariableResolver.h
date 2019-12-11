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

#ifndef CPPMICROSERVICES_WEBCONSOLEVARIABLERESOLVER_H
#define CPPMICROSERVICES_WEBCONSOLEVARIABLERESOLVER_H

#include <string>

#include "cppmicroservices/GlobalConfig.h"

#include "cppmicroservices/webconsole/WebConsoleExport.h"

namespace cppmicroservices {

/**
 * The <code>WebConsoleVariableResolver</code> interface defines the API for an object
 * which may be provided by plugins to provide replacement values for
 * variables in the generated content.
 *
 * Plugins should call the AbstractWebConsolePlugin#SetVariableResolver
 * method to provide their implementation for variable resolution.
 *
 * The main use of such a variable resolver is when a plugin is using a static
 * template which provides slots to place dynamically generated content
 * parts.
 *
 * \rststar
 * .. note::
 *
 *    The variable resolver must be set in the request *before*
 *    the response stream is retrieved calling the
 *    :any:`HttpServletResponse::GetOutputStream <cppmicroservices::HttpServletResponse::GetOutputStream>`
 *    method. Otherwise the variable resolver will not be used for
 *    resolving variables.
 * \endrststar
 *
 * @see AbstractWebConsolePlugin#GetVariableResolver(HttpServletRequest&)
 * @see AbstractWebConsolePlugin#SetVariableResolver
 */
struct US_WebConsole_EXPORT WebConsoleVariableResolver
{

  virtual ~WebConsoleVariableResolver();

  /**
   * Returns a replacement value for the named variable.
   *
   * @param variable The name of the variable for which to return a
   *      replacement.
   * @return The replacement value.
   */
  virtual std::string Resolve(const std::string& variable) const = 0;
};
}

#endif // CPPMICROSERVICES_WEBCONSOLEVARIABLERESOLVER_H
