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

#include "usServletContainer.h"
#include "usServletContainer_p.h"

#include "usHttpServlet.h"
#include "usHttpServlet_p.h"
#include "usHttpServletRequest.h"
#include "usHttpServletRequest_p.h"
#include "usHttpServletResponse.h"
#include "usHttpServletResponse_p.h"
#include "usServletContext.h"
#include "usServletConfig_p.h"

#include "usGetBundleContext.h"
#include "usBundleContext.h"
#include "usBundle.h"

#include "civetweb/CivetServer.h"

#include <cassert>

namespace us {

class ServletHandler : public CivetHandler
{
public:

  ServletHandler(HttpServlet* servlet, const std::string& servletPath)
    : m_Servlet(servlet)
    , m_ServletPath(servletPath)
  {}

  ServletContext* GetServletContext() const
  {
    return m_Servlet->GetServletContext();
  }

  HttpServlet* GetServlet() const
  {
    return m_Servlet;
  }

private:

  virtual bool handleGet(CivetServer* server, mg_connection* conn)
  {
    HttpServletRequest request(new HttpServletRequestPrivate(m_Servlet->GetServletContext(), server, conn));
    request.d->m_ContextPath = m_Servlet->GetServletContext()->GetContextPath();
    request.d->m_ServletPath = m_ServletPath;

    std::string uri = mg_get_request_info(conn)->uri;
    std::size_t pos = uri.find_first_of('?');
    uri = uri.substr(0, pos);
    std::string pathPrefix = request.d->m_ContextPath + request.d->m_ServletPath;
                                std::cout << "Checking path prefix: " << pathPrefix << std::endl;
                                std::cout << "Against uri: " << uri << std::endl;
    assert(pathPrefix.size() <= uri.size());
    assert(uri.compare(0, pathPrefix.size(), pathPrefix) == 0);
    if(uri.size() > pathPrefix.size())
    {
      request.d->m_PathInfo = uri.substr(pathPrefix.size());
    }

    HttpServletResponse response(new HttpServletResponsePrivate(&request, server, conn));
    response.SetStatus(HttpServletResponse::SC_OK);

    try
    {
      m_Servlet->Service(request, response);
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << std::endl;
      return false;
    }
    return true;
  }

  //virtual bool handlePost(CivetServer *server, struct mg_connection *conn);

  //virtual bool handlePut(CivetServer *server, struct mg_connection *conn);

  //virtual bool handleDelete(CivetServer *server, struct mg_connection *conn);

private:

  HttpServlet* m_Servlet;
  std::string m_ServletPath;
};

//-------------------------------------------------------------------
//-----------        ServletContainerPrivate       ------------------
//-------------------------------------------------------------------

class ServletConfigImpl : public ServletConfig
{
public:
  ServletConfigImpl(ServletContext* context)
  {
    ServletConfig::SetServletContext(context);
  }
};

ServletContainerPrivate::ServletContainerPrivate(ServletContainer* q)
    : m_Context(GetBundleContext())
    , m_Server(NULL)
    , m_ServletTracker(m_Context, this)
    , q(q)
  {}

void ServletContainerPrivate::Start()
{
  if (m_Server) return;

  m_Server = new CivetServer(NULL);
  const mg_context* serverContext = m_Server->getContext();
  if (serverContext == NULL)
  {
    std::cout << "Servlet Container could not be started." << std::endl;
    delete m_Server;
    m_Server = NULL;
    return;
  }
  int port, sslPort;
  mg_get_ports(serverContext, 1, &port, &sslPort);
  std::cout << "Servlet Container listening on http://localhost:" << port << std::endl;
  m_ServletTracker.Open();
}

void ServletContainerPrivate::Stop()
{
  m_ServletTracker.Close();
  if (m_Server)
  {
    for (std::list<ServletHandler*>::iterator iter = m_Handler.begin(),
         endIter = m_Handler.end(); iter != endIter; ++iter)
    {
      m_Server->removeHandler((*iter)->GetServletContext()->GetContextPath());
      delete *iter;
    }
  }
  m_Handler.clear();

  delete m_Server;
  m_Server = NULL;
}

std::string ServletContainerPrivate::GetMimeType(const ServletContext* /*context*/, const std::string& file) const
{
  const char* mimeType = mg_get_builtin_mime_type(file.c_str());
  if (mimeType == NULL) return std::string();
  return mimeType;
}

ServletContainerPrivate::TrackedType ServletContainerPrivate::AddingService(const ServiceReferenceType& reference)
{
  Any contextRoot = reference.GetProperty(HttpServlet::PROP_CONTEXT_ROOT());
  if (contextRoot.Empty())
  {
    std::cout << "HttpServlet from " << reference.GetBundle()->GetName() << " is missing the context root property." << std::endl;
    return NULL;
  }

  HttpServlet* servlet = m_Context->GetService(reference);
  if (servlet == NULL)
  {
    std::cout << "HttpServlet from " << reference.GetBundle()->GetName() << " is NULL." << std::endl;
    return NULL;
  }
  ServletContext* servletContext = new ServletContext(q);
  servlet->Init(ServletConfigImpl(servletContext));
  ServletHandler* handler = new ServletHandler(servlet, contextRoot.ToString());
  m_Handler.push_back(handler);
  m_ServletContextMap[contextRoot.ToString()] = servletContext;

  m_Server->addHandler(m_ContextPath + contextRoot.ToString(), handler);
  return handler;
}

void ServletContainerPrivate::ModifiedService(const ServiceReferenceType& /*reference*/, TrackedType /*service*/)
{
  // no-op
}

void ServletContainerPrivate::RemovedService(const ServiceReferenceType& reference, TrackedType handler)
{
  std::string contextPath = handler->GetServletContext()->GetContextPath();
  m_Server->removeHandler(contextPath);
  m_Handler.remove(handler);
  handler->GetServlet()->Destroy();
  m_ServletContextMap.erase(contextPath);

  delete handler->GetServletContext();
  delete handler;

  m_Context->UngetService(reference);
}


//-------------------------------------------------------------------
//-----------            ServletContainer          ------------------
//-------------------------------------------------------------------

ServletContainer::ServletContainer(const std::string& contextPath)
  : d(new ServletContainerPrivate(this))
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
  if (d->m_Server)
  {
    return;
  }
  std::string contextPath = path;
  std::size_t pos = path.find_last_of('/');
  if (pos != std::string::npos)
  {
    contextPath = contextPath.substr(0, pos);
  }
  if (!contextPath.empty() && contextPath[0] != '/')
  {
    contextPath = "/" + contextPath;
  }
  d->m_ContextPath = contextPath;
}

std::string ServletContainer::GetContextPath() const
{
  return d->m_ContextPath;
}

void ServletContainer::Start()
{
  d->Start();
}

void ServletContainer::Stop()
{
  d->Stop();
}

ServletContext* ServletContainer::GetContext(const std::string& uripath) const
{
  std::map<std::string, ServletContext*>::iterator iter = d->m_ServletContextMap.find(uripath);
  if (iter == d->m_ServletContextMap.end()) return NULL;
  return iter->second;
}

std::string ServletContainer::GetContextPath(const ServletContext* /*context*/) const
{
  return d->m_ContextPath;
}

}
