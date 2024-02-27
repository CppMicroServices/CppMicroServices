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

#include "EventAdminImpl.hpp"

namespace cppmicroservices::emimpl
{
    using cppmicroservices::logservice::SeverityLevel;

    EventAdminImpl::EventAdminImpl(std::string const& adminName, cppmicroservices::BundleContext& bc)
        : name(adminName)
        , bc(bc)
        , logger(bc)
    {
        if (auto asyncWSSRef = bc.GetServiceReference<cppmicroservices::async::AsyncWorkService>(); asyncWSSRef)
        {
            asyncWorkService = bc.GetService<cppmicroservices::async::AsyncWorkService>(asyncWSSRef);
        }
    }

    void
    EventAdminImpl::CallHandler(std::shared_ptr<cppmicroservices::service::em::EventHandler>& handler,
                                cppmicroservices::service::em::Event const& evt)
    {
        try
        {
            handler->HandleEvent(evt);
        }
        catch (std::exception const& e)
        {
            logger.Log(SeverityLevel::LOG_ERROR,
                       "Exception thrown by ManagedServiceFactory with figuration Exception: " + std::string(e.what()));
            return;
        }
    }

    void
    EventAdminImpl::PostEvent(cppmicroservices::service::em::Event const& evt) noexcept
    {
        auto topic = evt.GetTopic();
        auto refs = bc.GetServiceReferences<cppmicroservices::service::em::EventHandler>();
        for (auto const& sr : refs)
        {
            auto obj = bc.GetService<cppmicroservices::service::em::EventHandler>(sr);

            std::packaged_task<void()> post_task([evt, obj, this]() mutable { CallHandler(obj, evt); });

            asyncWorkService->post(std::move(post_task));
        }
    }

    void
    EventAdminImpl::SendEvent(cppmicroservices::service::em::Event const& evt) noexcept
    {
        auto topic = evt.GetTopic();
        auto refs = bc.GetServiceReferences<cppmicroservices::service::em::EventHandler>();
        for (auto const& sr : refs)
        {
            auto obj = bc.GetService<cppmicroservices::service::em::EventHandler>(sr);
            CallHandler(obj, evt);
        }
    }

} // namespace cppmicroservices::emimpl