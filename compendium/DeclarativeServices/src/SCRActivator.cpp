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

#include "SCRActivator.hpp"
#include "ConfigurationListenerImpl.hpp"
#include "SCRAsyncWorkService.hpp"
#include "SCRLogger.hpp"
#include "ServiceComponentRuntimeImpl.hpp"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/SharedLibraryException.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "cppmicroservices/servicecomponent/runtime/dto/ComponentConfigurationDTO.hpp"
#include "cppmicroservices/servicecomponent/runtime/dto/ComponentDescriptionDTO.hpp"
#include "cppmicroservices/servicecomponent/runtime/dto/ReferenceDTO.hpp"
#include "manager/ComponentManager.hpp"
#include "manager/ReferenceManager.hpp"
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "cppmicroservices/detail/ScopeGuard.h"

using cppmicroservices::logservice::SeverityLevel;
using cppmicroservices::service::component::ComponentConstants::SERVICE_COMPONENT;

namespace cppmicroservices
{
    namespace scrimpl
    {

        void
        SCRActivator::Start(BundleContext context)
        {
            runtimeContext = context;

            // Create the component registry
            componentRegistry = std::make_shared<ComponentRegistry>();

            // Create the Logger object used by this runtime
            logger = std::make_shared<SCRLogger>(context);
            logger->Log(SeverityLevel::LOG_DEBUG, "Starting SCR bundle");

            asyncWorkService = std::make_shared<SCRAsyncWorkService>(context, logger);

            // Create SCRExtension Registry
            bundleRegistry = std::make_shared<SCRExtensionRegistry>(logger);

            // Create configuration object notifier
            configNotifier = std::make_shared<ConfigurationNotifier>(context, logger, asyncWorkService, bundleRegistry);
            activatorStopped = std::make_shared<bool>(false);
            notificationLock = std::make_shared<std::shared_mutex>();

            // Add bundle listener
            bundleListenerToken
                = context.AddBundleListener([this, activatorStoppedCopy = activatorStopped, notificationLockCopy = notificationLock] (cppmicroservices::BundleEvent const& evt) {                    ReadLock l(*notificationLockCopy);
                    if (*activatorStoppedCopy){
                        return;
                    }
                    this->BundleChanged(evt);
                });
            // HACK: Workaround for lack of Bundle Tracker. Iterate over all bundles and call the tracker method
            // manually
            for (auto const& bundle : context.GetBundles())
            {
                if (bundle.GetState() & cppmicroservices::Bundle::State::STATE_ACTIVE)
                {
                    cppmicroservices::BundleEvent evt(cppmicroservices::BundleEvent::BUNDLE_STARTED, bundle);
                    BundleChanged(evt);
                }
            }
            // Publish ServiceComponentRuntimeService
            auto service = std::make_shared<ServiceComponentRuntimeImpl>(runtimeContext, componentRegistry, logger);
            scrServiceReg = context.RegisterService<ServiceComponentRuntime>(std::move(service));

            // Publish ConfigurationListener
            auto configListener
                = std::make_shared<cppmicroservices::service::cm::ConfigurationListenerImpl>(runtimeContext,
                                                                                             logger,
                                                                                             configNotifier);
            configListenerReg = context.RegisterService<cppmicroservices::service::cm::ConfigurationListener>(
                std::move(configListener));
        }

        void
        SCRActivator::Stop(cppmicroservices::BundleContext context)
        {
            WriteLock l(*notificationLock);
            *activatorStopped = true;
            try
            {
                // remove the bundle listener
                context.RemoveListener(std::move(bundleListenerToken));
                // remove the runtime service from the framework
                scrServiceReg.Unregister();
                // remove the configuration listener service from the framework
                configListenerReg.Unregister();

                // dispose all components created by SCR
                auto const bundles = context.GetBundles();
                for (auto const& bundle : bundles)
                {
                    DisposeExtension(bundle);
                }

                // clear bundle registry
                bundleRegistry->Clear();

                // clear component registry
                componentRegistry->Clear();

                logger->Log(SeverityLevel::LOG_DEBUG, "SCR Bundle stopped.");
            }
            catch (...)
            {
                logger->Log(SeverityLevel::LOG_DEBUG,
                            "Exception while stopping the declarative services runtime bundle",
                            std::current_exception());
            }
            logger->StopTracking();
            asyncWorkService->StopTracking();
        }

