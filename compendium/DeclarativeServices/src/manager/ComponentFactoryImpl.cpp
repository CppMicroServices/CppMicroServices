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

#include "ComponentFactoryImpl.hpp"
#include "../ComponentRegistry.hpp"
#include "../SCRExtensionRegistry.hpp"
#include "../metadata/ComponentMetadata.hpp"
#include "ComponentConfigurationImpl.hpp"
#include "ComponentManagerImpl.hpp"
#include "ConcurrencyUtil.hpp"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/SharedLibraryException.h"
#include "cppmicroservices/asyncworkservice/AsyncWorkService.hpp"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"
#include <regex>

namespace cppmicroservices::scrimpl
{
    std::string
    ReplacePlaceholdersInTarget(std::string targetExpression, cppmicroservices::AnyMap const& properties)
    {
        if (targetExpression.empty())
        {
            return targetExpression;
        }

        std::regex pattern(R"(\{\{([^{}]+)\}\})");
        std::string result;
        std::sregex_iterator currentMatch(targetExpression.begin(), targetExpression.end(), pattern);
        std::sregex_iterator lastMatch;

        size_t lastPos = 0;
        while (currentMatch != lastMatch)
        {
            std::smatch match = *currentMatch;
            result.append(targetExpression, lastPos, match.position() - lastPos);

            std::string key = match[1].str();
            if (auto iter = properties.find(key); iter != properties.end())
            {
                result.append(cppmicroservices::ref_any_cast<std::string>(iter->second));
            }
            else
            {
                throw std::invalid_argument("reference target not in configuration " + key);
            }

            lastPos = match.position() + match.length();
            ++currentMatch;
        }
        result.append(targetExpression, lastPos, std::string::npos);

        return result;
    }

    using cppmicroservices::scrimpl::metadata::ComponentMetadata;

    ComponentFactoryImpl::ComponentFactoryImpl(cppmicroservices::BundleContext const& context,
                                               std::shared_ptr<cppmicroservices::logservice::LogService> logger,
                                               std::shared_ptr<cppmicroservices::async::AsyncWorkService> asyncWorkSvc,
                                               std::shared_ptr<SCRExtensionRegistry> extensionReg)
        : bundleContext(context)
        , logger(std::move(logger))
        , asyncWorkService(asyncWorkSvc)
        , extensionRegistry(extensionReg)
    {
        if (!bundleContext || !(this->logger) || !(this->asyncWorkService) || !(this->extensionRegistry))
        {
            throw std::invalid_argument("ComponentFactoryImpl Constructor "
                                        "provided with invalid arguments");
        }
    }

    void
    ComponentFactoryImpl::CreateFactoryComponent(std::string const& pid,
                                                 std::shared_ptr<ComponentConfigurationImpl>& mgr,
                                                 cppmicroservices::AnyMap const& properties)
    {
        // Create the virtual metadata for the factory instance.
        // Start with the metadata from the factory
        auto const newMetadata = std::make_shared<ComponentMetadata>(*mgr->GetMetadata());

        newMetadata->name = newMetadata->name + "_" + pid;
        // this is a factory instance not a factory component
        newMetadata->factoryComponentID = "";

        // Factory instance is dependent on the factory instance pid
        newMetadata->configurationPids.clear();
        newMetadata->configurationPids.emplace_back(pid);

        // Look for dynamic targets in the references.
        // A dynamic target will appear in the properties for the configuration object
        // with the referenceName.target as the key and the target as the value.
        for (auto& ref : newMetadata->refsMetadata)
        {
            auto const target = ref.name + ".target";
            auto const iter = properties.find(target);

            // if manually injecting target in using refName.target, override existing
            if (iter != properties.end())
            {
                // This reference has a dynamic target
                ref.target = cppmicroservices::ref_any_cast<std::string>(iter->second);
            }
            else
            {
                // look for targets that are dependent on configuration. They key into the configuration will be
                // {{KEY}}
                ref.target = ReplacePlaceholdersInTarget(ref.target, properties);
            }
            // Verify that the ref.target is a valid LDAPFilter
            if (!ref.target.empty())
            {
                try
                {
                    LDAPFilter const filter(ref.target);
                }
                catch (std::exception const& e)
                {
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                "CreateFactoryComponent failed because of invalid target ldap filter"
                                    + newMetadata->name + " target= " + ref.target);
                    throw std::invalid_argument(e.what());
                }
            }
        }

        auto const bundle = mgr->GetBundle();
        auto const registry = mgr->GetRegistry();
        auto const logger = mgr->GetLogger();
        auto const configNotifier = mgr->GetConfigNotifier();
        try
        {
            auto const compManager = std::make_shared<ComponentManagerImpl>(newMetadata,
                                                                            registry,
                                                                            bundle.GetBundleContext(),
                                                                            logger,
                                                                            asyncWorkService,
                                                                            configNotifier);
            if (registry->AddComponentManager(compManager))
            {
                if (auto const& extension = extensionRegistry->Find(bundle.GetBundleId()); extension)
                {
                    extension->AddComponentManager(compManager);
                    compManager->Initialize();
                }
                else
                {
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                "Failed to find ComponentManager with name " + newMetadata->name
                                    + " from bundle with Id "
                                    + std::to_string(bundleContext.GetBundle().GetBundleId()));
                }
            }
        }
        catch (cppmicroservices::SharedLibraryException const&)
        {
            throw;
        }
        catch (cppmicroservices::SecurityException const&)
        {
            throw;
        }
    }

} // namespace cppmicroservices::scrimpl
