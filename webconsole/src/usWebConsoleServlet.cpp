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

#include "usWebConsoleServlet_p.h"

#include "usWebConsoleConstants.h"
#include "usAbstractWebConsolePlugin.h"
#include "usWebConsoleVariableResolver.h"
#include "usVariableResolverStreamBuffer_p.h"

#include "usHttpServletRequest.h"
#include "usHttpServletResponse.h"
#include "usServletConfig.h"

#include <iostream>

US_BEGIN_NAMESPACE

class FilteringResponseWrapper : public HttpServletResponse
{

public:

  FilteringResponseWrapper(HttpServletRequest& request, HttpServletResponse& response, AbstractWebConsolePlugin* plugin)
   : HttpServletResponse(response)
   , m_Stream(NULL)
   , m_StreamBuf(NULL)
   , m_Plugin(plugin)
   , m_Request(request)
  {}

  ~FilteringResponseWrapper()
  {
  }

private:

  virtual std::streambuf* GetOutputStreamBuffer()
  {
    std::string contentType = this->GetContentType();
    if (contentType.size() >= 9 && contentType.compare(0, 9, "text/html") == 0)
    {
      if (m_StreamBuf == NULL)
      {
        WebConsoleVariableResolver* resolver = m_Plugin->GetVariableResolver(m_Request);
        m_StreamBuf = new VariableResolverStreamBuffer(
                        new std::ostream(HttpServletResponse::GetOutputStreamBuffer()),
                        resolver);
      }
      return m_StreamBuf;
    }
    else
    {
      return HttpServletResponse::GetOutputStreamBuffer();
    }
  }

  std::ostream* m_Stream;
  std::streambuf* m_StreamBuf;
  AbstractWebConsolePlugin* m_Plugin;
  HttpServletRequest& m_Request;
};

void WebConsolePluginTracker::AddPlugin(const std::string& label, AbstractWebConsolePlugin* plugin)
{
  std::cout << "Adding console plugin with label " << label << std::endl;

  struct PluginConfig : public ServletConfig
  {
    PluginConfig(ServletContext* context)
    {
      ServletConfig::SetServletContext(context);
    }
  };

  plugin->Init(PluginConfig(m_ServletContext));
  m_Plugins[label] = plugin;

  std::map<std::string, Any> menuItem;
  menuItem["label"] = label;
  menuItem["href"] = label;
  menuItem["current"] = false;

  m_Labels->push_back(menuItem);
}

WebConsolePluginTracker::WebConsolePluginTracker()
  : ServiceTracker<HttpServlet>(GetModuleContext())
  , m_LabelMapAny(std::map<std::string, Any>())
  , m_Labels(NULL)
  , m_ServletContext(NULL)
{
  ref_any_cast<std::map<std::string, Any> >(m_LabelMapAny)["menuItems"] = std::vector<Any>();
  m_Labels = &ref_any_cast<std::vector<Any> >(ref_any_cast<std::map<std::string, Any> >(m_LabelMapAny)["menuItems"]);
}

void WebConsolePluginTracker::Open(ServletContext* context)
{
  this->m_ServletContext = context;
  Superclass::Open();
}

AbstractWebConsolePlugin* WebConsolePluginTracker::GetPlugin(const std::string& label) const
{
  PluginMapType::const_iterator iter = m_Plugins.find(label);
  if (iter == m_Plugins.end()) return NULL;
  return iter->second;
}

std::string WebConsolePluginTracker::GetLabelMapJSON() const
{
  return m_LabelMapAny.ToJSON();
}

void WebConsolePluginTracker::Open()
{
  Superclass::Open();
}

HttpServlet* WebConsolePluginTracker::AddingService(const ServiceReferenceType& reference)
{
  std::string label = this->GetProperty(reference, WebConsoleConstants::PLUGIN_LABEL());
  if (label.empty())
  {
    return NULL;
  }
  HttpServlet* servlet = Superclass::AddingService(reference);
  AbstractWebConsolePlugin* plugin = dynamic_cast<AbstractWebConsolePlugin*>(servlet);
  if (plugin)
  {
    this->AddPlugin(label, plugin);
  }
  return plugin;
}

