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

#ifndef SERVICECOMPONENTRUNTIMEIMPL_HPP
#define SERVICECOMPONENTRUNTIMEIMPL_HPP

#if defined(USING_GTEST)
#    include "gtest/gtest_prod.h"
#else
#    define FRIEND_TEST(x, y)
#endif
#include "ComponentRegistry.hpp"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/logservice/LogService.hpp"
#include "cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp"

using cppmicroservices::service::component::runtime::ServiceComponentRuntime;
using cppmicroservices::service::component::runtime::dto::ComponentConfigurationDTO;
using cppmicroservices::service::component::runtime::dto::ComponentDescriptionDTO;
using cppmicroservices::service::component::runtime::dto::SatisfiedReferenceDTO;
using cppmicroservices::service::component::runtime::dto::UnsatisfiedReferenceDTO;

namespace cppmicroservices
{
    namespace scrimpl
    {
        class ReferenceManager;
        class ComponentManager;
        class ComponentConfiguration;
        /**
         * This class implements the {@code ServiceComponentRuntime} interface.
         */
        class ServiceComponentRuntimeImpl final : public ServiceComponentRuntime
        {
          public:
            ServiceComponentRuntimeImpl(cppmicroservices::BundleContext context,
                                        std::shared_ptr<ComponentRegistry> componentRegistry,
                                        std::shared_ptr<cppmicroservices::logservice::LogService> logger);
            ~ServiceComponentRuntimeImpl() override = default;
            ServiceComponentRuntimeImpl(ServiceComponentRuntimeImpl const&) = delete;
            ServiceComponentRuntimeImpl& operator=(ServiceComponentRuntimeImpl const&) = delete;
            ServiceComponentRuntimeImpl(ServiceComponentRuntimeImpl&&) = delete;
            ServiceComponentRuntimeImpl& operator=(ServiceComponentRuntimeImpl&&) = delete;

            /**
             * This method returns the component descriptions from the list of bundles provided.
             * See {@code ServiceComponentRuntime#GetComponentDescriptionDTOs}
             */
            std::vector<ComponentDescriptionDTO> GetComponentDescriptionDTOs(
                std::vector<cppmicroservices::Bundle> const& bundles) const override;

            /**
             * This method returns the component description for the component with the given
             * name in the given {@code Bundle}
             * See {@code ServiceComponentRuntime#GetComponentDescriptionDTO}
             */
            ComponentDescriptionDTO GetComponentDescriptionDTO(cppmicroservices::Bundle const& bundle,
                                                               std::string const& name) const override;

            /**
             * This method returns a vector of DTO objects representing component configurations
             * for a component See {@code ServiceComponentRuntime#GetComponentConfigurationDTOs}
             *
             * @throw std::out_of_range exception if the provided ComponentDescriptionDTO does not match
             * any of the known components in the runtime.
             */
            std::vector<ComponentConfigurationDTO> GetComponentConfigurationDTOs(
                ComponentDescriptionDTO const& description) const override;

            /**
             * This method returns true if a component represented by the {@code ComponentDescriptionDTO}
             * is currently enabled, false otherwise
             * See {@code ServiceComponentRuntime#IsComponentEnabled}
             *
             * @throw std::out_of_range exception if the provided ComponentDescriptionDTO does not match
             * any of the known components in the runtime.
             */

            bool IsComponentEnabled(ComponentDescriptionDTO const& description) const override;

            /**
             * This method changes the state of the component represented by the {@code ComponentDescriptionDTO}
             * to \c ENABLED and returns a future object
             * This is an asynchronous call and will return after the state change. Any tasks resulting from the
             * state change are performed on a separate thread.
             * See {@code ServiceRuntimeComponent#EnableComponent}
             *
             * @throw std::out_of_range exception if the provided ComponentDescriptionDTO does not match
             * any of the known components in the runtime.
             */
            std::shared_future<void> EnableComponent(ComponentDescriptionDTO const& description) override;

            /**
             * This method changes the state of the component represented by the {@code ComponentDescriptionDTO}
             * to \c DISABLED and returns a future object
             * This is an asynchronous call and will return after the state change. Any tasks resulting from the
             * state change are performed on a separate thread.
             * See {@code ServiceRuntimeComponent#DisableComponent}
             *
             * @throw std::out_of_range exception if the provided ComponentDescriptionDTO does not match
             * any of the known components in the runtime.
             */
            std::shared_future<void> DisableComponent(ComponentDescriptionDTO const& description) override;

          private:
            FRIEND_TEST(ServiceComponentRuntimeImplTest, Validate_Ctor);

            ComponentDescriptionDTO CreateDTO(std::shared_ptr<ComponentManager> const& compManager) const;
            SatisfiedReferenceDTO CreateSatisfiedReferenceDTO(
                std::shared_ptr<ReferenceManager> const& refManager) const;
            UnsatisfiedReferenceDTO CreateUnsatisfiedReferenceDTO(
                std::shared_ptr<ReferenceManager> const& refManager) const;
            ComponentConfigurationDTO CreateComponentConfigurationDTO(
                std::shared_ptr<ComponentConfiguration> const& config) const;

            cppmicroservices::BundleContext scrContext;
            std::shared_ptr<ComponentRegistry> registry;
            std::shared_ptr<cppmicroservices::logservice::LogService> logger;
        };
    } // namespace scrimpl
} // namespace cppmicroservices

#endif /* SERVICECOMPONENTRUNTIMEIMPL_HPP */
