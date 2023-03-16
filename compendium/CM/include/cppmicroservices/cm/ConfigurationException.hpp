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

#ifndef CppMicroServices_CM_ConfigurationException_hpp
#define CppMicroServices_CM_ConfigurationException_hpp

#include <stdexcept>
#include <string>

namespace
{
    std::string
    makeMessage(const std::string reason, const std::string property)
    {
        return "ConfigurationException due to " + reason
               + (!property.empty() ? (" (with property: " + property + ")") : "");
    }
} // namespace

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {

            /**
             \defgroup gr_configurationexception ConfigurationException
             \brief Groups ConfigurationException class related symbols.
             */

            /**
             * \ingroup gr_configurationexception
             *
             * Exception which may be thrown by ManagedService or ManagedServiceFactory
             * subclasses to indicate to the ConfigurationAdmin implementation that the
             * Configuration they have been given is invalid. The ConfigurationAdmin
             * implementation will log the exception with as much detail as it can. The
             * ConfigurationException class is not final to ensure it can be used with
             * std::throw_with_nested - the ConfigurationAdmin implementation will attempt
             * to print the details of any nested exceptions as well.
             */
            class ConfigurationException : public std::runtime_error
            {
              public:
                /**
                 * Construct a new ConfigurationException with the specified reason and
                 * optionally specify which property caused the error.
                 *
                 * @param rsn The reason for the exception.
                 * @param prop The property which caused the excpetion, if applicable.
                 */
                ConfigurationException(std::string rsn, std::string prop = "")
                    : std::runtime_error(makeMessage(rsn, prop).c_str())
                    , reason(std::move(rsn))
                    , property(std::move(prop))
                {
                }

                /**
                 * Returns the reason for this exception.
                 *
                 * @return The reason for this exception.
                 */
                std::string
                GetReason() const
                {
                    return reason;
                }

                /**
                 * Returns the property which was resonsible for this exception being throws,
                 * if applicable. Could be empty.
                 *
                 * @return The property which caused this exception.
                 */
                std::string
                GetProperty() const
                {
                    return property;
                }

                virtual ~ConfigurationException() noexcept {}

              private:
                const std::string reason;
                const std::string property;
            };
        } // namespace cm
    }     // namespace service
} // namespace cppmicroservices

#endif /* ConfigurationException_hpp */
