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

#ifndef CPPMICROSERVICES_SERVLETCONTAINER_H
#define CPPMICROSERVICES_SERVLETCONTAINER_H

#include "cppmicroservices/httpservice/HttpServiceExport.h"

#include <memory>
#include <string>

namespace cppmicroservices {

struct ServletContainerPrivate;
class ServletContext;
class BundleContext;

class US_HttpService_EXPORT ServletContainer
{
public:
  ServletContainer(BundleContext bundleCtx,
                   const std::string& contextPath = std::string());
  ~ServletContainer();

  void SetContextPath(const std::string& contextPath);
  std::string GetContextPath() const;

  void Start();
  void Stop();

  std::shared_ptr<ServletContext> GetContext(const std::string& uripath) const;
  std::string GetContextPath(const ServletContext* context) const;

private:
  friend class ServletContext;

  ServletContainerPrivate* d;
};
}

#endif // CPPMICROSERVICES_SERVLETCONTAINER_H
