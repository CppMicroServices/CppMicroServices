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

#include "cppmicroservices/GetBundleContext.h"

#include "ServletContainerPrivate.h"
#include "cppmicroservices/httpservice/ServletContainer.h"

#include "HttpServletPrivate.h"
#include "HttpServletRequestPrivate.h"
#include "HttpServletResponsePrivate.h"
#include "ServletConfigPrivate.h"
#include "cppmicroservices/httpservice/HttpServlet.h"
#include "cppmicroservices/httpservice/HttpServletRequest.h"
#include "cppmicroservices/httpservice/HttpServletResponse.h"
#include "cppmicroservices/httpservice/ServletContext.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"

#include "civetweb/CivetServer.h"

#include <cassert>
#include <utility>
#include <memory>

namespace cppmicroservices {

using Lock = std::unique_lock<std::mutex>;

class ServletHandler : public CivetHandler
{
public:
  ServletHandler(std::shared_ptr<HttpServlet>  servlet,
                 std::string  servletPath)
    : m_Servlet(std::move(servlet))
    , m_ServletPath(std::move(servletPath))
  {}

  std::shared_ptr<ServletContext> GetServletContext() const
  {
    return m_Servlet->GetServletContext();
  }

  std::shared_ptr<HttpServlet> GetServlet() const { return m_Servlet; }

private:
  bool handleGet(CivetServer* server, mg_connection* conn) override
  {
    auto mg_req_info = mg_get_request_info(conn);
    if (mg_req_info->local_uri == nullptr) {
      return true;
    }

    HttpServletRequest request(new HttpServletRequestPrivate(
      m_Servlet->GetServletContext(), server, conn));
    request.d->m_ContextPath = m_Servlet->GetServletContext()->GetContextPath();
    request.d->m_ServletPath = m_ServletPath;

    std::string uri = mg_req_info->local_uri;
    std::string pathPrefix =
      request.d->m_ContextPath + request.d->m_ServletPath;
    //std::cout << "Checking path prefix: " << pathPrefix << std::endl;
    //std::cout << "Against uri: " << uri << std::endl;
    assert(pathPrefix.size() <= uri.size());
    assert(uri.compare(0, pathPrefix.size(), pathPrefix) == 0);
    if (uri.size() > pathPrefix.size()) {
      request.d->m_PathInfo = uri.substr(pathPrefix.size());
    }

    HttpServletResponse response(
      new HttpServletResponsePrivate(&request, server, conn));
    response.SetStatus(HttpServletResponse::SC_OK);

    try {
      m_Servlet->Service(request, response);
    } catch (const std::exception& e) {
      std::cout << e.what() << std::endl;
      return false;
    }
    return true;
  }

  //virtual bool handlePost(CivetServer *server, struct mg_connection *conn);

  //virtual bool handlePut(CivetServer *server, struct mg_connection *conn);

