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

#include "cppmicroservices/httpservice/HttpConstants.h"

#include <string>

namespace cppmicroservices {

std::string HttpConstants::HTTP_SERVICE_ENDPOINT_ATTRIBUTE()
{
  static std::string s = "org.cppmicroservices.http.endpoint";
  return s;
}

std::string HttpConstants::HTTP_WHITEBOARD_CONTEXT_NAME()
{
  static std::string s = "org.cppmicroservices.http.whiteboard.context.name";
  return s;
}

std::string HttpConstants::HTTP_WHITEBOARD_CONTEXT_SELECT()
{
  static std::string s = "org.cppmicroservices.http.whiteboard.context.select";
  return s;
}

std::string HttpConstants::HTTP_WHITEBOARD_CONTEXT_SHARED()
{
  static std::string s = "org.cppmicroservices.http.whiteboard.context.shared";
  return s;
}

std::string HttpConstants::HTTP_WHITEBOARD_RESOURCE_PREFIX()
{
  static std::string s = "org.cppmicroservices.http.whiteboard.resource.prefix";
  return s;
}

std::string HttpConstants::HTTP_WHITEBOARD_SERVLET_NAME()
{
  static std::string s = "org.cppmicroservices.http.whiteboard.servlet.name";
  return s;
}

std::string HttpConstants::HTTP_WHITEBOARD_SERVLET_PATTERN()
{
  static std::string s = "org.cppmicroservices.http.whiteboard.servlet.pattern";
  return s;
}

std::string HttpConstants::HTTP_WHITEBOARD_TARGET()
{
  static std::string s = "org.cppmicroservices.http.whiteboard.target";
  return s;
}
}
