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

#ifndef CPPMICROSERVICES_WEBCONSOLEVARIABLERESOLVER_H
#define CPPMICROSERVICES_WEBCONSOLEVARIABLERESOLVER_H

#include <string>

#include "cppmicroservices/GlobalConfig.h"

namespace cppmicroservices {

/**
 * The <code>VariableResolver</code> interface defines the API for an object
 * which may be provided by plugins to provide replacement values for
 * variables in the generated content.
 * <p>
 * Plugins should call the
 * {@link WebConsoleUtil#setVariableResolver(javax.servlet.ServletRequest, VariableResolver)}
 * method to provide their implementation for variable resolution.
 * <p>
 * The main use of such a variable resolver is when a plugin is using a static
 * template which provides slots to place dynamically generated content
 * parts.
 * <p>
 * <b>Note</b>: The variable resolver must be set in the request <b>before</b>
 * the response writer is retrieved calling the
 * <code>ServletRequest.getWriter()</code> method. Otherwise the variable
 * resolver will not be used for resolving variables.
 *
 * @see WebConsoleUtil#getVariableResolver(javax.servlet.ServletRequest)
 * @see WebConsoleUtil#setVariableResolver(javax.servlet.ServletRequest, VariableResolver)
 */
struct WebConsoleVariableResolver
{

  virtual ~WebConsoleVariableResolver();

  /**
   * Returns a replacement value for the named variable or <code>null</code>
   * if no replacement is available.
   *
   * @param variable The name of the variable for which to return a
   *      replacement.
   * @return The replacement value or <code>null</code> if no replacement is
   *      available.
   */
  virtual std::string Resolve(const std::string& variable) const = 0;
};

}

#endif // CPPMICROSERVICES_WEBCONSOLEVARIABLERESOLVER_H
