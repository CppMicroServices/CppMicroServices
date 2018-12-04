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

#include "cppmicroservices/httpservice/HttpServlet.h"
#include "HttpServletPrivate.h"

#include "HttpOutputStreamBuffer.h"
#include "cppmicroservices/httpservice/HttpServletRequest.h"
#include "cppmicroservices/httpservice/HttpServletResponse.h"

#include <cassert>
#include <functional>
#include <iostream>

namespace cppmicroservices {

class NoBodyOutputStreamBuffer : public HttpOutputStreamBuffer
{
public:
  explicit NoBodyOutputStreamBuffer(HttpServletResponsePrivate* response)
    : HttpOutputStreamBuffer(response)
    , m_ContentLength(0)
  {}

  std::size_t GetContentLength() const { return m_ContentLength; }

protected:
  std::streamsize xsputn(const char* /*s*/, std::streamsize n) override
  {
    m_ContentLength += static_cast<std::size_t>(n);
    return n;
  }

private:
  int_type overflow(int_type ch) override
  {
    if (ch != traits_type::eof()) {
      assert(std::less_equal<char*>()(pptr(), epptr()));
      ++m_ContentLength;
      // discard the character
      pbump(static_cast<int>(pbase() - pptr()));
      return ch;
    }
    return traits_type::eof();
  }

  int sync() override { return this->CommitStream() ? 0 : -1; }

  NoBodyOutputStreamBuffer(const NoBodyOutputStreamBuffer&) = delete;
  NoBodyOutputStreamBuffer& operator=(const NoBodyOutputStreamBuffer&) = delete;

private:
  std::size_t m_ContentLength;
};

class NoBodyResponse : public HttpServletResponse
{
public:
  NoBodyResponse(HttpServletResponsePrivate* d)
    : HttpServletResponse(d)
    , m_NoBodyBuffer(new NoBodyOutputStreamBuffer(d))
  //, m_DidSetContentLength(false)
  {
    this->SetOutputStreamBuffer(m_NoBodyBuffer);
  }

  using HttpServletResponse::SetContentLength;

