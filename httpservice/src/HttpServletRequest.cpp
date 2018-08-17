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

#include "cppmicroservices/httpservice/HttpServletRequest.h"
#include "HttpServletRequestPrivate.h"
#include "cppmicroservices/util/FileSystem.h"

#include "FileHttpServletPartPrivate.h"
#include "MemoryHttpServletPartPrivate.h"

#include "HttpServletPartPrivate.h"
#include "cppmicroservices/httpservice/HttpServletPart.h"
#include "cppmicroservices/httpservice/ServletContext.h"

#include "civetweb/civetweb.h"

#include <cassert>
#include <cstring>
#include <ctime>
#include <utility>

#include <iostream>
#include <regex>
#include <stdexcept>
#include <stdlib.h>

#include <map>
#include <regex>
#include <string>

#ifdef US_PLATFORM_WINDOWS
#  define timegm(x) (_mkgmtime(x))
#endif

namespace cppmicroservices {

HttpServletRequestPrivate::HttpServletRequestPrivate(
  std::shared_ptr<ServletContext> servletContext,
  CivetServer* server,
  mg_connection* conn)
  : m_ServletContext(servletContext)
  , m_Server(server)
  , m_Connection(conn)
  , m_Scheme("http")
  , m_ServerPort("80")
  , m_PartsMap()
{
  const char* headerHostLine = mg_get_header(m_Connection, "Host");
  std::string host;
  if (headerHostLine) {

    host = headerHostLine;
    // find the scheme
    std::size_t HostNamePos;
    std::size_t schemeColonPos = host.find_first_of(':');
    std::size_t firstSlashPos = host.find_first_of('/');
    if (schemeColonPos != std::string::npos &&
        firstSlashPos != std::string::npos &&
        firstSlashPos == schemeColonPos + 1) {
      m_Scheme = host.substr(0, schemeColonPos);
      if (firstSlashPos == host.size() - 1) {
        return;
      } else {
        HostNamePos = firstSlashPos + 2; // two slashes
      }
    } else {
      HostNamePos = 0;
    }
    // find the host name
    m_ServerName = host.substr(HostNamePos);
    // at this point, m_ServerName can
    // contain toto.com:8080, we must strip
    // port
    std::size_t portColonPos = m_ServerName.find_first_of(':');
    if (portColonPos != std::string::npos) {
      std::size_t uriDelimiterPos = m_ServerName.find_first_of('/');
      if (uriDelimiterPos != std::string::npos) {
        m_ServerPort = m_ServerName.substr(portColonPos + 1, uriDelimiterPos);
      } else {
        m_ServerPort = m_ServerName.substr(portColonPos + 1);
      }
      m_ServerName = m_ServerName.substr(0, portColonPos);
    }

    // get the uri, which is query string without parameter
    // ie http://foo:8080/index.php?bar=baz : uri is http://foo:8080/index.php
    std::string uri = mg_get_request_info(m_Connection)->request_uri;
    std::size_t uriPos = uri.find_first_of('?');
    if (portColonPos != std::string::npos) {
      m_Uri = uri.substr(0, uriPos);
    } else {
      m_Uri = uri;
    }
  }
  const mg_request_info* requInfo = mg_get_request_info(m_Connection);

  std::string civetQueryString = "";
  if (nullptr != requInfo->query_string) {
    civetQueryString = requInfo->query_string;
  }
  if (civetQueryString.length() > 0) {
    // trust Civet
    m_QueryString = civetQueryString;
    if (m_QueryString.length() > 0) {
      ExtractAttributesFromQueryString(m_QueryString);
    }
  }

  // reconstruct the url
  m_Url = m_Scheme + "://" + m_ServerName + ":" + m_ServerPort + m_Uri;
  if (m_QueryString.length() > 0) {
    m_Url += "?" + m_QueryString;
  }
}

HttpServletRequestPrivate::~HttpServletRequestPrivate()
{
  for (auto partMapElement : m_PartsMap) {
    delete partMapElement.second;
  }
}

HttpServletRequest::~HttpServletRequest() {}
HttpServletRequest::HttpServletRequest(const HttpServletRequest& o)
  : d(o.d)
{}

HttpServletRequest& HttpServletRequest::operator=(const HttpServletRequest& o)
{
  d = o.d;
  return *this;
}

void HttpServletRequestPrivate::ExtractAttributesFromQueryString(
  const std::string& queryString)
{
  std::map<std::string, std::string> data;
  std::regex pattern("([\\w+%]+)=([^&]*)");
  auto words_begin =
    std::sregex_iterator(queryString.begin(), queryString.end(), pattern);
  auto words_end = std::sregex_iterator();
  char decoded[1024];
  for (std::sregex_iterator i = words_begin; i != words_end; i++) {
    std::string key = (*i)[1].str();
    std::string value = (*i)[2].str();
    data[key] = value;
    const int srcLen = static_cast<int>(value.length()) + 1;
    mg_url_decode(value.c_str(), srcLen, decoded, srcLen, 0);
    m_Parameters[key] = std::string(decoded);
  }
}

Any HttpServletRequest::GetParameter(const std::string& name) const
{
  HttpServletRequestPrivate::ParameterMapType::const_iterator iter =
    d->m_Parameters.find(name);
  if (iter == d->m_Parameters.end()) {
    return Any();
  }
  return Any(iter->second);
}

std::shared_ptr<ServletContext> HttpServletRequest::GetServletContext() const
{
  return d->m_ServletContext;
}

Any HttpServletRequest::GetAttribute(const std::string& name) const
{
  HttpServletRequestPrivate::AttributeMapType::const_iterator iter =
    d->m_Attributes.find(name);
  if (iter == d->m_Attributes.end()) {
    return Any();
  }
  return iter->second;
}

std::vector<std::string> HttpServletRequest::GetAttributeNames() const
{
  std::vector<std::string> names;
  for (HttpServletRequestPrivate::AttributeMapType::const_iterator
         iter = d->m_Attributes.begin(),
         endIter = d->m_Attributes.end();
       iter != endIter;
       ++iter) {
    names.push_back(iter->first);
  }
  return names;
}

std::size_t HttpServletRequest::GetContentLength() const
{
  const char* contentLength = mg_get_header(d->m_Connection, "Content-Length");
  std::string contentLengthStr =
    contentLength ? std::string(contentLength) : std::string();
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
  if (header)
    return header;
  return std::string();
}

long long HttpServletRequest::GetDateHeader(const std::string& name) const
{
  // Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
  // Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
  // Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format

  std::string datetime = this->GetHeader(name);
  if (datetime.empty())
    return -1;

  const std::size_t num_months = 12;
  const char* months[num_months] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

  char month_str[32] = { 0 };
  int second, minute, hour, day, month, year;
  time_t result{};
  struct tm tm;

  if ((sscanf(datetime.c_str(),
              "%d/%3s/%d %d:%d:%d",
              &day,
              month_str,
              &year,
              &hour,
              &minute,
              &second) == 6) ||
      (sscanf(datetime.c_str(),
              "%d %3s %d %d:%d:%d",
              &day,
              month_str,
              &year,
              &hour,
              &minute,
              &second) == 6) ||
      (sscanf(datetime.c_str(),
              "%*3s, %d %3s %d %d:%d:%d",
              &day,
              month_str,
              &year,
              &hour,
              &minute,
              &second) == 6) ||
      (sscanf(datetime.c_str(),
              "%d-%3s-%d %d:%d:%d",
              &day,
              month_str,
              &year,
              &hour,
              &minute,
              &second) == 6)) {

    month = -1;
    for (std::size_t i = 0; i < num_months; ++i) {
      if (std::strcmp(months[i], month_str) == 0) {
        month = static_cast<int>(i);
        break;
      }
    }

    if ((month >= 0) && (year >= 1970)) {
      memset(&tm, 0, sizeof(tm));
      tm.tm_year = year - 1900;
      tm.tm_mon = month;
      tm.tm_mday = day;
      tm.tm_hour = hour;
      tm.tm_min = minute;
      tm.tm_sec = second;
      result = timegm(&tm);
    }
  }

  return result * 1000;
}

std::vector<std::string> HttpServletRequest::GetHeaderNames() const
{
  std::vector<std::string> names;
  for (int i = 0; i < mg_get_request_info(d->m_Connection)->num_headers; ++i) {
    names.emplace_back(
      mg_get_request_info(d->m_Connection)->http_headers[i].name);
  }
  return names;
}

std::vector<std::string> HttpServletRequest::GetHeaders(
  const std::string& name) const
{
  std::vector<std::string> headers;
  for (int i = 0; i < mg_get_request_info(d->m_Connection)->num_headers; ++i) {
    if (name == mg_get_request_info(d->m_Connection)->http_headers[i].name) {
      std::string text =
        mg_get_request_info(d->m_Connection)->http_headers[i].value;

      // split comma-separated values and trim tokens
      char sep = ',';
      std::size_t start = text.find_first_not_of(sep);
      std::size_t end = start;
      while ((end = text.find(sep, start)) != std::string::npos ||
             (end == std::string::npos && start != end)) {
        std::size_t p1 = text.find_first_not_of(' ', start);
        std::size_t p2 =
          text.find_last_not_of(' ', end == std::string::npos ? end : end - 1);
        headers.push_back(text.substr(p1, p2 - p1 + 1));
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

std::string HttpServletRequest::GetBody() const
{
  std::string putdata;

  char buf[2048] = { "\0" };
  int r = mg_read(d->m_Connection, buf, sizeof(buf));
  while (r > 0) {
    std::string p = std::string(buf);
    p.resize(r);
    putdata += p;
    r = mg_read(d->m_Connection, buf, sizeof(buf));
  }

  return putdata;
}

void HttpServletRequest::SetAttribute(const std::string& name, const Any& value)
{
  d->m_Attributes[name] = value;
}

void HttpServletRequest::RemoveAttribute(const std::string& name)
{
  if (d->m_Attributes.find(name) != d->m_Attributes.end()) {
    d->m_Attributes.erase(name);
  }
}

struct UserData
{
  HttpServletPartPrivate* PrivatePart;
  void* Callback;
  HttpServletRequest* request;

  UserData()
    : PrivatePart(nullptr)
    , Callback(nullptr)
    , request(nullptr)
  {}
};

int HttpServletRequest::field_found(const char* key,
                                    const char* filename,
                                    char* path,
                                    size_t /* pathlen */,
                                    void* user_data)
{
  int resultCode = -1;
  HttpServletPartPrivate* privatePart;

  UserData* user_data_parsed = static_cast<UserData*>(user_data);
  HttpServletRequest* request = user_data_parsed->request;

  if (filename != nullptr && strlen(filename) > 0) {
    // this is a file upload
    FileHttpServletPartPrivate* filePrivatePart;
    filePrivatePart =
      new FileHttpServletPartPrivate(request->d->m_ServletContext,
                                     request->d->m_Server,
                                     request->d->m_Connection);
    util::File file = util::MakeUniqueTempFile(request->d->m_TempDirname);
    filePrivatePart->m_TemporaryFileName = file.Path;

    filePrivatePart->m_SubmittedFileName = std::string(filename);
    strncpy(path,
            filePrivatePart->m_TemporaryFileName.c_str(),
            filePrivatePart->m_TemporaryFileName.size());

    privatePart = filePrivatePart;
    resultCode = MG_FORM_FIELD_STORAGE_STORE;
  } else {
    // This is a simple HTTP form parameter
    MemoryHttpServletPartPrivate* memoryPrivatePart;
    memoryPrivatePart =
      new MemoryHttpServletPartPrivate(request->d->m_ServletContext,
                                       request->d->m_Server,
                                       request->d->m_Connection);

    privatePart = memoryPrivatePart;
    resultCode = MG_FORM_FIELD_STORAGE_GET;
  }
  privatePart->m_Name = std::string(key);
  user_data_parsed->PrivatePart = privatePart;

  return resultCode;
}

int HttpServletRequest::field_get(const char* key,
                                  const char* value,
                                  size_t valuelen,
                                  void* user_data)
{
  if (strlen(key) == 0) {
    return 0;
  }
  UserData* user_data_parsed = static_cast<UserData*>(user_data);
  HttpServletRequest* request = user_data_parsed->request;
  request->d->m_Parameters[key] = std::string(value).substr(0, valuelen);

  MemoryHttpServletPartPrivate* privatePart;
  privatePart =
    dynamic_cast<MemoryHttpServletPartPrivate*>(user_data_parsed->PrivatePart);
  if (nullptr != privatePart) {
    privatePart->m_Value = std::string(value).substr(0, valuelen);
    HttpServletPart* part = new HttpServletPart(privatePart);

#ifdef _MSC_VER
    void (*callback)(HttpServletPart*) =
      static_cast<void (*)(HttpServletPart*)>(user_data_parsed->Callback);
#else
    void (*callback)(HttpServletPart*) =
      reinterpret_cast<decltype(callback)>(user_data_parsed->Callback);
#endif

    HttpServletRequest* httpRequest;
    httpRequest = static_cast<HttpServletRequest*>(user_data_parsed->request);
    httpRequest->d->m_PartsMap.insert(
      std::pair<std::string, HttpServletPart*>(privatePart->m_Name, part));

    if (callback != NULL) {
      callback(part);
    }
  }
  return 0;
}

int HttpServletRequest::field_store(const char* path,
                                    long long file_size,
                                    void* user_data)
{
  UserData* user_data_parsed = static_cast<UserData*>(user_data);

  FileHttpServletPartPrivate* privatePart;
  privatePart =
    dynamic_cast<FileHttpServletPartPrivate*>(user_data_parsed->PrivatePart);
  if (nullptr != privatePart) {
    HttpServletPart* part = new HttpServletPart(privatePart);

    privatePart->m_TemporaryFileName = std::string(path);
    privatePart->m_Size = file_size;

#ifdef _MSC_VER
    void (*callback)(HttpServletPart*) =
      static_cast<void (*)(HttpServletPart*)>(user_data_parsed->Callback);
#else
    void (*callback)(HttpServletPart*) =
      reinterpret_cast<decltype(callback)>(user_data_parsed->Callback);
#endif

    HttpServletRequest* httpRequest;
    httpRequest = static_cast<HttpServletRequest*>(user_data_parsed->request);
    httpRequest->d->m_PartsMap.insert(
      std::pair<std::string, HttpServletPart*>(privatePart->m_Name, part));

    if (callback != NULL) {
      callback(part);
    }
  }
  return 0;
}

void HttpServletRequest::ReadParts(void* callback(HttpServletPart*))
{
  UserData user_data;
  user_data.PrivatePart = nullptr;
  user_data.Callback = reinterpret_cast<decltype(user_data.Callback)>(callback);
  user_data.request = this;

  mg_form_data_handler fdh = {
    HttpServletRequest::field_found,
    HttpServletRequest::field_get,
    HttpServletRequest::field_store,
    &user_data,
  };

  mg_handle_form_request(d->m_Connection, &fdh);
}

HttpServletRequest::HttpServletRequest(HttpServletRequestPrivate* d)
  : d(d)
{}

std::vector<HttpServletPart*> HttpServletRequest::GetParts() const
{
  std::vector<HttpServletPart*> v;
  for (std::map<std::string, HttpServletPart*>::iterator it =
         d->m_PartsMap.begin();
       it != d->m_PartsMap.end();
       ++it) {
    v.push_back(it->second);
  }
  return v;
}

HttpServletPart* HttpServletRequest::GetPart(const std::string& name) const
{
  HttpServletPart* result = nullptr;
  std::map<std::string, HttpServletPart*>::iterator it;
  it = d->m_PartsMap.find(name);
  bool found = (it != d->m_PartsMap.end());
  if (found) {
    result = it->second;
  } else {
    throw std::runtime_error("Cannot found part named '" + name + "'");
  }
  return result;
}
}
