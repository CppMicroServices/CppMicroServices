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

#ifndef CPPMICROSERVICES_SERVLETCONTAINERPRIVATE_H
#define CPPMICROSERVICES_SERVLETCONTAINERPRIVATE_H

#include "cppmicroservices/ServiceTracker.h"
#include "cppmicroservices/ServiceTrackerCustomizer.h"

#include "cppmicroservices/httpservice/HttpServlet.h"

class CivetServer;

namespace cppmicroservices {

class BundleContext;

class ServletContainer;
class ServletContext;
class ServletHandler;

struct ServletContainerPrivate
  : private ServiceTrackerCustomizer<HttpServlet, ServletHandler>
{
  ServletContainerPrivate(BundleContext bundleCtx, ServletContainer* q);

  void Start();
  void Stop();

  std::string GetMimeType(const ServletContext* context,
                          const std::string& file) const;

  BundleContext m_Context;

  std::mutex m_Mutex;
  std::unique_ptr<CivetServer> m_Server;
  ServiceTracker<HttpServlet, ServletHandler> m_ServletTracker;

  std::map<std::string, std::shared_ptr<ServletContext>> m_ServletContextMap;
  std::string m_ContextPath;

private:
  ServletContainer* const q;
  std::list<std::shared_ptr<ServletHandler>> m_Handler;

  virtual std::shared_ptr<ServletHandler> AddingService(
    const ServiceReference<HttpServlet>& reference);
  virtual void ModifiedService(
    const ServiceReference<HttpServlet>& /*reference*/,
    const std::shared_ptr<ServletHandler>& /*service*/);
  virtual void RemovedService(const ServiceReference<HttpServlet>& reference,
                              const std::shared_ptr<ServletHandler>& handler);
};
}

#endif // CPPMICROSERVICES_SERVLETCONTAINERPRIVATE_H
