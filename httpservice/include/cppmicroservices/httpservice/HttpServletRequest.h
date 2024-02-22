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

#ifndef CPPMICROSERVICES_HTTPSERVLETREQUEST_H
#define CPPMICROSERVICES_HTTPSERVLETREQUEST_H

#include "cppmicroservices/Any.h"
#include "cppmicroservices/httpservice/HttpServiceExport.h"

#include <string>
#include <vector>

namespace cppmicroservices
{

    class ServletContext;
    struct HttpServletRequestPrivate;

    class US_HttpService_EXPORT HttpServletRequest
    {
      public:
        ~HttpServletRequest();

        HttpServletRequest(HttpServletRequest const& o);
        HttpServletRequest& operator=(HttpServletRequest const& o);

        std::shared_ptr<ServletContext> GetServletContext() const;

        Any GetAttribute(std::string const& name) const;

        std::vector<std::string> GetAttributeNames() const;

        std::size_t GetContentLength() const;

        std::string GetContentType() const;

        std::string GetLocalName() const;

        std::string GetRemoteHost() const;

        int GetLocalPort() const;

        int GetRemotePort() const;

        std::string GetScheme() const;

        std::string GetServerName() const;

        int GetServerPort() const;

        std::string GetProtocol() const;

        std::string GetContextPath() const;

        std::string GetPathInfo() const;

        std::string GetRequestUri() const;

        std::string GetRequestUrl() const;

        std::string GetServletPath() const;

        std::string GetQueryString() const;

        std::string GetHeader(std::string const& name) const;

        long long GetDateHeader(std::string const& name) const;

        std::vector<std::string> GetHeaderNames() const;

        std::vector<std::string> GetHeaders(std::string const& name) const;

        std::string GetMethod() const;

        std::vector<std::pair<std::string, float>> GetAcceptHeader() const;

        void RemoveAttribute(std::string const& name);

        void SetAttribute(std::string const& name, Any const& value);

      private:
        friend class ServletHandler;
        HttpServletRequest(HttpServletRequestPrivate* d);

        std::shared_ptr<HttpServletRequestPrivate> d;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_HTTPREQUEST_H
