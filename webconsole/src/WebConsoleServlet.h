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

#ifndef CPPMICROSERVICES_WEBCONSOLESERVLET_H
#define CPPMICROSERVICES_WEBCONSOLESERVLET_H

#include "cppmicroservices/httpservice/HttpServlet.h"

#include "cppmicroservices/webconsole/AbstractWebConsolePlugin.h"

#include "cppmicroservices/ServiceTracker.h"

namespace cppmicroservices
{

    class HttpServletRequest;
    class HttpServletResponse;

    class WebConsolePluginTracker : public ServiceTracker<HttpServlet>
    {

      public:
        WebConsolePluginTracker();

        void Open(std::shared_ptr<ServletContext> const& servlet);

        AbstractWebConsolePlugin* GetPlugin(std::string const& label) const;

        AbstractWebConsolePlugin::TemplateData GetLabelMap(std::string const& current) const;

      private:
        using Superclass = ServiceTracker<HttpServlet>;

        struct LabelMapEntry
        {
            std::string label;
            std::string title;
        };

        void Open();

        std::shared_ptr<HttpServlet> AddingService(ServiceReference<HttpServlet> const& reference);

        void AddPlugin(std::string const& label, AbstractWebConsolePlugin* plugin);

        std::string GetProperty(ServiceReference<HttpServlet> const& reference, std::string const& property) const;

        using PluginMapType = std::map<std::string, AbstractWebConsolePlugin*>;
        PluginMapType m_Plugins;
        std::map<std::string, LabelMapEntry> m_LabelMap;

        std::shared_ptr<ServletContext> m_ServletContext;
    };

    class WebConsoleServlet : public HttpServlet
    {

      public:
        WebConsoleServlet();

        void Init(ServletConfig const& config);

      private:
        void Service(HttpServletRequest& request, HttpServletResponse& response);

        AbstractWebConsolePlugin* GetConsolePlugin(std::string const& label) const;

        WebConsolePluginTracker m_PluginTracker;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_WEBCONSOLESERVLET_H
