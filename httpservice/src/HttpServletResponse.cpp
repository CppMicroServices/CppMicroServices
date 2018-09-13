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
#include <time.h>
#include <vector>

namespace cppmicroservices {

HttpServletResponsePrivate::HttpServletResponsePrivate(HttpServletRequest* request,
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
  for (std::map<std::string, std::string>::iterator iter = m_Headers.begin(),
                                                    endIter = m_Headers.end();
       iter != endIter;
       ++iter) {
    ss << iter->first << ": " << iter->second << "\r\n";
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

HttpServletResponse::~HttpServletResponse() {}

HttpServletResponse::HttpServletResponse(const HttpServletResponse& o)
  : respdata_ptr(o.respdata_ptr)
{}

HttpServletResponse& HttpServletResponse::operator=(
  const HttpServletResponse& o)
{
  respdata_ptr = o.respdata_ptr;
  return *this;
}

void HttpServletResponse::FlushBuffer()
{
  if (respdata_ptr->m_HttpOutputStream) {
    *respdata_ptr->m_HttpOutputStream << std::flush;
  } else {
    respdata_ptr->Commit();
  }
}

bool HttpServletResponse::IsCommitted() const
{
  return respdata_ptr->m_IsCommited;
}

std::size_t HttpServletResponse::GetBufferSize() const
{
  return respdata_ptr->m_BufferSize;
}

std::string HttpServletResponse::GetContentType() const
{
  std::map<std::string, std::string>::iterator iter =
    respdata_ptr->m_Headers.find("Content-Type");
  if (iter != respdata_ptr->m_Headers.end()) {
    return iter->second;
  }
  return std::string();
}

std::ostream& HttpServletResponse::GetOutputStream()
{
  if (!respdata_ptr->m_HttpOutputStream) {
    respdata_ptr->m_StreamBuf = this->GetOutputStreamBuffer();
    respdata_ptr->m_HttpOutputStream = new std::ostream(respdata_ptr->m_StreamBuf);
  }
  return *respdata_ptr->m_HttpOutputStream;
}

void HttpServletResponse::Reset()
{
  this->ResetBuffer();
  respdata_ptr->m_StatusCode = SC_NOT_FOUND;
  respdata_ptr->m_Headers.clear();
}

void HttpServletResponse::ResetBuffer()
{
  if (this->IsCommitted()) {
    throw std::logic_error(
      "Resetting buffer not possible, response is already committed.");
  }
  if (respdata_ptr->m_HttpOutputStream) {
    delete respdata_ptr->m_HttpOutputStream;
    delete respdata_ptr->m_StreamBuf;
    respdata_ptr->m_HttpOutputStream = nullptr;
    respdata_ptr->m_StreamBuf = nullptr;
  }
}

void HttpServletResponse::SetBufferSize(std::size_t size)
{
  if (respdata_ptr->m_StreamBuf) {
    throw std::logic_error(
      "Buffer size cannot be set because the buffer is already in use.");
  }
  respdata_ptr->m_BufferSize = size;
}

void HttpServletResponse::SetCharacterEncoding(const std::string& charset)
{
  if (this->IsCommitted() || respdata_ptr->m_HttpOutputStream != nullptr) {
    return;
  }
  respdata_ptr->m_Charset = charset;
  std::string contentType = this->GetContentType();
  if (!contentType.empty()) {
    std::size_t pos = contentType.find_first_of(';');
    if (pos == std::string::npos) {
      contentType += "; " + charset;
    } else {
      contentType = contentType.substr(0, pos) + "; " + charset;
    }
    respdata_ptr->m_Headers["Content-Type"] = contentType;
  }
}

void HttpServletResponse::SetContentLength(std::size_t size)
{
  std::stringstream ss;
  ss << size;
  respdata_ptr->m_Headers["Content-Length"] = ss.str();
}

void HttpServletResponse::SetContentType(const std::string& type)
{
  if (this->IsCommitted() || respdata_ptr->m_HttpOutputStream != nullptr) {
    return;
  }
  std::string contentType = type;
  if (!contentType.empty()) {
    std::size_t pos = type.find_first_of(';');
    if (pos == std::string::npos) {
      if (!respdata_ptr->m_Charset.empty()) {
        contentType += "; " + respdata_ptr->m_Charset;
      }
    } else {
      pos = contentType.find_first_not_of(' ', pos);
      respdata_ptr->m_Charset = contentType.substr(pos);
    }
  }
  respdata_ptr->m_Headers["Content-Type"] = contentType;
}

void HttpServletResponse::AddHeader(const std::string& name,
                                    const std::string& value)
{
  std::string& currValue = respdata_ptr->m_Headers[name];
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
  respdata_ptr->m_Headers[name] = value;
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
  this->AddHeader(name, respdata_ptr->LexicalCast(value));
}

void HttpServletResponse::SetIntHeader(const std::string& name, int value)
{
  this->SetHeader(name, respdata_ptr->LexicalCast(value));
}

bool HttpServletResponse::ContainsHeader(const std::string& name) const
{
  return respdata_ptr->m_Headers.find(name) != respdata_ptr->m_Headers.end();
}

int HttpServletResponse::GetStatus() const
{
  return respdata_ptr->m_StatusCode;
}

void HttpServletResponse::SetStatus(int statusCode)
{
  respdata_ptr->m_StatusCode = statusCode;
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

  std::string contextPath = respdata_ptr->m_Request->GetServletContext()->GetContextPath();
  std::string scheme = respdata_ptr->m_Request->GetScheme();
  std::string redirectUri = (scheme.empty() ? std::string() : scheme) + "://" +
                            respdata_ptr->m_Request->GetServerName();
  std::stringstream ss;
  ss << respdata_ptr->m_Request->GetServerPort();
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
  respdata_ptr->Commit();
}

std::streambuf* HttpServletResponse::GetOutputStreamBuffer()
{
  if (respdata_ptr->m_HttpOutputStreamBuf == nullptr) {
    respdata_ptr->m_HttpOutputStreamBuf = new HttpOutputStreamBuffer(respdata_ptr, this->GetBufferSize());
  }
  return respdata_ptr->m_HttpOutputStreamBuf;
}

void HttpServletResponse::SetOutputStreamBuffer(std::streambuf* sb)
{
  delete respdata_ptr->m_StreamBuf;
  respdata_ptr->m_StreamBuf = sb;
}

HttpServletResponse::HttpServletResponse(const std::shared_ptr<HttpServletResponsePrivate>& d)
  : respdata_ptr(d)
{}
}
