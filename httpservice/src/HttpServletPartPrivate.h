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

#ifndef CPPMICROSERVICES_HTTPSERVLETPARTPRIVATE_H
#define CPPMICROSERVICES_HTTPSERVLETPARTPRIVATE_H

#include "cppmicroservices/SharedData.h"

#include <map>
#include <memory>
#include <string>

class CivetServer;
struct mg_connection;

namespace cppmicroservices {

class Any;
class ServletContext;

struct HttpServletPartPrivate : public SharedData
{
  HttpServletPartPrivate(const std::shared_ptr<ServletContext>& servletContext,
                         CivetServer* server,
                         mg_connection* conn);

  virtual ~HttpServletPartPrivate() {}

  virtual std::istream* GetInputStream() const = 0;

  virtual long long GetSize() const = 0;

  virtual std::string GetSubmittedFileName() const = 0;

  virtual void Delete() = 0;

  const std::shared_ptr<ServletContext> m_ServletContext;
  CivetServer* const m_Server;
  struct mg_connection* const m_Connection;

  std::string m_ContentType;
  std::string m_Name;
  typedef std::map<std::string, Any> AttributeMapType;
  AttributeMapType m_Headers;
};
}

#endif // CPPMICROSERVICES_HTTPSERVLETPARTPRIVATE_H
