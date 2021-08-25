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

namespace cppmicroservices {
namespace scrimpl {

CCActiveState::CCActiveState() = default;

std::shared_ptr<ComponentInstance> CCActiveState::Activate(
  ComponentConfigurationImpl& mgr,
  const cppmicroservices::Bundle& clientBundle)
{
  std::lock_guard<std::mutex> lock(oneAtATimeMutex);
  auto logger = mgr.GetLogger();

  // Make sure the state didn't change while we were waiting
  if (mgr.GetState()->GetValue() !=
      service::component::runtime::dto::ComponentState::ACTIVE) {
    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING,
                "Activate failed. Component no longer in Active State.");
    return nullptr;
  }
  // no state change, already in active state. create and return a ComponentInstance object
  std::shared_ptr<ComponentInstance> instance;
  instance = mgr.CreateAndActivateComponentInstance(clientBundle);

  // Just in case the configuration properties changed between Registration and
  // Construction of the component, update the properties in the service registration object.
  // An example of when this could happen is when immediate=false and configuration-policy
  // = optional. The component could be registered before all the configuration objects are
  // available but it won't be constructed until someone gets the service. In between those
  // two activities the configuration objects could change and the service registration properties
  // would be out of date.
  if (instance) {
    mgr.SetRegistrationProperties();
  }

  if (!instance) {
    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                "Component configuration activation failed");
  }
  return instance;
}

void CCActiveState::Deactivate(ComponentConfigurationImpl& mgr)
{
  std::lock_guard<std::mutex> lock(oneAtATimeMutex);

  // Make sure the state didn't change while we were waiting
  if (mgr.GetState()->GetValue() !=
      service::component::runtime::dto::ComponentState::ACTIVE) {
    auto logger = mgr.GetLogger();
    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                "Deactivate failed. Component no longer in Active State.");
    return;
  }
  DoDeactivateWork(mgr);
}

void CCActiveState::DoDeactivateWork(ComponentConfigurationImpl& mgr)
{
  auto currentState = shared_from_this();
  std::promise<void> transitionAction;
  auto fut = transitionAction.get_future();
  auto unsatisfiedState =
    std::make_shared<CCUnsatisfiedReferenceState>(std::move(fut));
  while (currentState->GetValue() !=
    service::component::runtime::dto::ComponentState::UNSATISFIED_REFERENCE) {
    if (mgr.CompareAndSetState(&currentState, unsatisfiedState)) {
      currentState
        ->WaitForTransitionTask(); // wait for the previous transition to finish
      mgr.UnregisterService();
      mgr.DestroyComponentInstances();
      transitionAction.set_value();
    }
  }
}

bool CCActiveState::Modified(ComponentConfigurationImpl& mgr)
{
  std::lock_guard<std::mutex> lock(oneAtATimeMutex);
  // Make sure the state didn't change while we were waiting
  if (mgr.GetState()->GetValue() !=
      service::component::runtime::dto::ComponentState::ACTIVE) {
    auto logger = mgr.GetLogger();
    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING,
                "Modified failed. Component no longer in Active State.");
    return false;
  }
  if (!mgr.ModifyComponentInstanceProperties()) {
    // Component instance does not have a Modified method. Deactivate
    // and reactivate
    DoDeactivateWork(mgr);
    // Service registration properties will be updated when the service is
    // registered. Don't need to do it here.
    return false;
  }
  // Update service registration properties
  mgr.SetRegistrationProperties();

  return true;
};

void CCActiveState::Rebind(ComponentConfigurationImpl& mgr,
                           const std::string& refName,
                           const ServiceReference<void>& svcRefToBind,
                           const ServiceReference<void>& svcRefToUnbind)
{
  std::lock_guard<std::mutex> lock(oneAtATimeMutex);
  // Make sure the state didn't change while we were waiting
  if (mgr.GetState()->GetValue() !=
      service::component::runtime::dto::ComponentState::ACTIVE) {
    auto logger = mgr.GetLogger();
    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING,
                "Rebind failed. Component no longer in Active State.");
    return;
  }
  if (svcRefToBind) {
    try {
      mgr.BindReference(refName, svcRefToBind);
    } catch (const std::exception&) {
      mgr.GetLogger()->Log(
        cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
        "Exception while dynamically binding a reference. ",
        std::current_exception());
    }
  }

  if (svcRefToUnbind) {
    try {
      mgr.UnbindReference(refName, svcRefToUnbind);
    } catch (const std::exception&) {
      mgr.GetLogger()->Log(
        cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
        "Exception while dynamically unbinding a reference. ",
        std::current_exception());
    }
  }
}
}
}
