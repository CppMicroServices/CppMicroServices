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

#include "SCRBundleExtension.hpp"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/SharedLibraryException.h"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "manager/ComponentManagerImpl.hpp"
#include "manager/ConfigurationNotifier.hpp"
#include "metadata/ComponentMetadata.hpp"
#include "metadata/MetadataParser.hpp"
#include "metadata/MetadataParserFactory.hpp"
#include "metadata/Util.hpp"

using cppmicroservices::service::component::ComponentConstants::SERVICE_COMPONENT;

namespace cppmicroservices::scrimpl
{

    using metadata::ComponentMetadata;
    using util::ObjectValidator;

    SCRBundleExtension::SCRBundleExtension(cppmicroservices::Bundle const& bundle,
                                           std::shared_ptr<ComponentRegistry> const& registry,
                                           std::shared_ptr<LogService> const& logger,
                                           std::shared_ptr<ConfigurationNotifier> const& configNotifier)
        : bundle_(bundle)
        , registry(registry)
        , logger(logger)
        , configNotifier(configNotifier)
    {
        if (!bundle || !registry || !logger || !configNotifier)
        {
            throw std::invalid_argument("Invalid parameters passed to SCRBundleExtension constructor");
        }
    }

    void
    SCRBundleExtension::Initialize(cppmicroservices::AnyMap const& scrMetadata,
                                   std::shared_ptr<cppmicroservices::async::AsyncWorkService> const& asyncWorkService)
    {
        if (scrMetadata.empty() || !asyncWorkService)
        {
            throw std::invalid_argument("Invalid parameters passed to SCRBundleExtension::Initialize");
        }

        auto version = ObjectValidator(scrMetadata, "version").GetValue<int>();
        auto metadataparser = metadata::MetadataParserFactory::Create(version, logger);
        std::vector<std::shared_ptr<ComponentMetadata>> componentsMetadata;
        componentsMetadata = metadataparser->ParseAndGetComponentsMetadata(scrMetadata);
        for (auto& oneCompMetadata : componentsMetadata)
        {
            try
            {
                auto compManager = std::make_shared<ComponentManagerImpl>(oneCompMetadata,
                                                                          registry,
                                                                          bundle_.GetBundleContext(),
                                                                          logger,
                                                                          asyncWorkService,
                                                                          configNotifier);
                if (registry->AddComponentManager(compManager))
                {
                    {
                        std::unique_lock<std::mutex> l(managersMutex);
                        managers.push_back(compManager);
                    }
                    compManager->Initialize();
                }
            }
            catch (cppmicroservices::SharedLibraryException const&)
            {
                throw;
            }
            catch (cppmicroservices::SecurityException const&)
            {
                std::unique_lock<std::mutex> l(managersMutex);
                DisableAndRemoveAllComponentManagers();
                managers.clear();
                throw;
            }
            catch (std::exception const&)
            {
                logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                            "Failed to create ComponentManager with name " + oneCompMetadata->name
                                + " from bundle with Id " + std::to_string(bundle_.GetBundleId()),
                            std::current_exception());
            }
        }
        logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                    "Created instance of SCRBundleExtension for " + bundle_.GetSymbolicName());
    }

    SCRBundleExtension::~SCRBundleExtension()
    {
        {
            std::unique_lock<std::mutex> l(managersMutex);
            try
            {
                DisableAndRemoveAllComponentManagers();
            }
            catch (...)
            {
                logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING,
                            "Exception while removing component managers for bundle " + bundle_.GetSymbolicName(),
                            std::current_exception());
            }

            managers.clear();
        }
        registry.reset();
    }

    void
    SCRBundleExtension::DisableAndRemoveAllComponentManagers()
    {
        logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                    "Deleting instance of SCRBundleExtension for " + bundle_.GetSymbolicName());
        for (auto& compManager : managers)
        {
            auto singleInvoke = std::make_shared<SingleInvokeTask>();
            auto fut = compManager->Disable(singleInvoke);
            registry->RemoveComponentManager(compManager);
            try
            {
                // since this happens when the bundle is stopped,
                // wait until the disable is finished on the other thread.
                compManager->WaitForFuture(fut, singleInvoke);
            }
            catch (...)
            {
                std::string errMsg("An exception occurred while disabling "
                                   "component manager: ");
                errMsg += compManager->GetName();
                logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING, errMsg, std::current_exception());
            }
        }
    }
    void
    SCRBundleExtension::AddComponentManager(std::shared_ptr<ComponentManager> compManager)
    {
        std::unique_lock<std::mutex> l(managersMutex);
        managers.push_back(std::move(compManager));
    }

} // namespace cppmicroservices::scrimpl