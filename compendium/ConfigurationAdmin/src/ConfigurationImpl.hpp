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

#ifndef CONFIGURATIONIMPL_HPP
#define CONFIGURATIONIMPL_HPP

#include <mutex>

#include "cppmicroservices/cm/Configuration.hpp"

#include "ConfigurationAdminPrivate.hpp"
#include "ConfigurationPrivate.hpp"

namespace cppmicroservices {
  namespace cmimpl {

    /**
     * This class implements the {@code cppmicroservices::service::cm::Configuration} interface.
     */
    class ConfigurationImpl final : public cppmicroservices::service::cm::Configuration, public ConfigurationPrivate
    {
    public:
      ConfigurationImpl(ConfigurationAdminPrivate* configAdminImpl,
                        std::string pid,
                        std::string factoryPid,
                        AnyMap properties);
      ~ConfigurationImpl() override = default;
      ConfigurationImpl(const ConfigurationImpl&) = delete;
      ConfigurationImpl& operator=(const ConfigurationImpl&) = delete;
      ConfigurationImpl(ConfigurationImpl&&) = delete;
      ConfigurationImpl& operator=(ConfigurationImpl&&) = delete;

      /**
       * Get the PID of this Configuration.
       *
       * See {@code Configuration#GetPid}
       */
      std::string GetPid() const override;

      /**
       * Get the Factory PID which is responsible for this Configuration.
       *
       * See {@code Configuration#GetFactoryPid}
       */
      std::string GetFactoryPid() const override;

      /**
       * Get the properties of this Configuration.
       *
       * See {@code Configuration#GetProperties}
       */
      AnyMap GetProperties() const override;

      /**
       * Update the properties of this Configuration.
       *
       * See {@code Configuration#Update}
       */
      void Update(AnyMap properties) override;

      /**
       * Update the properties of this Configuration if they differ from the current properties.
       *
       * See {@code Configuration#UpdateIfDifferent}
       */
      bool UpdateIfDifferent(AnyMap properties) override;

      /**
       * Remove this Configuration from ConfigurationAdmin.
       *
       * See {@code Configuration#Remove}
       */
      void Remove() override;

      /**
       * Internal method used by {@code ConfigurationAdminImpl} to update the properties without triggering
       * the notification to the corresponding ManagedService / ManagedServiceFactory.
       *
       * See {@code ConfigurationPrivate#UpdateWithoutNotificationIfDifferent}
       */
      std::pair<bool, unsigned long> UpdateWithoutNotificationIfDifferent(AnyMap properties) override;

      /**
       * Internal method used by {@code ConfigurationAdminImpl} to Remove the Configuration without triggering
       * the notification to the corresponding ManagedService / ManagedServiceFactory.
       *
       * See {@code ConfigurationPrivate#RemoveWithoutNotificationIfChangeCountEquals}
       */
      bool RemoveWithoutNotificationIfChangeCountEquals(unsigned long expectedChangeCount) override;

      /**
       * Internal method used by {@code ConfigurationAdminImpl} to invalidate the Configuration.
       *
       * See {@code ConfigurationPrivate#Invalidate}
       */
      void Invalidate() override;

    private:
      std::mutex configAdminMutex;
      ConfigurationAdminPrivate* configAdminImpl;
      mutable std::mutex propertiesMutex;
      std::string pid;
      std::string factoryPid;
      AnyMap properties;
      unsigned long changeCount;
      bool removed;
    };
  } // cmimpl
} // cppmicroservices

#endif // CONFIGURATIONIMPL_HPP
