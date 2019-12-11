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

#include "cppmicroservices/httpservice/HttpServletResponse.h"
#include "HttpOutputStreamBuffer.h"
#include "HttpServletResponsePrivate.h"
#include "cppmicroservices/httpservice/HttpServletRequest.h"
#include "cppmicroservices/httpservice/ServletContext.h"

#include "civetweb/civetweb.h"

#include <sstream>
#include <stdexcept>
#include <ctime>
#include <vector>

namespace cppmicroservices {

HttpServletResponsePrivate::HttpServletResponsePrivate(
  HttpServletRequest* request,
  CivetServer* server,
  mg_connection* conn)
  : m_Request(request)
  , m_Server(server)
  , m_Connection(conn)
  , m_StatusCode(HttpServletResponse::SC_NOT_FOUND)
  , m_StreamBuf(nullptr)
  , m_HttpOutputStreamBuf(nullptr)
  , m_HttpOutputStream(nullptr)
  , m_IsCommited(false)
  , m_BufferSize(1024)
{}

HttpServletResponsePrivate::~HttpServletResponsePrivate()
{
  delete m_StreamBuf;
  if (m_StreamBuf != m_HttpOutputStreamBuf) {
    delete m_HttpOutputStreamBuf;
  }
  delete m_HttpOutputStream;
}

bool HttpServletResponsePrivate::Commit()
{
  if (m_IsCommited)
    return true;

  std::stringstream ss;
  ss << "HTTP/1.1 " << m_StatusCode << "\r\n";
  for (auto & m_Header : m_Headers) {
    ss << m_Header.first << ": " << m_Header.second << "\r\n";
  }
  ss << "\r\n";

  std::string header = ss.str();
  int n = mg_write(m_Connection, &header[0], header.size());
  m_IsCommited = n > 0;
  return m_IsCommited;
}

std::string HttpServletResponsePrivate::LexicalCast(long value)
{
  char result[100];
  char* ptr = result;
  char* ptr1 = ptr;
  char tmp_char;

  int tmp_value;

  do {
    tmp_value = value;
    value /= 10;
    *ptr++ = "0123456789"[tmp_value - value * 10];
  } while (value);

  // Apply negative sign
  if (tmp_value < 0)
    *ptr++ = '-';

  *ptr-- = '\0';
  // Invert the string
  while (ptr1 < ptr) {
    tmp_char = *ptr;
    *ptr-- = *ptr1;
    *ptr1++ = tmp_char;
  }
  return result;
}

std::string HttpServletResponsePrivate::LexicalCastHex(long value)
{
  char result[100];
  char* ptr = result;
  char* ptr1 = ptr;
  char tmp_char;

  int tmp_value;

  do {
    tmp_value = value;
    value /= 16;
    *ptr++ = "0123456789abcdef"[tmp_value - value * 16];
  } while (value);

  // Apply negative sign
  if (tmp_value < 0)
    *ptr++ = '-';

  *ptr-- = '\0';
  // Invert the string
  while (ptr1 < ptr) {
    tmp_char = *ptr;
    *ptr-- = *ptr1;
    *ptr1++ = tmp_char;
  }
  return result;
}

HttpServletResponse::~HttpServletResponse() = default;
HttpServletResponse::HttpServletResponse(const HttpServletResponse&) = default;
HttpServletResponse& HttpServletResponse::operator=(const HttpServletResponse&) = default;

void HttpServletResponse::FlushBuffer()
{
  if (d->m_HttpOutputStream) {
    *d->m_HttpOutputStream << std::flush;
  } else {
    d->Commit();
  }
}

bool HttpServletResponse::IsCommitted() const
{
  return d->m_IsCommited;
}

std::size_t HttpServletResponse::GetBufferSize() const
{
  return d->m_BufferSize;
}

std::string HttpServletResponse::GetContentType() const
{
  auto iter =
    d->m_Headers.find("Content-Type");
  if (iter != d->m_Headers.end()) {
    return iter->second;
  }
  return std::string();
}

std::ostream& HttpServletResponse::GetOutputStream()
{
  if (!d->m_HttpOutputStream) {
    d->m_StreamBuf = this->GetOutputStreamBuffer();
    d->m_HttpOutputStream = new std::ostream(d->m_StreamBuf);
  }
  return *d->m_HttpOutputStream;
}

void HttpServletResponse::Reset()
{
  this->ResetBuffer();
  d->m_StatusCode = SC_NOT_FOUND;
  d->m_Headers.clear();
}

void HttpServletResponse::ResetBuffer()
{
  if (this->IsCommitted()) {
    throw std::logic_error(
      "Resetting buffer not possible, response is already committed.");
  }
  if (d->m_HttpOutputStream) {
    delete d->m_HttpOutputStream;
    delete d->m_StreamBuf;
    d->m_HttpOutputStream = nullptr;
    d->m_StreamBuf = nullptr;
  }
}

void HttpServletResponse::SetBufferSize(std::size_t size)
{
  if (d->m_StreamBuf) {
    throw std::logic_error(
      "Buffer size cannot be set because the buffer is already in use.");
  }
  d->m_BufferSize = size;
}

void HttpServletResponse::SetCharacterEncoding(const std::string& charset)
{
  if (this->IsCommitted() || d->m_HttpOutputStream != nullptr) {
    return;
  }
  d->m_Charset = charset;
  std::string contentType = this->GetContentType();
  if (!contentType.empty()) {
    std::size_t pos = contentType.find_first_of(';');
    if (pos == std::string::npos) {
      contentType += "; " + charset;
    } else {
      contentType = contentType.substr(0, pos) + "; " + charset;
    }
    d->m_Headers["Content-Type"] = contentType;
  }
}

void HttpServletResponse::SetContentLength(std::size_t size)
{
  std::stringstream ss;
  ss << size;
  d->m_Headers["Content-Length"] = ss.str();
}

void HttpServletResponse::SetContentType(const std::string& type)
{
  if (this->IsCommitted() || d->m_HttpOutputStream != nullptr) {
    return;
  }
  std::string contentType = type;
  if (!contentType.empty()) {
    std::size_t pos = type.find_first_of(';');
    if (pos == std::string::npos) {
      if (!d->m_Charset.empty()) {
        contentType += "; " + d->m_Charset;
      }
    } else {
      pos = contentType.find_first_not_of(' ', pos);
      d->m_Charset = contentType.substr(pos);
    }
  }
  d->m_Headers["Content-Type"] = contentType;
}

void HttpServletResponse::AddHeader(const std::string& name,
                                    const std::string& value)
{
  std::string& currValue = d->m_Headers[name];
  if (currValue.empty()) {
    currValue += value;
  } else {
    if (currValue[currValue.size() - 1] != ',') {
      currValue += ',';
    }
    currValue += value;
  }
}

void HttpServletResponse::SetHeader(const std::string& name,
                                    const std::string& value)
{
  d->m_Headers[name] = value;
}

#ifdef US_PLATFORM_WINDOWS
#  ifndef snprintf
#    define snprintf _snprintf
#  endif
#endif

void HttpServletResponse::SetDateHeader(const std::string& name, long long date)
{
  // Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123

  const char* months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                             "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
  const char* days[7] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
  std::time_t timeep = date / 1000;
  std::tm t;
#ifdef US_PLATFORM_WINDOWS
  if (gmtime_s(&t, &timeep) != 0)
#else
  if (gmtime_r(&timeep, &t) != nullptr)
#endif
  {
    char dataStr[30];
    int n = snprintf(dataStr,
                     30,
                     "%s, %02d %s %d %02d:%02d:%02d GMT",
                     days[t.tm_wday],
                     t.tm_mday,
                     months[t.tm_mon],
                     (1900 + t.tm_year),
                     t.tm_hour,
                     t.tm_min,
                     t.tm_sec);
    if (n < 0)
      return;
    this->SetHeader(name, dataStr);
  }
}

void HttpServletResponse::AddIntHeader(const std::string& name, int value)
{
  this->AddHeader(name, d->LexicalCast(value));
}

void HttpServletResponse::SetIntHeader(const std::string& name, int value)
{
  this->SetHeader(name, d->LexicalCast(value));
}

bool HttpServletResponse::ContainsHeader(const std::string& name) const
{
  return d->m_Headers.find(name) != d->m_Headers.end();
}

int HttpServletResponse::GetStatus() const
{
  return d->m_StatusCode;
}

void HttpServletResponse::SetStatus(int statusCode)
{
  d->m_StatusCode = statusCode;
}

void HttpServletResponse::SendError(int statusCode, const std::string& msg)
{
  if (this->IsCommitted()) {
    throw std::logic_error("Response already committed.");
  }
  this->SetStatus(statusCode);
  this->SetContentType("text/html");
  this->SetContentLength(msg.size());

  this->GetOutputStream() << msg << std::flush;
}

void HttpServletResponse::SendRedirect(const std::string& location)
{
  if (this->IsCommitted()) {
    throw std::logic_error("Response already committed.");
  }
  this->Reset();
  this->SetStatus(SC_FOUND);

  std::string contextPath = d->m_Request->GetServletContext()->GetContextPath();
  std::string scheme = d->m_Request->GetScheme();
  std::string redirectUri = (scheme.empty() ? std::string() : scheme) + "://" +
                            d->m_Request->GetServerName();
  std::stringstream ss;
  ss << d->m_Request->GetServerPort();
  std::string port = ss.str();
  if (!port.empty()) {
    redirectUri += std::string(":") + ss.str();
  }
  redirectUri += contextPath;

  std::size_t pos = location.find_first_of('/');

  if (redirectUri[redirectUri.size() - 1] != '/') {
    if (!location.empty() && location[0] != '/') {
      redirectUri += '/';
    }
  } else if (!location.empty() && location[0] == '/') {
    redirectUri = redirectUri.substr(0, redirectUri.size() - 1);
  }

  if (pos == 0 || location.empty()) {
    // relative url with leading '/' or an empty location
    redirectUri += location;
  } else {
    if (pos == std::string::npos) {
      // no '/' at all
      redirectUri += location;
    } else {
      std::size_t pos2 = location.find_first_of(':');
      if (pos2 != std::string::npos && pos2 == pos - 1) {
        // absolute url
        redirectUri = location;
      } else {
        // something weird
        redirectUri += location;
      }
    }
  }

  this->SetStatus(SC_TEMPORARY_REDIRECT);
  this->SetHeader("Location", redirectUri);
  d->Commit();
}

std::streambuf* HttpServletResponse::GetOutputStreamBuffer()
{
  if (d->m_HttpOutputStreamBuf == nullptr) {
    d->m_HttpOutputStreamBuf =
      new HttpOutputStreamBuffer(d.Data(), this->GetBufferSize());
  }
  return d->m_HttpOutputStreamBuf;
}

void HttpServletResponse::SetOutputStreamBuffer(std::streambuf* sb)
{
  delete d->m_StreamBuf;
  d->m_StreamBuf = sb;
}

HttpServletResponse::HttpServletResponse(HttpServletResponsePrivate* d)
  : d(d)
{}
}
