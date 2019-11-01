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

#ifndef CPPMICROSERVICES_IHTTPSERVLETREQUEST_H
#define CPPMICROSERVICES_IHTTPSERVLETREQUEST_H

#include "cppmicroservices/Any.h"
#include "cppmicroservices/SharedData.h"
#include "cppmicroservices/httpservice/HttpServiceExport.h"
#include "cppmicroservices/httpservice/IHttpServletPart.h"

#include <string>
#include <vector>

namespace cppmicroservices {

class ServletContext;
class HttpServletPart;

class US_HttpService_EXPORT IHttpServletRequest
{
public:
  virtual ~IHttpServletRequest() = default;

  virtual std::shared_ptr<ServletContext> GetServletContext() const = 0;

  virtual Any GetAttribute(const std::string& name) const = 0;

  virtual std::vector<std::string> GetAttributeNames() const = 0;

  virtual std::size_t GetContentLength() const = 0;

  virtual std::string GetContentType() const = 0;

  //virtual std::string GetLocalName() const = 0;

  //virtual std::string GetRemoteHost() const = 0;

  //virtual int GetLocalPort() const = 0;

  //virtual int GetRemotePort() const = 0;

  virtual std::string GetScheme() const = 0;

  virtual std::string GetServerName() const = 0;

  virtual int GetServerPort() const = 0;

  virtual std::string GetProtocol() const = 0;

  virtual std::string GetContextPath() const = 0;

  virtual std::string GetPathInfo() const = 0;

  virtual std::string GetRequestUri() const = 0;

  virtual std::string GetRequestUrl() const = 0;

  virtual std::string GetServletPath() const = 0;

  virtual std::string GetQueryString() const = 0;

  virtual std::string GetHeader(const std::string& name) const = 0;

  virtual long long GetDateHeader(const std::string& name) const = 0;

  virtual std::vector<std::string> GetHeaderNames() const = 0;

  virtual std::vector<std::string> GetHeaders(
    const std::string& name) const = 0;

  virtual std::string GetMethod() const = 0;

  //virtual std::vector<std::pair<std::string, float>> GetAcceptHeader() const = 0;

  virtual void RemoveAttribute(const std::string& name) = 0;

  virtual void SetAttribute(const std::string& name, const Any& value) = 0;

  /*!
   * Return the value of asked query parameter or empty Any if the parameter is
   * not found
   */
  virtual Any GetParameter(const std::string& name) const = 0;

  /*!
   *
   * Gets all the Part components of this request, provided that it is of type
   * multipart/form-data.
   */
  virtual std::vector<IHttpServletPart*> GetParts() const = 0;

  /*!
  * Get the body of the request, if any
  */
  virtual std::string GetBody() const = 0;

  /*!
   * Gets the Part with the given name.
   */
  virtual IHttpServletPart* GetPart(const std::string& name) const = 0;

  /*!
   * Triggers the load of files transmitted by HTTP POST, multipart/form-data
   * encoded
   *
   * An optional callback can be used to perform special operation on the
   * ServletPart
   */
  virtual void ReadParts(void* callback(IHttpServletPart*) = 0) = 0;

private:
  friend class HttpServlet;
  virtual void* RawData() = 0;
};
}

#endif // CPPMICROSERVICES_IHTTPREQUEST_H