std::string WebConsolePluginTracker::GetProperty(const ServiceTracker::ServiceReferenceType& reference, const std::string& property) const
{
  us::Any labelProp = reference.GetProperty(property);
  if (labelProp.Empty() || labelProp.Type() != typeid(std::string))
  {
    return std::string();
  }
  return labelProp.ToString();
}

WebConsoleServlet::WebConsoleServlet()
{
}

void WebConsoleServlet::Init(const ServletConfig& config)
{
  HttpServlet::Init(config);
  this->m_PluginTracker.Open(config.GetServletContext());
}

void WebConsoleServlet::Service(HttpServletRequest& request, HttpServletResponse& response)
{
  std::cout << "RequestUrl: " << request.GetRequestUrl() << std::endl;
  std::cout << "RequestUri: " << request.GetRequestUri() << std::endl;
  std::cout << "ContextPath: " << request.GetContextPath() << std::endl;
  std::cout << "PathInfo: " << request.GetPathInfo() << std::endl;
  std::cout << "ServletPath: " << request.GetServletPath() << std::endl;
  //std::cout << "LocalName: " << request.GetLocalName() << std::endl;
  //std::cout << "LocalPort: " << request.GetLocalPort() << std::endl;
  //std::cout << "Protocol: " << request.GetProtocol() << std::endl;
  std::cout << "Scheme: " << request.GetScheme() << std::endl;
  //std::cout << "RemoteAddr: " << request.GetRemoteAddr() << std::endl;
  //std::cout << "RemoteHost: " << request.GetRemoteHost() << std::endl;
  //std::cout << "RemotePort: " << request.GetRemotePort() << std::endl;
  std::cout << "ServerName: " << request.GetServerName() << std::endl;
  std::cout << "ServerPort: " << request.GetServerPort() << std::endl;

  // check whether we are not at .../{webManagerRoot}
  std::string pathInfo = request.GetPathInfo();
  if (pathInfo.empty() || pathInfo == "/")
  {
    std::string path = request.GetServletPath();
    if (path[path.size()-1] != '/')
    {
      path += '/';
    }
    // path += holder.getDefaultPluginLabel();
    path += "settings";
    response.SendRedirect(path);
    return;
  }

  std::size_t slash = pathInfo.find_first_of('/', 1);
  if (slash < 2)
  {
    slash = std::string::npos;
  }

  std::string label = pathInfo.substr(1, slash != std::string::npos ? slash-1 : slash);

  // WORKAROUND until registration of resources is supported
  if (label == "res")
  {
    label = "settings";
  }

  AbstractWebConsolePlugin* plugin = GetConsolePlugin(label);
  if (plugin != NULL)
  {
    //final Map labelMap = holder.getLocalizedLabelMap( resourceBundleManager, locale, this.defaultCategory );
    //final Object flatLabelMap = labelMap.remove( WebConsoleConstants.ATTR_LABEL_MAP );

    // the official request attributes
    //request.SetAttribute(WebConsoleConstants::ATTR_LANG_MAP, getLangMap());
    request.SetAttribute(WebConsoleConstants::ATTR_LABEL_MAP(), m_PluginTracker.GetLabelMapJSON());
    //request.SetAttribute(WebConsoleConstants::ATTR_LABEL_MAP_CATEGORIZED, labelMap );
    request.SetAttribute(WebConsoleConstants::ATTR_APP_ROOT(),
                         request.GetContextPath() + request.GetServletPath());
    request.SetAttribute(WebConsoleConstants::ATTR_PLUGIN_ROOT(),
                         request.GetContextPath() + request.GetServletPath() + '/' + label);

    // wrap the response for localization and template variable replacement
    //request = wrapRequest(request, locale);
    FilteringResponseWrapper filteringResponse(request, response, plugin);

    plugin->Service(request, filteringResponse);
  }
  else
  {
    response.SetCharacterEncoding("utf-8"); //$NON-NLS-1$
    response.SetContentType("text/html"); //$NON-NLS-1$
    response.SetStatus(HttpServletResponse::SC_NOT_FOUND);
    response.GetOutputStream() << "No plug-in found handling this URL";
  }
}

AbstractWebConsolePlugin* WebConsoleServlet::GetConsolePlugin(const std::string& label) const
{
  return m_PluginTracker.GetPlugin(label);
}

US_END_NAMESPACE
