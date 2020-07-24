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

#ifndef Configuration_hpp
#define Configuration_hpp

#include "cppmicroservices/cm/CMExport.h"

#include <string>

#include "cppmicroservices/AnyMap.h"

namespace cppmicroservices {
  namespace service {
    namespace cm {

      /**
       * The Configuration object (normally obtained as a std::shared_ptr<Configuration>)
       * is the principal means for clients of ConfigurationAdmin to inspect or update the
       * Configuration of a given service or service factory.
       */
      class US_cm_EXPORT Configuration {
      public:
        virtual ~Configuration() noexcept = default;

        /**
         * Get the PID of this Configuration.
         *
         * @throws std::runtime_error if this Configuration object has been Removed
         *
         * @return the PID of this Configuration
         */
        virtual std::string GetPid() const = 0;

        /**
         * Get the Factory PID which is responsible for this Configuration. If this
         * Configuration does not belong to any Factory, returns an empty string.
         *
         * @throws std::runtime_error if this Configuration object has been Removed
         *
         * @return the Factory PID associated with this Configuration, if applicable
         */
        virtual std::string GetFactoryPid() const = 0;

        /**
         * Get the properties of this Configuration. Returns a copy.
         *
         * @throws std::runtime_error if this Configuration object has been Removed
         *
         * @return the properties of this Configuration
         */
        virtual AnyMap GetProperties() const = 0;

        /**
         * Update the properties of this Configuration. Invoking this method will trigger the
         * ConfigurationAdmin impl to push the updated properties to any ManagedService /
         * ManagedServiceFactory which has a matching PID / Factory PID.
         *
         * If the properties are empty, the Configuration will not be removed, but instead
         * updated with an empty properties map.
         *
         * @throws std::runtime_error if this Configuration object has been Removed
         *
         * @param properties The properties to update this Configuration with.
         */
        virtual void Update(AnyMap properties = AnyMap{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS}) = 0;

        /**
         * Update the properties of this Configuration if they differ from the current properties.
         * Invoking this method will trigger the ConfigurationAdmin impl to push the updated
         * properties to any ManagedService / ManagedServiceFactory which has a matching PID /
         * Factory PID, but only if the properties differ from the current properties. It will
         * return true in this case, and false otherwise.
         *
         * If the properties are empty, the Configuration will not be removed, but instead
         * updated with an empty properties map, unless it already had empty properties.
         *
         * @throws std::runtime_error if this Configuration object has been Removed
         *
         * @param properties The properties to update this Configuration with (if they differ)
         * @return boolean indicating whether the properties were updated or not.
         */
        virtual bool UpdateIfDifferent(AnyMap properties = AnyMap{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS}) = 0;

        /**
         * Remove this Configuration from ConfigurationAdmin. This will trigger the ConfigurationAdmin
         * implementation to update any corresponding ManagedService with an empty AnyMap. Any
         * corresponding ManagedServiceFactory will have its Removed method invoked with the
         * corresponding PID.
         *
         * @throws std::runtime_error if this Configuration object has been Removed already
         */
        virtual void Remove() = 0;
      };
    }
  }
}
#endif // Configuration_hpp
