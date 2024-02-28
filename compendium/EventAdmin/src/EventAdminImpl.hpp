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

#ifndef EVENTADMINIMPL_HPP
#define EVENTADMINIMPL_HPP

#include <cstdint>
#include <future>
#include <mutex>
#include <random>
#include <unordered_map>

#include "EMLogger.hpp"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceTracker.h"
#include "cppmicroservices/ServiceTrackerCustomizer.h"
#include "cppmicroservices/em/EventAdmin.hpp"
#include "cppmicroservices/em/EventHandler.hpp"
#include "cppmicroservices/logservice/LogService.hpp"
#include <cppmicroservices/asyncworkservice/AsyncWorkService.hpp>

#include "EventAdminPrivate.hpp"

using EventHandler = cppmicroservices::service::em::EventHandler;
namespace cppmicroservices::emimpl
{
    /**
     * A wrapper class used for storing the pid of a given ManagedService or ManagedServiceFactory
     * with the service in the ServiceTracker.
     */
    template <typename TrackedServiceType>
    class TrackedServiceWrapper
    {
      public:
        TrackedServiceWrapper(std::string trackedPid,
                              std::unordered_map<std::string, unsigned long> initialChangeCountPerPid,
                              std::shared_ptr<TrackedServiceType> service)
            : pid(std::move(trackedPid))
            , trackedService(std::move(service))
            , lastUpdatedChangeCountPerPid(std::move(initialChangeCountPerPid))
        {
        }

        TrackedServiceWrapper(TrackedServiceWrapper const&) = delete;
        TrackedServiceWrapper& operator=(TrackedServiceWrapper const&) = delete;
        TrackedServiceWrapper(TrackedServiceWrapper&&) = delete;
        TrackedServiceWrapper& operator=(TrackedServiceWrapper&&) = delete;

        explicit
        operator bool() const
        {
            return static_cast<bool>(trackedService);
        }

        std::string
        getPid() noexcept
        {
            return pid;
        }

        std::shared_ptr<TrackedServiceType>
        getTrackedService() noexcept
        {
            return trackedService;
        }

        void
        setLastUpdatedChangeCount(std::string const& pid, unsigned long const& changeCount)
        {
            std::unique_lock<std::mutex> lock(updatedChangeCountMutex);
            lastUpdatedChangeCountPerPid[pid] = changeCount;
        }

        bool
        needsAnUpdateNotification(std::string const& pid, unsigned long const& changeCount)
        {
            std::unique_lock<std::mutex> lock(updatedChangeCountMutex);
            return lastUpdatedChangeCountPerPid[pid] < changeCount;
        }

        void
        removeLastUpdatedChangeCount(std::string const& pid) noexcept
        {
            std::unique_lock<std::mutex> lock(updatedChangeCountMutex);
            (void)lastUpdatedChangeCountPerPid.erase(pid);
        }

      private:
        std::string pid;
        std::shared_ptr<TrackedServiceType> trackedService;
        std::unordered_map<std::string, unsigned long>
            lastUpdatedChangeCountPerPid;   ///< the change count for each pid or factory pid instance
        std::mutex updatedChangeCountMutex; ///< guard read/write access to lastUpdatedChangeCountPerPid
    };

    class EventAdminImpl final
        : public cppmicroservices::service::em::EventAdmin
        , public EventAdminPrivate
        , public cppmicroservices::ServiceTrackerCustomizer<EventHandler, TrackedServiceWrapper<EventHandler>>
    {
      public:
        EventAdminImpl(std::string const& adminName, cppmicroservices::BundleContext& bc);
        void CallHandler(std::shared_ptr<EventHandler>& handler, cppmicroservices::service::em::Event const& evt);
        void PostEvent(cppmicroservices::service::em::Event const& evt) noexcept override;

        void SendEvent(cppmicroservices::service::em::Event const& evt) noexcept override;

        // methods from the cppmicroservices::ServiceTrackerCustomizer interface for EventHandler
        std::shared_ptr<TrackedServiceWrapper<EventHandler>> AddingService(
            ServiceReference<EventHandler> const& reference) override;
        void ModifiedService(ServiceReference<EventHandler> const& reference,
                             std::shared_ptr<TrackedServiceWrapper<EventHandler>> const& service) override;
        void RemovedService(ServiceReference<EventHandler> const& reference,
                            std::shared_ptr<TrackedServiceWrapper<EventHandler>> const& service) override;

      private:
        std::string name;
        cppmicroservices::BundleContext bc;
        std::shared_ptr<cppmicroservices::async::AsyncWorkService> asyncWorkService;
        cppmicroservices::emimpl::EMLogger logger;
    };

} // namespace cppmicroservices::emimpl

#endif
