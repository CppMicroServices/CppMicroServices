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
#include <cppmicroservices/ThreadpoolSafeFuture.h>

#include "ConfigurationAdminPrivate.hpp"
#include "ConfigurationPrivate.hpp"
#include "ThreadpoolSafeFuturePrivate.hpp"

namespace cppmicroservices
{
    namespace cmimpl
    {
        using cppmicroservices::async::AsyncWorkService;
        /**
         * This class implements the {@code cppmicroservices::service::cm::Configuration} interface.
         */
        class ConfigurationImpl final
            : public cppmicroservices::service::cm::Configuration
            , public ConfigurationPrivate
        {
          public:
            ConfigurationImpl(ConfigurationAdminPrivate* configAdminImpl,
                              std::string pid,
                              std::string factoryPid,
                              AnyMap properties,
                              std::shared_ptr<AsyncWorkService> aws,
                              unsigned long const iCount,
                              unsigned long const cCount = 0);
            ~ConfigurationImpl() override = default;
            ConfigurationImpl(ConfigurationImpl const&) = delete;
            ConfigurationImpl& operator=(ConfigurationImpl const&) = delete;
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
             * Get the value of the changeCount
             *
             * See {@code Configuration#GetChangeCount}
             */
            unsigned long GetChangeCount() const override;

            /**
             * Update the properties of this Configuration.
             *
             * @return a std::shared_future<void>
             * @note not safe to wait on future from within the AsyncWorkService
             *
             * See {@code Configuration#Update}
             */
            std::shared_future<void> Update(AnyMap properties) override;

            /**
             * Update the properties of this Configuration and return
             *
             * @return A std::shared_ptr<ThreadpoolSafeFuture>, safe to wait on from within the
             * AsyncWorkService used by the Framework
             *
             * See {@code Configuration#Update}
             */
            std::shared_ptr<ThreadpoolSafeFuture> SafeUpdate(AnyMap newProperties) override;
            std::shared_ptr<ThreadpoolSafeFuturePrivate> SafeUpdateImpl(AnyMap newProperties);

            /**
             * Update the properties of this Configuration if they differ from the current properties.
             *
             * @return a std::shared_future<void> and whether the config was updated
             * @note not safe to wait on future from within the AsyncWorkService
             *
             * See {@code Configuration#UpdateIfDifferent}
             */
            std::pair<bool, std::shared_future<void>> UpdateIfDifferent(AnyMap properties) override;

            /**
             * Update the properties of this Configuration if they differ from the current properties.
             *
             * @return A pair<bool, std::shared_ptr<ThreadpoolSafeFuture>>, safe to wait on from within the
             * AsyncWorkService used by the Framework
             *
             * See {@code Configuration#UpdateIfDifferent}
             */
            std::pair<bool, std::shared_ptr<ThreadpoolSafeFuture>> SafeUpdateIfDifferent(AnyMap properties) override;
            std::pair<bool, std::shared_ptr<ThreadpoolSafeFuturePrivate>> SafeUpdateIfDifferentImpl(AnyMap properties);

            /**
             * Remove this Configuration from ConfigurationAdmin.
             *
             * @return A std::shared_future<void>,
             * @note not safe to wait on future from within the AsyncWorkService
             *
             * See {@code Configuration#Remove}
             */
            std::shared_future<void> Remove() override;

            /**
             * Retrieve the instance of a configuration sharing this config-id
             *
             * @return unsigned long
             *
             * See {@code Configuration#GetInstanceCount}
             */
            unsigned long GetInstanceCount() override;

            /**
             * Remove this Configuration from ConfigurationAdmin.
             *
             * @return A std::shared_ptr<ThreadpoolSafeFuture>, safe to wait on from within the
             * AsyncWorkService used by the Framework
             *
             * See {@code Configuration#Remove}
             */
            std::shared_ptr<ThreadpoolSafeFuture> SafeRemove() override;
            std::shared_ptr<ThreadpoolSafeFuturePrivate> SafeRemoveImpl();

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

            /** Internal method used by {@code ConfigurationAdminImpl} to determine if a configuration
             * object has been updated.
             *
             * See {@code ConfigurationPrivate#HasBeenUpdatedAtLeastOnce}
             */
            bool
            HasBeenUpdatedAtLeastOnce() override
            {
                std::lock_guard<std::mutex> lk { propertiesMutex };
                if (changeCount > 0)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }

          private:
            std::shared_ptr<AsyncWorkService> strand;
            std::mutex configAdminMutex;
            ConfigurationAdminPrivate* configAdminImpl;
            mutable std::mutex propertiesMutex;
            std::string pid;
            std::string factoryPid;
            AnyMap properties;
            unsigned long changeCount;
            bool removed;
            unsigned long instance;
        };
    } // namespace cmimpl
} // namespace cppmicroservices

#endif // CONFIGURATIONIMPL_HPP
