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

#ifndef CPPMICROSERVICES_HTTPCONSTANTS_H
#define CPPMICROSERVICES_HTTPCONSTANTS_H

#include <string>

#include "cppmicroservices/httpservice/HttpServiceExport.h"

namespace cppmicroservices {

struct US_HttpService_EXPORT HttpConstants
{
  static std::string
  HTTP_SERVICE_ENDPOINT_ATTRIBUTE(); // "org.cppmicroservices.http.endpoint"
  static std::string
  HTTP_WHITEBOARD_CONTEXT_NAME(); // "org.cppmicroservices.http.whiteboard.context.name"
  static std::string
  HTTP_WHITEBOARD_CONTEXT_SELECT(); // "org.cppmicroservices.http.whiteboard.context.select"
  static std::string
  HTTP_WHITEBOARD_CONTEXT_SHARED(); // "org.cppmicroservices.http.whiteboard.context.shared"
  static std::string
  HTTP_WHITEBOARD_RESOURCE_PREFIX(); // "org.cppmicroservices.http.whiteboard.resource.prefix"
  static std::string
  HTTP_WHITEBOARD_SERVLET_NAME(); // "org.cppmicroservices.http.whiteboard.servlet.name"
  static std::string
  HTTP_WHITEBOARD_SERVLET_PATTERN(); // "org.cppmicroservices.http.whiteboard.servlet.pattern"
  static std::string
  HTTP_WHITEBOARD_TARGET(); // "org.cppmicroservices.http.whiteboard.target"
};
}

#endif // CPPMICROSERVICES_HTTPCONSTANTS_H
