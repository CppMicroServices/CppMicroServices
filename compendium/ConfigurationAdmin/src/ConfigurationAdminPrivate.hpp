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

#ifndef CONFIGURATIONADMINPRIVATE_HPP
#define CONFIGURATIONADMINPRIVATE_HPP

#include "ThreadpoolSafeFuturePrivate.hpp"
#include "metadata/ConfigurationMetadata.hpp"
#include <cppmicroservices/asyncworkservice/AsyncWorkService.hpp>

#include <cstdint>
#include <future>
#include <vector>

namespace cppmicroservices
{
    namespace cmimpl
    {
        using cppmicroservices::async::AsyncWorkService;
        /**
         * This class is a convenience container for tracking added Configurations.
         */
        struct ConfigurationAddedInfo final
        {
            ConfigurationAddedInfo(std::string pidAdded,
                                   unsigned long changeCountAdded,
                                   std::uintptr_t configurationIdAdded)
                : pid(std::move(pidAdded))
                , changeCount(std::move(changeCountAdded))
                , configurationId(std::move(configurationIdAdded))
            {
            }

            bool
            operator==(ConfigurationAddedInfo const& other) const
            {
                return ((pid == other.pid) && (changeCount == other.changeCount)
                        && (configurationId == other.configurationId));
            }

            std::string pid;
            unsigned long changeCount;
            std::uintptr_t configurationId;
        };

        /**
         * This class declares the internal methods of ConfigurationAdmin.
         */
        class ConfigurationAdminPrivate
        {
          public:
            virtual ~ConfigurationAdminPrivate() = default;

            /**
             * Internal method used by {@code CMBundleExtension} to add new {@code Configuration} objects
             *
             * @param configurationMetadata A vector of {@code ConfigurationMetadata} to create {@code Configuration}
             * objects for
             * @return A vector of ConfigurationAddedInfos, which store the PIDs, changeCounts, and configurationIds of
             * the Configurations that have been created/updated by this method.
             */
            virtual std::vector<ConfigurationAddedInfo> AddConfigurations(
                std::vector<metadata::ConfigurationMetadata> configurationMetadata)
                = 0;

            /**
             * Internal method used by {@code CMBundleExtension} to remove the {@code Configuration} objects that it
             * created
             *
             * @param pidsAndChangeCountsAndIDs A vector of ConfigurationAddedInfos which contain the PIDs to remove,
             *        the changeCount that was associated with adding them, and the IDs of the Configuration objects
             *        when they were added. The combination of these three values is used to prevent the removal of
             *        any  {@code Configuration} which has subsequently been updated by other services since the
             *        {@code CMBundleExtension} added them.
             */
            virtual void RemoveConfigurations(std::vector<ConfigurationAddedInfo> pidsAndChangeCountsAndIDs) = 0;

            /**
             * Internal method used to notify any {@code ManagedService} or {@code ManagedServiceFactory} of an
             * update to a {@code Configuration}. Performs the notifications asynchronously with the latest state
             * of the properties at the time.
             *
             * @param pid The PID of the {@code Configuration} which has been updated
             */
            virtual std::shared_ptr<ThreadpoolSafeFuturePrivate> NotifyConfigurationUpdated(
                std::string const& pid,
                unsigned long const changeCount,
                std::shared_ptr<AsyncWorkService> strand = nullptr)
                = 0;

            /**
             * Internal method used by {@code ConfigurationImpl} to notify any {@code ManagedService} or
             * {@code ManagedServiceFactory} of the removal of a {@code Configuration}. Performs the notifications
             * asynchronously.
             *
             * @param pid The PID of the {@code Configuration} which has been removed.
             * @param configurationId The unique id of the configuration which has been removed. Used to avoid race
             * conditions.
             */
            virtual std::shared_ptr<ThreadpoolSafeFuturePrivate> NotifyConfigurationRemoved(
                std::string const& pid,
                std::uintptr_t configurationId,
                unsigned long changeCount,
                std::shared_ptr<AsyncWorkService> strand = nullptr)
                = 0;
        };
    } // namespace cmimpl
} // namespace cppmicroservices

#endif /* CONFIGURATIONADMINPRIVATE_HPP */
