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

#ifndef CPPMICROSERVICES_WEBCONSOLESERVLET_H
#define CPPMICROSERVICES_WEBCONSOLESERVLET_H

#include "cppmicroservices/httpservice/HttpServlet.h"

#include "cppmicroservices/ServiceTracker.h"

namespace cppmicroservices {

class HttpServletRequest;
class HttpServletResponse;
class AbstractWebConsolePlugin;

class WebConsolePluginTracker : public ServiceTracker<HttpServlet>
{

public:

  WebConsolePluginTracker();

  void Open(const std::shared_ptr<ServletContext>& servlet);

  AbstractWebConsolePlugin* GetPlugin(const std::string& label) const;

  std::string GetLabelMapJSON() const;

private:

  typedef ServiceTracker<HttpServlet> Superclass;

  void Open();

  std::shared_ptr<HttpServlet> AddingService(const ServiceReference<HttpServlet>& reference);

  void AddPlugin(const std::string& label, AbstractWebConsolePlugin* plugin);

  std::string GetProperty(const ServiceReference<HttpServlet>& reference, const std::string& property) const;

  typedef std::map<std::string, AbstractWebConsolePlugin*> PluginMapType;
  PluginMapType m_Plugins;
  Any m_LabelMapAny;
  std::vector<Any>* m_Labels;

  std::shared_ptr<ServletContext> m_ServletContext;
};

class WebConsoleServlet : public HttpServlet
{

public:
  WebConsoleServlet();

  void Init(const ServletConfig &config);

private:

  void Service(HttpServletRequest& request, HttpServletResponse& response);

  AbstractWebConsolePlugin* GetConsolePlugin(const std::string& label) const;

  WebConsolePluginTracker m_PluginTracker;
};

}

#endif // CPPMICROSERVICES_WEBCONSOLESERVLET_H
