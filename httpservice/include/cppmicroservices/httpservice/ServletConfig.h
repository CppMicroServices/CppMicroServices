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

#ifndef CPPMICROSERVICES_SERVLETCONFIG_H
#define CPPMICROSERVICES_SERVLETCONFIG_H

#include "cppmicroservices/SharedData.h"
#include "cppmicroservices/httpservice/HttpServiceExport.h"

#include <memory>
#include <string>

namespace cppmicroservices {

class ServletContext;
struct ServletConfigPrivate;

/**
 *
 * A servlet configuration object used by a servlet container
 * to pass information to a servlet during initialization.
 *
 */
class US_HttpService_EXPORT ServletConfig
{

public:
  ServletConfig();
  ServletConfig(const ServletConfig& other);
  ServletConfig& operator=(const ServletConfig& other);

  virtual ~ServletConfig();

  /**
   * Returns the name of this servlet instance.
   * The name may be provided via server administration, assigned in the
   * web application deployment descriptor, or for an unregistered (and thus
   * unnamed) servlet instance it will be the servlet's class name.
   *
   * @return the name of the servlet instance
   */
  std::string GetServletName() const;

  /**
   * Returns a reference to the {@link ServletContext} in which the caller
   * is executing.
   *
   *
   * @return  a {@link ServletContext} object, used
   *   by the caller to interact with its servlet
   *                  container
   *
   * @see  ServletContext
   *
   */
  std::shared_ptr<ServletContext> GetServletContext() const;

protected:
  void SetServletName(const std::string& name);
  void SetServletContext(const std::shared_ptr<ServletContext>& context);

private:
  ExplicitlySharedDataPointer<ServletConfigPrivate> d;
};
}

#endif // SERVLETCONFIG_H
