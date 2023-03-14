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

#ifndef CONFIGURATIONPRIVATE_HPP
#define CONFIGURATIONPRIVATE_HPP

#include <mutex>

#include "ConfigurationAdminPrivate.hpp"

namespace cppmicroservices
{
    namespace cmimpl
    {

        /**
         * This class declares the internal methods of Configuration
         */
        class ConfigurationPrivate
        {
          public:
            virtual ~ConfigurationPrivate() = default;

            /**
             * Internal method used by {@code ConfigurationAdminImpl} to update the properties without triggering
             * the notification to the corresponding ManagedService / ManagedServiceFactory. That will be taken
             * care of by {@code ConfigurationAdminImpl} instead.
             *
             * @param properties The properties to update this Configuration with (if they differ)
             * @return boolean indicating whether this Configuration has been updated or not, and the value of
             */
            virtual std::pair<bool, unsigned long> UpdateWithoutNotificationIfDifferent(AnyMap properties) = 0;

            /**
             * Internal method used by {@code ConfigurationAdminImpl} to Remove the Configuration without triggering
             * the notification to the corresponding ManagedService / ManagedServiceFactory. That will be taken
             * care of by {@code ConfigurationAdminImpl} instead.
             *
             * @param expectedChangeCount the expected value of the changeCount. The Configuration will only be
             *        removed if this equals the actual change count.
             * @return boolean indicating whether the change count was correct, and therefore whether this
             *         configuration has been Removed or not.
             */
            virtual bool RemoveWithoutNotificationIfChangeCountEquals(unsigned long expectedChangeCount) = 0;

            /**
             * Internal method used by {@code ConfigurationAdminImpl} to invalidate the Configuration. Used when
             * ConfigurationAdminImpl is shutting down or when it is deleting a Configuration.
             */
            virtual void Invalidate() = 0;

            /** Internal method used by {@code ConfigurationAdminImpl} to determine if a configuration
             * object has been updated.
             *
             * See {@code ConfigurationPrivate#Invalidate}
             */
            virtual bool HasBeenUpdatedAtLeastOnce() = 0;
        };
    } // namespace cmimpl
} // namespace cppmicroservices

#endif // CONFIGURATIONPRIVATE_HPP