  //virtual bool handleDelete(CivetServer *server, struct mg_connection *conn);

private:
  std::shared_ptr<HttpServlet> m_Servlet;
  std::string m_ServletPath;
};

//-------------------------------------------------------------------
//-----------        ServletContainerPrivate       ------------------
//-------------------------------------------------------------------

class ServletConfigImpl : public ServletConfig
{
public:
  ServletConfigImpl(const std::shared_ptr<ServletContext>& context)
  {
    ServletConfig::SetServletContext(context);
  }
};

ServletContainerPrivate::ServletContainerPrivate(BundleContext bundleCtx,
                                                 ServletContainer* q)
  : m_Context(std::move(bundleCtx))
  , m_Server(nullptr)
  , m_ServletTracker(m_Context, this)
  , q(q)
{}

void ServletContainerPrivate::Start()
{
  int port = 0;
  int sslPort = 0;
  {
    Lock l(m_Mutex);
    US_UNUSED(l);
    if (m_Server)
      return;

    m_Server = std::make_unique<CivetServer>(nullptr);
    const mg_context* serverContext = m_Server->getContext();
    if (serverContext == nullptr) {
      std::cout << "Servlet Container could not be started." << std::endl;
      m_Server.reset();
      return;
    }
    mg_get_ports(serverContext, 1, &port, &sslPort);
  }

  std::cout << "Servlet Container listening on http://localhost:" << port
            << std::endl;
  m_ServletTracker.Open();
}

void ServletContainerPrivate::Stop()
{
  m_ServletTracker.Close();

  std::unique_ptr<CivetServer> server;
  std::list<std::shared_ptr<ServletHandler>> handler;
  {
    Lock l(m_Mutex);
    US_UNUSED(l);
    server = std::move(m_Server);
    handler = std::move(m_Handler);
  }

  if (server) {
    for (auto& h : handler) {
      server->removeHandler(h->GetServletContext()->GetContextPath());
    }
  }
}

std::string ServletContainerPrivate::GetMimeType(
  const ServletContext* /*context*/,
  const std::string& file) const
{
  const char* mimeType = mg_get_builtin_mime_type(file.c_str());
  if (mimeType == nullptr)
    return std::string();
  return mimeType;
}

std::shared_ptr<ServletHandler> ServletContainerPrivate::AddingService(
  const ServiceReference<HttpServlet>& reference)
{
  Any contextRoot = reference.GetProperty(HttpServlet::PROP_CONTEXT_ROOT);
  if (contextRoot.Empty()) {
    std::cout << "HttpServlet from " << reference.GetBundle().GetSymbolicName()
              << " is missing the context root property." << std::endl;
    return nullptr;
  }

  auto servlet = m_Context.GetService(reference);
  if (!servlet) {
    std::cout << "HttpServlet from " << reference.GetBundle().GetSymbolicName()
              << " is nullptr." << std::endl;
    return nullptr;
  }
  std::shared_ptr<ServletContext> servletContext(new ServletContext(q));
  servlet->Init(ServletConfigImpl(servletContext));
  auto handler =
    std::make_shared<ServletHandler>(servlet, contextRoot.ToString());

  std::string ctxPath;
  {
    Lock l(m_Mutex);
    US_UNUSED(l);
    m_Handler.push_back(handler);
    m_ServletContextMap[contextRoot.ToString()] = servletContext;
    ctxPath = m_ContextPath + contextRoot.ToString();
  }

  m_Server->addHandler(ctxPath, handler.get());
  return handler;
}

void ServletContainerPrivate::ModifiedService(
  const ServiceReference<HttpServlet>& /*reference*/,
  const std::shared_ptr<ServletHandler>& /*service*/)
{
  // no-op
}

void ServletContainerPrivate::RemovedService(
  const ServiceReference<HttpServlet>& /*reference*/,
  const std::shared_ptr<ServletHandler>& handler)
{
  std::string contextPath = handler->GetServletContext()->GetContextPath();

  Lock l(m_Mutex);
  US_UNUSED(l);
  m_Server->removeHandler(contextPath);
  m_Handler.remove(handler);
  handler->GetServlet()->Destroy();
  m_ServletContextMap.erase(contextPath);
}

//-------------------------------------------------------------------
//-----------            ServletContainer          ------------------
//-------------------------------------------------------------------

ServletContainer::ServletContainer(BundleContext bundleCtx,
                                   const std::string& contextPath)
  : d(new ServletContainerPrivate(std::move(bundleCtx), this))
{
  this->SetContextPath(contextPath);
}

ServletContainer::~ServletContainer()
{
  this->Stop();
  delete d;
}

void ServletContainer::SetContextPath(const std::string& path)
{
  Lock l(d->m_Mutex);
  US_UNUSED(l);
  if (d->m_Server) {
    return;
  }
  std::string contextPath = path;
  std::size_t pos = path.find_last_of('/');
  if (pos != std::string::npos) {
    contextPath = contextPath.substr(0, pos);
  }
  if (!contextPath.empty() && contextPath[0] != '/') {
    contextPath = "/" + contextPath;
  }
  d->m_ContextPath = contextPath;
}

std::string ServletContainer::GetContextPath() const
{
  return Lock(d->m_Mutex), d->m_ContextPath;
}

void ServletContainer::Start()
{
  d->Start();
}

void ServletContainer::Stop()
{
  d->Stop();
}

std::shared_ptr<ServletContext> ServletContainer::GetContext(
  const std::string& uripath) const
{
  Lock l(d->m_Mutex);
  US_UNUSED(l);
  auto iter = d->m_ServletContextMap.find(uripath);
  if (iter == d->m_ServletContextMap.end())
    return nullptr;
  return iter->second;
}

std::string ServletContainer::GetContextPath(
  const ServletContext* /*context*/) const
{
  return Lock(d->m_Mutex), d->m_ContextPath;
}
}