  void SetContentLength()
  {
    if (!this->ContainsHeader("Content-Type")) {
      this->SetContentLength(m_NoBodyBuffer->GetContentLength());
    }
  }

private:
  NoBodyOutputStreamBuffer* m_NoBodyBuffer;
  //bool m_DidSetContentLength;
};

static const std::string METHOD_DELETE = "DELETE";
static const std::string METHOD_HEAD = "HEAD";
static const std::string METHOD_GET = "GET";
static const std::string METHOD_OPTIONS = "OPTIONS";
static const std::string METHOD_POST = "POST";
static const std::string METHOD_PUT = "PUT";
static const std::string METHOD_TRACE = "TRACE";

static const std::string HEADER_IFMODSINCE = "If-Modified-Since";
static const std::string HEADER_LASTMOD = "Last-Modified";

/*
 * Sets the Last-Modified entity header field, if it has not
 * already been set and if the value is meaningful.  Called before
 * DoGet, to ensure that headers are set before response data is
 * written. A subclass might have set this header already, so we
 * check.
 */
static void MaybeSetLastModified(HttpServletResponse& resp,
                                 long long lastModified)
{
  if (resp.ContainsHeader(HEADER_LASTMOD))
    return;
  if (lastModified >= 0)
    resp.SetDateHeader(HEADER_LASTMOD, lastModified);
}

const std::string HttpServlet::PROP_CONTEXT_ROOT =
  "org.cppmicroservices.HttpServlet.contextRoot";

HttpServlet::HttpServlet()
  : d(new HttpServletPrivate)
{}

void HttpServlet::Init(const ServletConfig& config)
{
  d->m_Config = config;
}

void HttpServlet::Destroy()
{
  Lock(), d->m_Config = ServletConfig();
}

ServletConfig HttpServlet::GetServletConfig() const
{
  return d->m_Config;
}

std::shared_ptr<ServletContext> HttpServlet::GetServletContext() const
{
  return Lock(), d->m_Config.GetServletContext();
}

void HttpServlet::Service(HttpServletRequest& request,
                          HttpServletResponse& response)
{
  std::string method = request.GetMethod();

  if (method == METHOD_GET) {
    long long lastModified = GetLastModified(request);
    if (lastModified == -1) {
      // servlet doesn't support if-modified-since, no reason
      // to go through further expensive logic
      DoGet(request, response);
    } else {
      long long ifModifiedSince = request.GetDateHeader(HEADER_IFMODSINCE);
      std::cout << "ifModifiedSince: " << ifModifiedSince << std::endl;
      std::cout << "lastModified: " << lastModified << std::endl;
      if (ifModifiedSince < lastModified) {
        // If the servlet mod time is later, call doGet()
        // Round down to the nearest second for a proper compare
        // A ifModifiedSince of -1 will always be less
        MaybeSetLastModified(response, lastModified);
        DoGet(request, response);
      } else {
        response.SetStatus(HttpServletResponse::SC_NOT_MODIFIED);
      }
    }

  } else if (method == METHOD_HEAD) {
    long long lastModified = GetLastModified(request);
    MaybeSetLastModified(response, lastModified);
    DoHead(request, response);
  } else if (method == METHOD_POST) {
    DoPost(request, response);
  } else if (method == METHOD_PUT) {
    DoPut(request, response);
  } else if (method == METHOD_DELETE) {
    DoDelete(request, response);
  }
  //  else if (method == METHOD_OPTIONS)
  //  {
  //    DoOptions(request, response);
  //  }
  else if (method == METHOD_TRACE) {
    DoTrace(request, response);
  } else {
    //
    // Note that this means NO servlet supports whatever
    // method was requested, anywhere on this server.
    //

    std::string errMsg =
      std::string("Http method ") + method + " not implemented";
    response.SendError(HttpServletResponse::SC_NOT_IMPLEMENTED, errMsg);
  }
}

HttpServlet::~HttpServlet()
{
  delete d;
}

long long HttpServlet::GetLastModified(HttpServletRequest& /*request*/)
{
  return -1;
}

void HttpServlet::DoGet(HttpServletRequest& request,
                        HttpServletResponse& response)
{
  std::string protocol = request.GetProtocol();
  std::string msg = "Http GET method not supported";
  if (protocol.size() > 2 && protocol.substr(protocol.size() - 3) == "1.1") {
    response.SendError(HttpServletResponse::SC_METHOD_NOT_ALLOWED, msg);
  } else {
    response.SendError(HttpServletResponse::SC_BAD_REQUEST, msg);
  }
}

void HttpServlet::DoHead(HttpServletRequest& request,
                         HttpServletResponse& response)
{
  NoBodyResponse noBodyResponse(response.d.Data());

  DoGet(request, noBodyResponse);
  noBodyResponse.SetContentLength();
}

void HttpServlet::DoDelete(HttpServletRequest& request,
                           HttpServletResponse& response)
{
  std::string protocol = request.GetProtocol();
  std::string msg = "Http DELETE method not supported";
  if (protocol.size() > 2 && protocol.substr(protocol.size() - 3) == "1.1") {
    response.SendError(HttpServletResponse::SC_METHOD_NOT_ALLOWED, msg);
  } else {
    response.SendError(HttpServletResponse::SC_BAD_REQUEST, msg);
  }
}

void HttpServlet::DoPost(HttpServletRequest& request,
                         HttpServletResponse& response)
{
  std::string protocol = request.GetProtocol();
  std::string msg = "Http POST method not supported";
  if (protocol.size() > 2 && protocol.substr(protocol.size() - 3) == "1.1") {
    response.SendError(HttpServletResponse::SC_METHOD_NOT_ALLOWED, msg);
  } else {
    response.SendError(HttpServletResponse::SC_BAD_REQUEST, msg);
  }
}

void HttpServlet::DoPut(HttpServletRequest& request,
                        HttpServletResponse& response)
{
  std::string protocol = request.GetProtocol();
  std::string msg = "Http PUT method not supported";
  if (protocol.size() > 2 && protocol.substr(protocol.size() - 3) == "1.1") {
    response.SendError(HttpServletResponse::SC_METHOD_NOT_ALLOWED, msg);
  } else {
    response.SendError(HttpServletResponse::SC_BAD_REQUEST, msg);
  }
}

void HttpServlet::DoTrace(HttpServletRequest& request,
                          HttpServletResponse& response)
{
  std::string CRLF = "\r\n";
  std::string responseString =
    "TRACE " + request.GetRequestUri() + " " + request.GetProtocol();

  std::vector<std::string> reqHeaders = request.GetHeaderNames();

  for (const auto & reqHeader : reqHeaders) {
    responseString +=
      CRLF + reqHeader + ": " + request.GetHeader(reqHeader);
  }

  responseString += CRLF;

  response.SetContentType("message/http");
  response.SetContentLength(responseString.size());
  response.GetOutputStream() << responseString << std::flush;
  return;
}

std::unique_lock<std::mutex> HttpServlet::Lock() const
{
  return std::unique_lock<std::mutex>(d->m_Mutex);
}
}
