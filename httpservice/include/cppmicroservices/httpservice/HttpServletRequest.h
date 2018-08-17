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
#include "cppmicroservices/SharedData.h"
#include "cppmicroservices/httpservice/HttpServiceExport.h"

#include <string>
#include <vector>

namespace cppmicroservices {

class ServletContext;
struct HttpServletRequestPrivate;
struct HttpServletPartPrivate;
class HttpServletPart;

class US_HttpService_EXPORT HttpServletRequest
{
public:
  virtual ~HttpServletRequest();

  HttpServletRequest(const HttpServletRequest& o);
  HttpServletRequest& operator=(const HttpServletRequest& o);

  virtual std::shared_ptr<ServletContext> GetServletContext() const;

  virtual Any GetAttribute(const std::string& name) const;

  virtual std::vector<std::string> GetAttributeNames() const;

  virtual std::size_t GetContentLength() const;

  virtual std::string GetContentType() const;

  //virtual std::string GetLocalName() const;

  //virtual std::string GetRemoteHost() const;

  //virtual int GetLocalPort() const;

  //virtual int GetRemotePort() const;

  virtual std::string GetScheme() const;

  virtual std::string GetServerName() const;

  virtual int GetServerPort() const;

  virtual std::string GetProtocol() const;

  virtual std::string GetContextPath() const;

  virtual std::string GetPathInfo() const;

  virtual std::string GetRequestUri() const;

  virtual std::string GetRequestUrl() const;

  virtual std::string GetServletPath() const;

  virtual std::string GetQueryString() const;

  virtual std::string GetHeader(const std::string& name) const;

  virtual long long GetDateHeader(const std::string& name) const;

  virtual std::vector<std::string> GetHeaderNames() const;

  virtual std::vector<std::string> GetHeaders(const std::string& name) const;

  virtual std::string GetMethod() const;

  //virtual std::vector<std::pair<std::string, float>> GetAcceptHeader() const;

  virtual void RemoveAttribute(const std::string& name);

  virtual void SetAttribute(const std::string& name, const Any& value);

  /*!
   * Return the value of asked query parameter or empty Any if the parameter is
   * not found
   */
  virtual Any GetParameter(const std::string& name) const;

  /*!
   *
   * Gets all the Part components of this request, provided that it is of type
   * multipart/form-data.
   */
  virtual std::vector<HttpServletPart*> GetParts() const;

  /*!
  * Get the body of the request, if any
  */
  virtual std::string GetBody() const;

  /*!
   * Gets the Part with the given name.
   */
  virtual HttpServletPart* GetPart(const std::string& name) const;

  /*!
   * Triggers the load of files transmitted by HTTP POST, multipart/form-data
   * encoded
   *
   * An optional callback can be used to perform special operation on the
   * ServletPart
   */
  virtual void ReadParts(void* callback(HttpServletPart*) = 0);

  HttpServletRequest(HttpServletRequestPrivate* d);

private:
  friend class ServletHandler;
  friend class HttpServiceFactory;

  ExplicitlySharedDataPointer<HttpServletRequestPrivate> d;

  // CiveWeb callbacks used when calling ReadParts()

  static int field_found(const char* key,
                         const char* filename,
                         char* path,
                         size_t pathlen,
                         void* user_data);
  static int field_store(const char* path,
                         long long file_size,
                         void* user_data);
  static int field_get(const char* key,
                       const char* value,
                       size_t valuelen,
                       void* user_data);
};
}

#endif // CPPMICROSERVICES_HTTPREQUEST_H
