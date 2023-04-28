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

#include "CMEnabledState.hpp"
#include "../ComponentConfigurationFactory.hpp"
#include "../ComponentManagerImpl.hpp"
#include "CMDisabledState.hpp"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/SharedLibraryException.h"

namespace cppmicroservices
{
    namespace scrimpl
    {

        // if already in enabled state, simply return the existing future object. Equivalent to a no-op.
        std::shared_future<void>
        CMEnabledState::Enable(ComponentManagerImpl&)
        {
            return GetFuture();
        }

        std::shared_future<void>
        CMEnabledState::Disable(ComponentManagerImpl& cm)
        {
            auto currentState = shared_from_this(); // assume this object is the current state object
            return cm.PostAsyncEnabledToDisabled(currentState);
        }

        void
        CMEnabledState::CreateConfigurations(std::shared_ptr<const metadata::ComponentMetadata> compDesc,
                                             cppmicroservices::Bundle const& bundle,
                                             std::shared_ptr<ComponentRegistry> registry,
                                             std::shared_ptr<logservice::LogService> logger,
                                             std::shared_ptr<ConfigurationNotifier> configNotifier)
        {
            try
            {
                auto cc = ComponentConfigurationFactory::CreateConfigurationManager(compDesc,
                                                                                    bundle,
                                                                                    registry,
                                                                                    logger,
                                                                                    configNotifier);
               configurations.push_back(cc);
            }
            catch (cppmicroservices::SharedLibraryException const&)
            {
                throw;
            }
            catch (cppmicroservices::SecurityException const&)
            {
                throw;
            }
            catch (...)
            {
                logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                            "Failed to create component configuration",
                            std::current_exception());
            }
        }

        // wait for the task to finish creating configurations and return them
        std::vector<std::shared_ptr<ComponentConfiguration>>
        CMEnabledState::GetConfigurations(ComponentManagerImpl const& /*cm*/) const
        {
            GetFuture().get(); // wait for the task created in #CreateConfigurationsAsync
            // Note: Exceptions from the task associated with the future are captured and
            // logged on the other thread. See #CreateConfigurations.
            std::vector<std::shared_ptr<ComponentConfiguration>> retVec(configurations.begin(), configurations.end());
            return retVec;
        }

        void
        CMEnabledState::DeleteConfigurations()
        {
            auto fut = GetFuture();
            if (fut.valid())
            {
                fut.get(); // wait for the configurations to become available
                // No exceptions are expected from the future. Exceptions are
                // logged on the otherside of the thread boundary. See #CreateConfigurations
                auto configs = std::move(configurations);
                for (auto const& config : configs)
                {
                    config->Deactivate();
                    config->Stop();
                }
            }
        }
    } // namespace scrimpl
} // namespace cppmicroservices
