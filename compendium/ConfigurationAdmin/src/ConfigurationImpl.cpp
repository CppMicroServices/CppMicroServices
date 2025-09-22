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

#include "ConfigurationImpl.hpp"
#include <cassert>
#include <future>
#include <sstream>

namespace
{
    constexpr auto REMOVED_EXCEPTION_MESSAGE = "This Configuration has been Removed";
}

namespace cppmicroservices
{
    namespace cmimpl
    {

        ConfigurationImpl::ConfigurationImpl(ConfigurationAdminPrivate* configAdmin,
                                             std::string thePid,
                                             std::string theFactoryPid,
                                             AnyMap props,
                                             std::shared_ptr<AsyncWorkService> aws,
                                             unsigned long const iCount,
                                             unsigned long const cCount)
            : strand(aws->createStrand())
            , configAdminImpl(configAdmin)
            , pid(std::move(thePid))
            , factoryPid(std::move(theFactoryPid))
            , properties(std::move(props))
            , changeCount { cCount }
            , removed { false }
            , instance { iCount }
        {
            assert(configAdminImpl != nullptr && "Invalid ConfigurationAdminPrivate pointer");
            // constructing a configuration object with properties is the equivalent
            // of a Create and an Update operation.
            if ((properties.size() > 0) && (changeCount == 0u))
            {
                changeCount++;
            }
        }

        std::string
        ConfigurationImpl::GetPid() const
        {
            std::lock_guard<std::mutex> lk { propertiesMutex };
            if (removed)
            {
                throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
            }
            return pid;
        }

        std::string
        ConfigurationImpl::GetFactoryPid() const
        {
            std::lock_guard<std::mutex> lk { propertiesMutex };
            if (removed)
            {
                throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
            }
            return factoryPid;
        }

        AnyMap
        ConfigurationImpl::GetProperties() const
        {
            std::lock_guard<std::mutex> lk { propertiesMutex };
            if (removed)
            {
                throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
            }
            return properties;
        }

        unsigned long
        ConfigurationImpl::GetChangeCount() const
        {
            std::lock_guard<std::mutex> lk { propertiesMutex };
            if (removed)
            {
                throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
            }
            return changeCount;
        }

        std::shared_future<void>
        ConfigurationImpl::Update(AnyMap newProperties)
        {
            return SafeUpdateImpl(newProperties)->retrieveFuture();
        }

        std::shared_ptr<ThreadpoolSafeFuture>
        ConfigurationImpl::SafeUpdate(AnyMap newProperties)
        {
            return SafeUpdateImpl(newProperties);
        }

        std::shared_ptr<ThreadpoolSafeFuturePrivate>
        ConfigurationImpl::SafeUpdateImpl(AnyMap newProperties)
        {
            {
                std::lock_guard<std::mutex> lk { propertiesMutex };
                if (removed)
                {
                    throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
                }
                properties = std::move(newProperties);
                ++changeCount;
            }
            std::lock_guard<std::mutex> lk { configAdminMutex };
            if (configAdminImpl)
            {
                return configAdminImpl->NotifyConfigurationUpdated(pid, changeCount, strand);
            }
            std::promise<void> ready;
            std::shared_future<void> fut = ready.get_future();
            ready.set_value();
            return std::make_shared<ThreadpoolSafeFuturePrivate>(fut);
        }

        std::pair<bool, std::shared_future<void>>
        ConfigurationImpl::UpdateIfDifferent(AnyMap newProperties)
        {
            auto tup = SafeUpdateIfDifferentImpl(newProperties);

            return std::make_pair(std::get<0>(tup), std::get<1>(tup)->retrieveFuture());
        }

        std::pair<bool, std::shared_ptr<ThreadpoolSafeFuture>>
        ConfigurationImpl::SafeUpdateIfDifferent(AnyMap newProperties)
        {
            return SafeUpdateIfDifferentImpl(newProperties);
        }

        std::pair<bool, std::shared_ptr<ThreadpoolSafeFuturePrivate>>
        ConfigurationImpl::SafeUpdateIfDifferentImpl(AnyMap newProperties)
        {
            std::promise<void> ready;
            std::shared_future<void> fut = ready.get_future();
            auto const updated = UpdateWithoutNotificationIfDifferent(std::move(newProperties));
            if (!updated.first)
            {
                ready.set_value();
                return std::pair<bool, std::shared_ptr<ThreadpoolSafeFuturePrivate>>(
                    updated.first,
                    std::make_shared<ThreadpoolSafeFuturePrivate>(fut));
            }
            std::lock_guard<std::mutex> lk { configAdminMutex };
            if (configAdminImpl)
            {
                auto fut = configAdminImpl->NotifyConfigurationUpdated(pid, changeCount, strand);
                return std::pair<bool, std::shared_ptr<ThreadpoolSafeFuturePrivate>>(true, fut);
            }

            ready.set_value();
            return std::pair<bool, std::shared_ptr<ThreadpoolSafeFuturePrivate>>(
                true,
                std::make_shared<ThreadpoolSafeFuturePrivate>(fut));
        }

        std::shared_future<void>
        ConfigurationImpl::Remove()
        {
            return SafeRemoveImpl()->retrieveFuture();
        }
        std::shared_ptr<ThreadpoolSafeFuture>
        ConfigurationImpl::SafeRemove()
        {
            return SafeRemoveImpl();
        }

        std::shared_ptr<ThreadpoolSafeFuturePrivate>
        ConfigurationImpl::SafeRemoveImpl()
        {
            {
                std::lock_guard<std::mutex> lk { propertiesMutex };
                if (removed)
                {
                    throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
                }
                removed = true;
            }
            std::lock_guard<std::mutex> lk { configAdminMutex };
            if (configAdminImpl)
            {
                auto fut = configAdminImpl->NotifyConfigurationRemoved(pid,
                                                                       reinterpret_cast<std::uintptr_t>(this),
                                                                       changeCount,
                                                                       strand);
                configAdminImpl = nullptr;
                return fut;
            }
            std::promise<void> ready;
            std::shared_future<void> fut = ready.get_future();
            ready.set_value();
            return std::make_shared<ThreadpoolSafeFuturePrivate>(fut);
        }
        std::pair<bool, unsigned long>
        ConfigurationImpl::UpdateWithoutNotificationIfDifferent(AnyMap newProperties)
        {
            std::lock_guard<std::mutex> lk { propertiesMutex };
            if (removed)
            {
                throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
            }
            if (properties == newProperties)
            {
                return std::pair<bool, unsigned long> { false, 0u };
            }
            properties = std::move(newProperties);
            return std::pair<bool, unsigned long> { true, ++changeCount };
        }

        bool
        ConfigurationImpl::RemoveWithoutNotificationIfChangeCountEquals(unsigned long expectedChangeCount)
        {
            std::lock_guard<std::mutex> lk { propertiesMutex };
            if (removed)
            {
                throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
            }
            if (expectedChangeCount == changeCount)
            {
                removed = true;
                return true;
            }
            return false;
        }

        void
        ConfigurationImpl::Invalidate()
        {
            std::lock_guard<std::mutex> lk { configAdminMutex };
            configAdminImpl = nullptr;
        }

        unsigned long
        ConfigurationImpl::GetInstanceCount()
        {
            return instance;
        }

    } // namespace cmimpl
} // namespace cppmicroservices
