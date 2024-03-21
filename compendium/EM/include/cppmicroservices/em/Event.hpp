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

#ifndef CppMicroServices_EM_Event_hpp
#define CppMicroServices_EM_Event_hpp

#include "cppmicroservices/em/EMExport.h"

#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/LDAPFilter.h"

namespace cppmicroservices::service::em
{

    using EventProperties = std::unordered_map<std::string, Any>;

    /**
     * \defgroup gr_emevent Event
     * \brief Groups cppmicroservices::service::em::Event related symbols.
     */

    /**
     * \ingroup gr_emevent
     *
     * An implementation of the OSGi EventAdmin Event specification.
     *
     * The Event is the event object that is both published and subscribed to. Any published events
     * are of type Event. The event can contain various properties that allow consumers to filter out
     * specific events and perform certain operations when specific events are received.
     *
     * Event properties are read-only and only allow for consumers to observe them. This is to prevent
     * event properties being changed by arbitrary pieces of code, ensuring the originality of the
     * published event.
     *
     * Interactions with events occurs through the EMEvent object directly. See
     * \c cppmicroservices::em::Constants for information regarding what properties an event can have.
     */
    class US_em_EXPORT Event
    {
      public:
        Event() = delete;
        Event(Event&&) = delete;
        Event(Event const&&) = delete;
        Event& operator=(Event const&) = delete;
        Event& operator=(Event&&) = delete;

        Event(Event const&) = default;

        virtual ~Event() = default;

        /**
         * @brief Construct a new Event object with the provided topic and event properties.
         *
         * Topics take the following form:
         *   topic-description := '*' | topic ( '/\*' )?
         *   topic := token ( '/' token )*
         *   token := [A-Za-z0-9_.]+
         *
         * @param topic The topic of the event.
         * @param properties The event's properties. This is immutable and a copy of the original
         * properties
         *
         * @throws std::logic_error If the topic format is invalid
         */
        Event(std::string const& topic, EventProperties const properties = EventProperties());

        /**
         * @brief Compares whether or not two Events are equal to each other
         *
         * Events are euqal iff their topics match and their properties exactly match.
         *
         * @param other The other Event to compare against
         * @return true if the events are equal
         * @return false otherwise
         */
        bool operator==(Event const& other) const;

        /**
         * @brief Compares whether or not two Events are not equal to each other
         *
         * Events are not equal if their topics do not match and/or their properties do not exactly
         * match.
         *
         * @param other The other Event to compare against
         * @return true if the events are not equal
         * @return false otherwise
         */
        bool operator!=(Event const& other) const;

        /**
         * @brief Returns whether or not the provided property is specified in the event properties.
         *
         * @param propName The peropty to search for
         * @return bool true if the property is defined, false otherwise
         */
        [[nodiscard]] bool ContainsProperty(std::string const& propName) const;

        /**
         * @brief Returns the property associated with the specified property name. If the specified
         * property does not exist, an empty <code>cppmicroservices::Any</code> is returned
         *
         * @param propName The name of the property to get
         * @return Any The value of the property if found, an empty Any if not found
         */
        [[nodiscard]] Any const GetProperty(std::string const& propName) const;

        /**
         * @brief Returns the properties map containing the properties of the event.
         *
         * @return The properties
         */
        [[nodiscard]] AnyMap const GetProperties() const;

        /**
         * @brief Returns a vector of all the specified property names.
         *
         * @return std::vector<std::string> The specified property names
         */
        [[nodiscard]] std::vector<std::string> GetPropertyNames() const;

        /**
         * @brief Returns the topic of the EMEvent.
         *
         * @return std::string The topic
         */
        [[nodiscard]] std::string GetTopic() const;

        /**
         * @brief Returns whether or not the Event matches against the provided \c LDAPFilter .
         *
         * @param filter The filter to match the event against
         * @return bool true if the specified filter matches this event, false otherwise
         */
        [[nodiscard]] bool Matches(LDAPFilter const& filter) const;

      private:
        std::string const topic;
        AnyMap const properties;
    };
} // namespace cppmicroservices::service::em

#endif
