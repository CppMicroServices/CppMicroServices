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

#ifndef CPPMICROSERVICES_MEMORY_HTTPSERVLETPARTPRIVATE_H
#define CPPMICROSERVICES_MEMORY_HTTPSERVLETPARTPRIVATE_H

#include "HttpServletPartPrivate.h"

namespace cppmicroservices {

class Any;
class ServletContext;

struct MemoryHttpServletPartPrivate : public HttpServletPartPrivate
{

  MemoryHttpServletPartPrivate(
    const std::shared_ptr<ServletContext>& servletContext,
    CivetServer* server,
    mg_connection* conn);

  std::istream* GetInputStream() const override;

  long long GetSize() const override;

  std::string GetSubmittedFileName() const override;

  void Delete() override;

  std::string m_Value;
};
}

#endif // CPPMICROSERVICES_MEMORY_HTTPSERVLETPARTPRIVATE_H
