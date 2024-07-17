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

#ifndef CppMicroServices_EM_EventAdmin_hpp
#define CppMicroServices_EM_EventAdmin_hpp

#include "cppmicroservices/em/Event.hpp"

namespace cppmicroservices::service::em
{

    /**
     * \defgroup gr_emeventadmin EventAdmin
     * \brief Groups cppmicroservices::service::em::EventAdmin related symbols.
     *
     */

    /**
     * \ingroup gr_emeventadmin
     * An implementation of the OSGi EventAdmin compendium service.
     *
     * The EventAdmin service is responsible for delivering published messages to EventHandlers who
     * subscribe to specific events matching a given criteria.
     *
     * There are two methods for publishing an event: \c PostEvent and \c SendEvent . PostEvent() will
     * send a published event asynchronously and returns back to the caller before delivery has
     * completed while SendEvent() will only return once the event delivery has completed.
     */
    class EventAdmin
    {
      public:
        virtual ~EventAdmin() = default;
        EventAdmin(EventAdmin&&) = default;
        EventAdmin(EventAdmin&) = default;
        EventAdmin& operator=(EventAdmin const&) = default;
        EventAdmin& operator=(EventAdmin&&) = default;
        /**
         * @brief Initiates the asynchronous delivery of an event. If the \c DELIVERY_ASYNC_UNORDERED
         * event property is specified, it will not be guaranteed that the event will be delivered in the
         * order that it was received.
         *
         * This function will return to the caller before event delivery has completed.
         *
         * @param evt The event to deliver to all handlers which subscribe to the topic of the event
         */
        virtual void PostEvent(Event const& evt) noexcept = 0;

        /**
         * @brief Initiates the synchronous delivery of an event.
         *
         * This function will not return to the caller until event delivery has completed.
         *
         * @param evt The event to delvier to all handlers which subscribe to the topic of the event
         */
        virtual void SendEvent(Event const& evt) noexcept = 0;
    };
} // namespace cppmicroservices::service::em

#endif
