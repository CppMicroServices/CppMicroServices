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
#include "cppmicroservices/httpservice/IHttpServletRequest.h"
#include "cppmicroservices/httpservice/IHttpServletPart.h"

#include <string>
#include <vector>

namespace cppmicroservices {

class ServletContext;
struct HttpServletRequestPrivate;
struct HttpServletPartPrivate;

class US_HttpService_EXPORT HttpServletRequest : public IHttpServletRequest
{
public:
  ~HttpServletRequest();

  HttpServletRequest(const HttpServletRequest& o);
  HttpServletRequest& operator=(const HttpServletRequest& o);

  std::shared_ptr<ServletContext> GetServletContext() const override;

  Any GetAttribute(const std::string& name) const override;

  std::vector<std::string> GetAttributeNames() const override;

  std::size_t GetContentLength() const override;

  std::string GetContentType() const override;

  // std::string GetLocalName() const override;

  // std::string GetRemoteHost() const override;

  // int GetLocalPort() const override;

  // int GetRemotePort() const override;

  std::string GetScheme() const override;

  std::string GetServerName() const override;

  int GetServerPort() const override;

  std::string GetProtocol() const override;

  std::string GetContextPath() const override;

  std::string GetPathInfo() const override;

  std::string GetRequestUri() const override;

  std::string GetRequestUrl() const override;

  std::string GetServletPath() const override;

  std::string GetQueryString() const override;

  std::string GetHeader(const std::string& name) const override;

  long long GetDateHeader(const std::string& name) const override;

  std::vector<std::string> GetHeaderNames() const override;

  std::vector<std::string> GetHeaders(const std::string& name) const override;

  std::string GetMethod() const override;

  //virtual std::vector<std::pair<std::string, float>> GetAcceptHeader() const override;

  void RemoveAttribute(const std::string& name) override;

  void SetAttribute(const std::string& name, const Any& value) override;

  /*!
   * Return the value of asked query parameter or empty Any if the parameter is
   * not found
   */
  Any GetParameter(const std::string& name) const override;

  /*!
   *
   * Gets all the Part components of this request, provided that it is of type
   * multipart/form-data.
   */
  std::vector<IHttpServletPart*> GetParts() const override;

  /*!
  * Get the body of the request, if any
  */
  std::string GetBody() const override;

  /*!
   * Gets the Part with the given name.
   */
  IHttpServletPart* GetPart(const std::string& name) const override;

  /*!
   * Triggers the load of files transmitted by HTTP POST, multipart/form-data
   * encoded
   *
   * An optional callback can be used to perform special operation on the
   * ServletPart
   */
  void ReadParts(void* callback(IHttpServletPart*) = 0) override;

  HttpServletRequest(HttpServletRequestPrivate* d);

private:
  friend class ServletHandler;
  friend class HttpServiceFactory;

  void* RawData() override;

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
