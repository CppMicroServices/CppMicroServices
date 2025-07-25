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

#include "CCActiveState.hpp"
#include "../ComponentConfigurationImpl.hpp"
#include "CCUnsatisfiedReferenceState.hpp"
#include "cppmicroservices/SharedLibraryException.h"
#include "cppmicroservices/detail/ScopeGuard.h"

namespace cppmicroservices
{
    namespace scrimpl
    {

        CCActiveState::CCActiveState() = default;

        std::shared_ptr<ComponentInstance>
        CCActiveState::Activate(ComponentConfigurationImpl& mgr, cppmicroservices::Bundle const& clientBundle)
        {
            std::shared_ptr<ComponentInstance> instance;
            auto logger = mgr.GetLogger();
            if (latch.CountUp())
            {
                {
                    detail::ScopeGuard sg(
                        [this, logger]()
                        {
                            // By using try/catch here, we ensure that this lambda function doesn't
                            // throw inside LatchScopeGuard's dtor.
                            try
                            {
                                latch.CountDown();
                            }
                            catch (...)
                            {
                                logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                            "latch.CountDown() threw an exception during "
                                            "LatchScopeGuard cleanup.",
                                            std::current_exception());
                            }
                        });
                    std::lock_guard<std::recursive_mutex> lock(oneAtATimeMutex);

                    // no state change, already in active state. create and return a ComponentInstance object
                    // This could throw; a scope guard is put in place to call latch.CountDown().
                    instance = mgr.CreateAndActivateComponentInstance(clientBundle);

                    // Just in case the configuration properties changed between Registration and
                    // Construction of the component, update the properties in the service registration object.
                    // An example of when this could happen is when immediate=false and configuration-policy
                    // = optional. The component could be registered before all the configuration objects are
                    // available but it won't be constructed until someone gets the service. In between those
                    // two activities the configuration objects could change and the service registration properties
                    // would be out of date.
                    if (instance)
                    {
                        mgr.SetRegistrationProperties();
                    }
                }
                if (!instance)
                {
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                "Component configuration activation failed");
                }
                return instance;
            }
            // do not allow any new component instances to be created if Deactivate was called
            logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
                        "Component configuration activation failed because component is "
                        "not in active state");
            return nullptr;
        }
        void
        CCActiveState::Deactivate(ComponentConfigurationImpl& mgr)
        {
            auto currentState = shared_from_this();
            std::promise<void> transitionAction;
            auto fut = transitionAction.get_future();
            auto unsatisfiedState = std::make_shared<CCUnsatisfiedReferenceState>(std::move(fut));
            while (currentState->GetValue() != service::component::runtime::dto::ComponentState::UNSATISFIED_REFERENCE)
            {
                if (mgr.CompareAndSetState(&currentState, unsatisfiedState))
                {
                    // The currentState is CCActiveState so the WaitForTransitionTask is the version
                    // that waits for the latch. The Deactivate function won't continue until
                    // the latch counts down to 0, thereby allowing all Activate, Rebind and Modified
                    // activities to complete.
                    currentState->WaitForTransitionTask(); // wait for the previous transition to finish
                    try
                    {
                        mgr.UnregisterService();
                    }
                    catch (...)
                    {
                        mgr.GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING,
                                             "Failed to unregister the service.",
                                             std::current_exception());
                    }
                    mgr.DestroyComponentInstances();
                    transitionAction.set_value();
                }
            }
        }

        bool
        CCActiveState::Modified(ComponentConfigurationImpl& mgr)
        {
            auto logger = mgr.GetLogger();
            if (latch.CountUp())
            {
                detail::ScopeGuard sg(
                    [this, logger]()
                    {
                        // By using try/catch here, we ensure that this lambda function doesn't
                        // throw inside LatchScopeGuard's dtor.
                        try
                        {
                            latch.CountDown();
                        }
                        catch (...)
                        {
                            logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                        "latch.CountDown() threw an exception during "
                                        "LatchScopeGuard cleanup in CCActiveState::Modified.",
                                        std::current_exception());
                        }
                    });

                std::lock_guard<std::recursive_mutex> lock(oneAtATimeMutex);
                // Make sure the state didn't change while we were waiting
                auto currentState = mgr.GetState();
                if (currentState->GetValue() != service::component::runtime::dto::ComponentState::ACTIVE)
                {
                    auto logger = mgr.GetLogger();
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING,
                                "Modified failed. Component no longer in Active State.");
                    return false;
                }
                bool result = mgr.ModifyComponentInstanceProperties();
                if (result)
                {
                    // Update service registration properties
                    mgr.SetRegistrationProperties();
                    return true;
                }
            } // count down the latch and release the lock
            Deactivate(mgr);
            // Service registration properties will be updated when the service is
            // registered. Don't need to do it here.
            return false;
        };

        void
        CCActiveState::Rebind(ComponentConfigurationImpl& mgr,
                              std::string const& refName,
                              ServiceReference<void> const& svcRefToBind,
                              ServiceReference<void> const& svcRefToUnbind)
        {
            auto logger = mgr.GetLogger();
            if (latch.CountUp())
            {
                detail::ScopeGuard sg(
                    [this, logger]()
                    {
                        // By using try/catch here, we ensure that this lambda function doesn't
                        // throw inside LatchScopeGuard's dtor.
                        try
                        {
                            latch.CountDown();
                        }
                        catch (...)
                        {
                            logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                        "latch.CountDown() threw an exception during "
                                        "LatchScopeGuard cleanup in CCActiveState::Rebind.",
                                        std::current_exception());
                        }
                    });
                std::lock_guard<std::recursive_mutex> lock(oneAtATimeMutex);
                // Make sure the state didn't change while we were waiting
                auto currentState = mgr.GetState();
                if (currentState->GetValue() != service::component::runtime::dto::ComponentState::ACTIVE)
                {
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING,
                                "Rebind failed. Component no longer in Active State.");
                    return;
                }
                if (svcRefToBind)
                {
                    try
                    {
                        mgr.BindReference(refName, svcRefToBind);
                    }
                    catch (std::exception const&)
                    {
                        mgr.GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                             "Exception while dynamically binding a reference. ",
                                             std::current_exception());
                    }
                }

                if (svcRefToUnbind)
                {
                    try
                    {
                        mgr.UnbindReference(refName, svcRefToUnbind);
                    }
                    catch (std::exception const&)
                    {
                        mgr.GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                             "Exception while dynamically unbinding a reference. ",
                                             std::current_exception());
                    }
                }
            }
        }
    } // namespace scrimpl
} // namespace cppmicroservices
