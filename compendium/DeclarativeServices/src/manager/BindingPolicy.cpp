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

#include "ReferenceManagerImpl.hpp"
#include "cppmicroservices/logservice/LogService.hpp"

namespace cppmicroservices {
namespace scrimpl {

void ReferenceManagerBaseImpl::BindingPolicy::Log(
  std::string&& logStr,
  cppmicroservices::logservice::SeverityLevel logLevel)
{
  mgr.logger->Log(logLevel, logStr);
}

bool ReferenceManagerBaseImpl::BindingPolicy::ShouldClearBoundRefs(
  const ServiceReferenceBase& reference)
{
  auto boundRefsHandle = mgr.boundRefs.lock();
  auto itr = boundRefsHandle->find(reference);
  return (itr != boundRefsHandle->end());
}

bool ReferenceManagerBaseImpl::BindingPolicy::ShouldNotifySatisfied()
{
  return (mgr.IsSatisfied() ? false : mgr.UpdateBoundRefs());
}

void ReferenceManagerBaseImpl::BindingPolicy::ClearBoundRefs()
{
  auto boundRefsHandle = mgr.boundRefs.lock();
  boundRefsHandle->clear();
}

void ReferenceManagerBaseImpl::BindingPolicy::GreedyServiceAdded(
  const ServiceReferenceBase& reference)
{
  if (!reference) {
    Log("ServiceAdded: service with id " +
        std::to_string(GetServiceId(reference)) +
        " has already been unregistered, no-op");

    return;
  }

  // If no service is bound, bind to better target service. Otherwise, unbind the bound
  // service and bind the better target service.

  // else Unbind the bound service, then bind the new service.

  auto replacementNeeded = false;
  ServiceReference<void> serviceToUnbind;
  if (mgr.IsSatisfied())
  // means that a service is bound because the refmgr is satisfied.
  {

    auto boundRefsHandle = mgr.boundRefs.lock(); // acquire lock on boundRefs
    if (boundRefsHandle->find(reference) == boundRefsHandle->end())
    // Means that reference is to a different service than what's bound, so unbind the
    // old service and then bind to the new service.
    {
      if (!boundRefsHandle->empty())
      // We only need to unbind if there's actually a bound ref.
      {
        const ServiceReferenceBase& minBound = *(boundRefsHandle->begin());
        if (minBound < reference)
        // And we only need to unbind if the new reference is a better match than the
        // current best match (i.e. boundRefs are stored in reverse order with the best
        // match in the first position).
        {
          replacementNeeded = true;
          serviceToUnbind = minBound; // remember which service to unbind.
        }
      } else {
        // not sure this is right.
        replacementNeeded = mgr.IsOptional();
      }
    }
  }

  auto notifySatisfied = ShouldNotifySatisfied();
  std::vector<RefChangeNotification> notifications;
  if (replacementNeeded) {
    Log("Notify UNSATISFIED for reference " + mgr.metadata.name);
    notifications.push_back(
      { mgr.metadata.name, RefEvent::BECAME_UNSATISFIED, reference });
    // The following "clear and copy" strategy is sufficient for
    // updating the boundRefs for static binding policy
    if (serviceToUnbind) {
      ClearBoundRefs();
    }
    notifySatisfied = mgr.UpdateBoundRefs();
  }
  if (notifySatisfied) {
    Log("Notify SATISFIED for reference " + mgr.metadata.name);
    notifications.push_back(
      { mgr.metadata.name, RefEvent::BECAME_SATISFIED, reference });
  }
  mgr.BatchNotifyAllListeners(notifications);
}

std::vector<RefChangeNotification>
ReferenceManagerBaseImpl::BindingPolicy::ReluctantServiceAdded(
  const ServiceReferenceBase& reference)
{
  std::vector<RefChangeNotification> notifications;
  if (!reference) {
    Log("ServiceAdded: service with id " +
        std::to_string(GetServiceId(reference)) +
        " has already been unregistered, no-op");

  } else {
    auto notifySatisfied = ShouldNotifySatisfied();
    if (notifySatisfied) {
      Log("Notify SATISFIED for reference " + mgr.metadata.name);
      notifications.push_back(
        { mgr.metadata.name, RefEvent::BECAME_SATISFIED, reference });
    }
  }
  return notifications;
}

std::vector<RefChangeNotification>
ReferenceManagerBaseImpl::BindingPolicy::RemoveService(
  const ServiceReferenceBase& reference)
{
  std::vector<RefChangeNotification> notifications;
  if (ShouldClearBoundRefs(reference)) {
    Log("Notify UNSATISFIED for reference " + mgr.metadata.name);
    notifications.push_back(
      { mgr.metadata.name, RefEvent::BECAME_UNSATISFIED, reference });

    ClearBoundRefs();
    if (mgr.UpdateBoundRefs()) {
      Log("Notify SATISFIED for reference " + mgr.metadata.name);
      notifications.push_back(
        { mgr.metadata.name, RefEvent::BECAME_SATISFIED, reference });
    }
  }
  return notifications;
}
}
}
