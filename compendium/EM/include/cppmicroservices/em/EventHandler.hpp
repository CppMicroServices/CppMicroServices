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

#ifndef CppMicroServices_EM_EventHandler_hpp
#define CppMicroServices_EM_EventHandler_hpp

#include "cppmicroservices/em/EMEvent.hpp"

namespace cppmicroservices {
namespace service {
namespace em {
/**
 * @brief An implementation of the OSGi EventHandler specification.
 *
 * EventHandlers are registered services that can subscribe to published events. An EventHandler can
 * specify two criteron to sufficiently select what types of events that will be handled. Upon an
 * event being published which matches the filter criteria, the HandleEvent() function will be
 * invoked.
 */
class EventHandler
{
public:
  /**
   * @brief The function that \c EventAdmin service calls to notify the listener that an event
   * matching their desired criteria (topic + (optionally filter)) was sent.
   * 
   * Any exceptions thrown by the implementation of this function must be caught and handled
   * by the \c EventAdmin service. If a \c LogService is present, the exception will be logged.
   * 
   * @param evt The received event
   */
  virtual void HandleEvent(
    const cppmicroservices::service::em::EMEvent& evt) = 0;
};
}
}
}

#endif
