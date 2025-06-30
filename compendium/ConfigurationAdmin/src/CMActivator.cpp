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

#include <functional>
#include <stdexcept>

#include "CMActivator.hpp"
#include "CMConstants.hpp"

using cppmicroservices::logservice::SeverityLevel;

namespace cppmicroservices::cmimpl
{
    void
    CMActivator::Start(BundleContext context)
    {
        runtimeContext = context;
        // Create the Logger object used by this runtime
        logger = std::make_shared<CMLogger>(context);
        logger->Log(SeverityLevel::LOG_DEBUG, "Starting CM bundle");
        // Create the AsyncWorkService object used by this runtime
        asyncWorkService = std::make_shared<CMAsyncWorkService>(context, logger);
        // Create ConfigurationAdminImpl
        configAdminImpl = std::make_shared<ConfigurationAdminImpl>(runtimeContext, logger, asyncWorkService);
        activatorStopped = std::make_shared<bool>(false);
        notificationLock = std::make_shared<std::shared_mutex>();
        // Add bundle listener
        bundleListenerToken = context.AddBundleListener(
            [this, activatorStoppedCopy = activatorStopped, notificationLockCopy = notificationLock](
                cppmicroservices::BundleEvent const& evt)
            {
                ReadLock l(*notificationLockCopy);
                if (*activatorStoppedCopy)
                {
                    return;
                }
                this->BundleChanged(evt);
            });
        // HACK: Workaround for lack of Bundle Tracker. Iterate over all bundles and call the tracker method
        // manually
        for (auto const& bundle : context.GetBundles())
        {
            if (cppmicroservices::Bundle::State::STATE_ACTIVE & bundle.GetState())
            {
                cppmicroservices::BundleEvent evt(cppmicroservices::BundleEvent::BUNDLE_STARTED, bundle);
                BundleChanged(evt);
            }
        }
        // Publish ConfigurationAdmin service
        configAdminReg = context.RegisterService<cppmicroservices::service::cm::ConfigurationAdmin>(configAdminImpl);
    }

    void
    CMActivator::Stop(cppmicroservices::BundleContext context)
    {
        WriteLock l(*notificationLock);
        *activatorStopped = true;
        try
        {
            // remove the bundle listener
            context.RemoveListener(std::move(bundleListenerToken));
            // remove the runtime service from the framework
            configAdminReg.Unregister();
            // clear bundle registry
            {
                std::lock_guard<std::mutex> l(bundleRegMutex);
                bundleRegistry.clear();
            }
            // Clean up the ConfigurationAdminImpl
            // WAITFOR all configAdmin work to stop
            // dont queue any new work and wait for existing work
            configAdminImpl->StopAndWaitForAllAsync();
            configAdminImpl = nullptr;
            logger->Log(SeverityLevel::LOG_DEBUG, "CM Bundle stopped.");
        }
        catch (...)
        {
            logger->Log(SeverityLevel::LOG_DEBUG, "Exception while stopping the CM bundle", std::current_exception());
        }
        asyncWorkService->StopTracking();
        logger = nullptr;
        runtimeContext = nullptr;
    }

    void
    CMActivator::CreateExtension(cppmicroservices::Bundle const& bundle)
    {
        auto const& headers = bundle.GetHeaders();
        // bundle has no "cm" configuration
        if (headers.find(CMConstants::CM_KEY) == std::end(headers))
        {
            logger->Log(SeverityLevel::LOG_DEBUG, "No CM Configuration found in bundle " + bundle.GetSymbolicName());
            return;
        }

        auto extensionFound = false;
        {
            std::lock_guard<std::mutex> l(bundleRegMutex);
            extensionFound = (bundleRegistry.find(bundle.GetBundleId()) != std::end(bundleRegistry));
        }
        // This bundle's configuration has not been loaded, so create the extension which will load it
        if (extensionFound)
        {
            logger->Log(SeverityLevel::LOG_DEBUG,
                        "CM Configuration already loaded from bundle " + bundle.GetSymbolicName());
            return;
        }

        logger->Log(SeverityLevel::LOG_DEBUG, "Creating CMBundleExtension ... " + bundle.GetSymbolicName());
        try
        {
            auto const& cmMetadata
                = cppmicroservices::ref_any_cast<cppmicroservices::AnyMap>(headers.at(CMConstants::CM_KEY));
            auto be
                = std::make_unique<CMBundleExtension>(bundle.GetBundleContext(), cmMetadata, configAdminImpl, logger);
            {
                std::lock_guard<std::mutex> l(bundleRegMutex);
                bundleRegistry.emplace(bundle.GetBundleId(), std::move(be));
            }
        }
        catch (std::exception const&)
        {
            logger->Log(SeverityLevel::LOG_WARNING,
                        "Failed to create CMBundleExtension for " + bundle.GetSymbolicName(),
                        std::current_exception());
        }
    }

    void
    CMActivator::RemoveExtension(cppmicroservices::Bundle const& bundle)
    {
        auto const& headers = bundle.GetHeaders();
        // bundle has no "cm" configuration
        if (headers.find(CMConstants::CM_KEY) == std::end(headers))
        {
            logger->Log(SeverityLevel::LOG_DEBUG, "No CM Configuration found in bundle " + bundle.GetSymbolicName());
            return;
        }

        bool extensionFound = false;
        {
            std::lock_guard<std::mutex> l(bundleRegMutex);
            auto extensionIt = bundleRegistry.find(bundle.GetBundleId());
            if (extensionIt != std::end(bundleRegistry))
            {
                bundleRegistry.erase(extensionIt);
                extensionFound = true;
            }
        }
        if (extensionFound)
        {
            logger->Log(SeverityLevel::LOG_DEBUG, "Removed CMBundleExtension for " + bundle.GetSymbolicName());
            return;
        }
        logger->Log(SeverityLevel::LOG_DEBUG, "Found no CMBundleExtension for " + bundle.GetSymbolicName());
    }

    void
    CMActivator::BundleChanged(cppmicroservices::BundleEvent const& evt)
    {
        auto bundle = evt.GetBundle();
        auto const eventType = evt.GetType();
        if (bundle == runtimeContext.GetBundle()) // skip events for this (runtime) bundle
        {
            return;
        }

        // TODO: revisit to include LAZY_ACTIVATION when supported by the framework
        if (cppmicroservices::BundleEvent::BUNDLE_STARTED & eventType)
        {
            CreateExtension(bundle);
        }
        else if (cppmicroservices::BundleEvent::BUNDLE_STOPPING & eventType)
        {
            RemoveExtension(bundle);
        }
        // else ignore
    }
} // namespace cppmicroservices::cmimpl

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(cppmicroservices::cmimpl::CMActivator) // NOLINT
