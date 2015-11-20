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

#include "usHttpServletRequest.h"
#include "usHttpServletRequest_p.h"

#include "usServletContext.h"

#include "civetweb/civetweb.h"

#include <cassert>
#include <ctime>
#include <cstring>

namespace us {

void putenv(const char* key, const char* val)
{
#ifdef US_PLATFORM_WINDOWS
  _putenv_s(key, val);
#else
  setenv(key, val, 1);
#endif
}

void unsetenv(const char* key)
{
#ifdef US_PLATFORM_WINDOWS
  _putenv_s(key, "");
#else
  unsetenv(key);
#endif
}

HttpServletRequestPrivate::HttpServletRequestPrivate(const std::shared_ptr<ServletContext>& servletContext,
                                       CivetServer* server, mg_connection* conn)
  : m_ServletContext(servletContext)
  , m_Server(server)
  , m_Connection(conn)
  , m_Scheme("http")
  , m_ServerPort("80")
{
  std::string host = mg_get_header(m_Connection, "Host");

  // find the scheme
  std::size_t pos = host.find_first_of(':');
  std::size_t pos2 = host.find_first_of('/');
  if (pos != std::string::npos && pos2 != std::string::npos &&
      pos2 == pos+1)
  {
    m_Scheme = host.substr(0, pos);
    if (pos2 == host.size()-1)
    {
      return;
    }
    else
    {
      pos = pos2+1;
    }
  }
  else
  {
    pos = 0;
  }

  // find the host name
  pos2 = host.find_first_of(":", pos);
  m_ServerName = host.substr(pos, pos2);
  pos = pos2;

  // find the port
  if (pos != std::string::npos)
  {
    pos2 = host.find_first_of('/', pos);
    m_ServerPort = host.substr(pos+1, pos2 == std::string::npos ? pos2 : pos2-pos);
    pos = pos2;
  }

  // get the uri
  std::string uri = mg_get_request_info(m_Connection)->uri;
  pos = uri.find_first_of('?');
  m_Uri = uri.substr(0, pos);

  // get the query string
  if (pos != std::string::npos)
  {
    m_QueryString = m_Uri.substr(pos);
  }

  // reconstruct the url
  m_Url = m_Scheme + "://" + m_ServerName + ":" + m_ServerPort + m_Uri;
}

HttpServletRequest::~HttpServletRequest()
{
}
HttpServletRequest::HttpServletRequest(const HttpServletRequest& o)
  : d(o.d)
{
}

HttpServletRequest& HttpServletRequest::operator=(const HttpServletRequest& o)
{
  d = o.d;
  return *this;
}

std::shared_ptr<ServletContext> HttpServletRequest::GetServletContext() const
{
  return d->m_ServletContext;
}

Any HttpServletRequest::GetAttribute(const std::string& name) const
{
  HttpServletRequestPrivate::AttributeMapType::const_iterator iter = d->m_Attributes.find(name);
  if (iter == d->m_Attributes.end())
  {
    return Any();
  }
  return iter->second;
}

std::vector<std::string> HttpServletRequest::GetAttributeNames() const
{
  std::vector<std::string> names;
  for(HttpServletRequestPrivate::AttributeMapType::const_iterator iter =
      d->m_Attributes.begin(), endIter = d->m_Attributes.end();
      iter != endIter; ++iter)
  {
    names.push_back(iter->first);
  }
  return names;
}

std::size_t HttpServletRequest::GetContentLength() const
{
  const char* contentLength = mg_get_header(d->m_Connection, "Content-Length");
  std::string contentLengthStr = contentLength ? std::string(contentLength) : std::string();
  std::stringstream ss(contentLengthStr);
  std::size_t length = 0;
  ss >> length;
  return length;
}

std::string HttpServletRequest::GetContentType() const
{
  const char* contentType = mg_get_header(d->m_Connection, "Content-Type");
  return contentType ? std::string(contentType) : std::string();
}

std::string HttpServletRequest::GetScheme() const
{
  return d->m_Scheme;
}

std::string HttpServletRequest::GetServerName() const
{
  return d->m_ServerName;
}

int HttpServletRequest::GetServerPort() const
{
  std::stringstream ss;
  ss.str(d->m_ServerPort);
  int port = 0;
  ss >> port;
  return port;
}

std::string HttpServletRequest::GetProtocol() const
{
  return mg_get_request_info(d->m_Connection)->http_version;
}

std::string HttpServletRequest::GetContextPath() const
{
  return d->m_ContextPath;
}

std::string HttpServletRequest::GetPathInfo() const
{
  return d->m_PathInfo;
}

std::string HttpServletRequest::GetRequestUri() const
{
  return d->m_Uri;
}

std::string HttpServletRequest::GetRequestUrl() const
{
  return d->m_Url;
}

std::string HttpServletRequest::GetServletPath() const
{
  return d->m_ServletPath;
}

std::string HttpServletRequest::GetQueryString() const
{
  return d->m_QueryString;
}

std::string HttpServletRequest::GetHeader(const std::string& name) const
{
  const char* header = mg_get_header(d->m_Connection, name.c_str());
  if (header) return header;
  return std::string();
}

long long HttpServletRequest::GetDateHeader(const std::string& name) const
{
  // Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
  // Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
  // Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format

  std::string data = this->GetHeader(name);
  if (data.empty()) return -1;

  const char* months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  char month[4];
  std::tm tm;
  int n = sscanf(data.c_str(), "%*[a-zA-Z], %d %3[a-zA-Z] %d %d:%d:%d GMT", &tm.tm_mday, month, &tm.tm_year, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
  if (n != 6)
  {
    n = sscanf(data.c_str(), "%*[a-zA-Z], %d-%3[a-zA-Z]-%d %d:%d:%d GMT", &tm.tm_mday, month, &tm.tm_year, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
  }
  if (n != 6)
  {
    n = sscanf(data.c_str(), "%*[a-zA-Z] %3[a-zA-Z] %d %d:%d:%d %d", month, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &tm.tm_year);
  }
  if (n != 6) return -1;

  for (int i = 0; i < 12; ++i)
  {
    if (std::strcmp(months[i], month) == 0)
    {
      tm.tm_mon = i;
      break;
    }
  }

  tm.tm_year -= 1900;

  time_t ret;
  char* tz = getenv("TZ");
  if (tz)
  {
    tz = strdup(tz);
  }
  unsetenv("TZ");
  tzset();
  ret = mktime(&tm);
  if (tz)
  {
    putenv("TZ", tz);
    free(tz);
  }
  else
  {
    unsetenv("TZ");
  }
  tzset();
  return ret * 1000;
}

std::vector<std::string> HttpServletRequest::GetHeaderNames() const
{
  std::vector<std::string> names;
  for (int i = 0; i < mg_get_request_info(d->m_Connection)->num_headers; ++i)
  {
    names.push_back(mg_get_request_info(d->m_Connection)->http_headers[i].name);
  }
  return names;
}

std::vector<std::string> HttpServletRequest::GetHeaders(const std::string& name) const
{
  std::vector<std::string> headers;
  for (int i = 0; i < mg_get_request_info(d->m_Connection)->num_headers; ++i)
  {
    if (name == mg_get_request_info(d->m_Connection)->http_headers[i].name)
    {
      std::string text = mg_get_request_info(d->m_Connection)->http_headers[i].value;

      // split comma-separated values and trim tokens
      char sep = ',';
      std::size_t start = text.find_first_not_of(sep);
      std::size_t end = start;
      while ((end = text.find(sep, start)) != std::string::npos ||
             (end == std::string::npos && start != end))
      {
        std::size_t p1 = text.find_first_not_of(' ', start);
        std::size_t p2 = text.find_last_not_of(' ', end == std::string::npos ? end : end-1);
        headers.push_back(text.substr(p1, p2-p1+1));
        start = text.find_first_not_of(sep, end);
      }
    }
  }
  return headers;
}

std::string HttpServletRequest::GetMethod() const
{
  return mg_get_request_info(d->m_Connection)->request_method;
}

void HttpServletRequest::SetAttribute(const std::string& name, const Any& value)
{
  d->m_Attributes[name] = value;
}

HttpServletRequest::HttpServletRequest(HttpServletRequestPrivate* d)
 : d(d)
{
}

}