        void
        SCRActivator::CreateExtension(cppmicroservices::Bundle const& bundle)
        {
            auto const& headers = bundle.GetHeaders();
            // bundle has no "scr" property
            if (headers.count(SERVICE_COMPONENT) == 0u)
            {
                logger->Log(SeverityLevel::LOG_DEBUG, "No SCR components found in bundle " + bundle.GetSymbolicName());
                return;
            }

            //create the extension which will load the components
            if (!bundleRegistry->Find(bundle.GetBundleId()))
            {
                logger->Log(SeverityLevel::LOG_DEBUG, "Creating SCRBundleExtension ... " + bundle.GetSymbolicName());
                try
                {
                    auto const& scrMap = ref_any_cast<cppmicroservices::AnyMap>(headers.at(SERVICE_COMPONENT));
                    auto ba = std::make_shared<SCRBundleExtension>(bundle,
                                                                   componentRegistry,
                                                                   logger,
                                                                   configNotifier);
                    bundleRegistry->Add(bundle.GetBundleId(), ba);
                    // Create the ComponentManagerImpl object to manage the component configurations.
                    ba->Initialize(scrMap, asyncWorkService);
                 }
                catch (cppmicroservices::SharedLibraryException const&)
                {
                    throw;
                }
                catch (cppmicroservices::SecurityException const&)
                {
                    throw;
                }
                catch (std::exception const&)
                {
                    logger->Log(SeverityLevel::LOG_DEBUG,
                                "Failed to create SCRBundleExtension for " + bundle.GetSymbolicName(),
                                std::current_exception());
                }
            }
            else
            {
                logger->Log(SeverityLevel::LOG_DEBUG,
                            "SCR components already loaded from bundle " + bundle.GetSymbolicName());
            }
        }

        void
        SCRActivator::DisposeExtension(cppmicroservices::Bundle const& bundle)
        {
            auto const& headers = bundle.GetHeaders();
            // bundle has no scr-component property
            if (headers.count(SERVICE_COMPONENT) == 0u)
            {
                logger->Log(SeverityLevel::LOG_DEBUG, "Found No SCR Metadata for " + bundle.GetSymbolicName());
                return;
            }

             if (bundleRegistry->Find(bundle.GetBundleId()))
            {
                logger->Log(SeverityLevel::LOG_DEBUG, "Found SCRBundleExtension for " + bundle.GetSymbolicName());
                // remove the bundle extension object from the map.
                bundleRegistry->Remove(bundle.GetBundleId());
            }
            else
            {
                logger->Log(SeverityLevel::LOG_DEBUG, "Found No SCRBundleExtension for " + bundle.GetSymbolicName());
            }
        }

        void
        SCRActivator::BundleChanged(cppmicroservices::BundleEvent const& evt)
        {
            auto bundle = evt.GetBundle();
            auto const eventType = evt.GetType();
            if (bundle == runtimeContext.GetBundle()) // skip events for this (runtime) bundle
            {
                return;
            }

            // TODO: revisit to include LAZY_ACTIVATION when supported by the framework
            if (eventType & cppmicroservices::BundleEvent::BUNDLE_STARTED)
            {
                CreateExtension(bundle);
            }
            else if (eventType & cppmicroservices::BundleEvent::BUNDLE_STOPPING)
            {
                DisposeExtension(bundle);
            }
            // else ignore
        }
    } // namespace scrimpl
} // namespace cppmicroservices

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(cppmicroservices::scrimpl::SCRActivator) // NOLINT
