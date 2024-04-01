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

#include <string>

#include "cppmicroservices/em/EMConstants.hpp"

namespace cppmicroservices::em::Constants
{
    std::string const BUNDLE = "bundle";
    std::string const BUNDLE_ID = "bundle.id";
    // const std::string BUNDLE_SIGNER = "bundle.signer";
    std::string const BUNDLE_SYMBOLICNAME = "bundle.symbolicName";
    std::string const BUNDLE_VERSION = "bundle.version";
    std::string const DELIVERY_ASYNC_ORDERED = "async.ordered";
    std::string const DELIVERY_ASYNC_UNORDERED = "async.unordered";
    std::string const EVENT = "event";
    // const std::string EVENT_ADMIN_IMPLEMENTATION = "osgi.event";
    // const std::string EVENT_ADMIN_SPECIFICATION_VERSION = "1.4.0";
    std::string const EVENT_DELIVERY = "event.delivery";
    std::string const EVENT_FILTER = "event.filter";
    std::string const EVENT_TOPIC = "event.topics";
    std::string const EXCEPTION = "exception";
    std::string const EXCEPTION_MESSAGE = "exception.message";
    std::string const MESSAGE = "message";
    std::string const SERVICE = "service";
    std::string const SERVICE_ID = "service.id";
    std::string const OBJECTCLASS = "objectClass";
    std::string const SERVICE_PID = "service.pid";
    std::string const TIMESTAMP = "timestamp";
} // namespace cppmicroservices::em::Constants