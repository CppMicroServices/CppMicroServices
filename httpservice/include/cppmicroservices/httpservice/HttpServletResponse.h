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

#ifndef CPPMICROSERVICES_HTTPSERVLETRESPONSE_H
#define CPPMICROSERVICES_HTTPSERVLETRESPONSE_H

#include "cppmicroservices/SharedData.h"
#include "cppmicroservices/httpservice/HttpServiceExport.h"
#include "cppmicroservices/httpservice/IHttpServletResponse.h"

#include <ctime>
#include <memory>
#include <string>

namespace cppmicroservices {

struct HttpServletResponsePrivate;

class US_HttpService_EXPORT HttpServletResponse : public IHttpServletResponse
{
public:
  ~HttpServletResponse();

  HttpServletResponse(const HttpServletResponse& o);
  HttpServletResponse(const IHttpServletResponse& o);
  HttpServletResponse& operator=(const HttpServletResponse& o);

  void FlushBuffer() override;

  bool IsCommitted() const override;

  std::size_t GetBufferSize() const override;

  std::string GetCharacterEncoding() const override;

  std::string GetContentType() const override;

  std::ostream& GetOutputStream() override;

  void Reset() override;

  void ResetBuffer() override;

  void SetBufferSize(std::size_t size) override;

  void SetCharacterEncoding(const std::string& charset) override;

  void SetContentLength(std::size_t size) override;

  void SetContentType(const std::string& type) override;

  void AddHeader(const std::string& name, const std::string& value) override;

  void SetHeader(const std::string& name, const std::string& value) override;

  void SetDateHeader(const std::string& name, long long date) override;

  void AddIntHeader(const std::string& name, int value) override;

  void SetIntHeader(const std::string& name, int value) override;

  bool ContainsHeader(const std::string& name) const override;

  std::string GetHeader(const std::string& name) const override;

  int GetStatus() const override;

  void SetStatus(int statusCode) override;

  void SendError(int statusCode,
                 const std::string& msg = std::string()) override;

  void SendRedirect(const std::string& location) override;

protected:
  virtual std::streambuf* GetOutputStreamBuffer();

  virtual void SetOutputStreamBuffer(std::streambuf* sb);

  HttpServletResponse(HttpServletResponsePrivate* d);

  ExplicitlySharedDataPointer<HttpServletResponsePrivate> d;

private:
  friend class HttpServlet;
  friend class ServletHandler;
  friend class HttpServiceFactory;
};

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_HTTPSERVLETRESPONSE_H
