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

#ifndef USHTTPSERVLETREQUEST_P_H
#define USHTTPSERVLETREQUEST_P_H

#include <usSharedData.h>

#include <string>
#include <map>

class CivetServer;
struct mg_connection;

US_BEGIN_NAMESPACE

class Any;
class ServletContext;

struct HttpServletRequestPrivate : public SharedData
{
  HttpServletRequestPrivate(ServletContext* servletContext, CivetServer* server, mg_connection* conn);

  ServletContext* const m_ServletContext;
  CivetServer* const m_Server;
  struct mg_connection* const m_Connection;

  std::string m_Scheme;
  std::string m_ServerName;
  std::string m_ServerPort;
  std::string m_Uri;
  std::string m_ContextPath;
  std::string m_ServletPath;
  std::string m_PathInfo;
  std::string m_QueryString;
  std::string m_Url;

  typedef std::map<std::string, Any> AttributeMapType;
  AttributeMapType m_Attributes;
};

US_END_NAMESPACE

#endif // USHTTPSERVLETREQUEST_P_H
