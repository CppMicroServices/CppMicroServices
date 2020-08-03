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
#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/logservice/LogService.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"

namespace cppmicroservices {
namespace scrimpl {

using namespace cppmicroservices::logservice;

void ReferenceManagerBaseImpl::BindingPolicyStaticGreedy::ServiceAdded(
  const ServiceReferenceBase& reference)
{
  if (!reference) {
    Log("BindingPolicyStaticGreedy::ServiceAdded called with an invalid "
        "service reference");
    return;
  }

  // If no service is bound, reactivate the component to bind to the better target service.
  // Otherwise, check if the service is a higher rank and if so, reactivate the component to bind
  // to the better target service

  auto replacementNeeded = false;
  ServiceReference<void> serviceToUnbind;
  if (mgr.IsSatisfied()) {
    // either a service is bound or the service reference is optional
    auto boundRefsHandle = mgr.boundRefs.lock(); // acquire lock on boundRefs
    if (boundRefsHandle->find(reference) == boundRefsHandle->end()) {
      // Means that reference is to a different service than what's bound, so unbind the
      // old service and then bind to the new service.
      if (!boundRefsHandle->empty()) {
        // We only need to unbind if there's actually a bound ref.
        const ServiceReferenceBase& minBound = *(boundRefsHandle->begin());
        if (minBound < reference) {
          // And we only need to unbind if the new reference is a better match than the
          // current best match (i.e. boundRefs are stored in reverse order with the best
          // match in the first position).
          replacementNeeded = true;
          serviceToUnbind = minBound; // remember which service to unbind.
        }
      } else {
        // A replacement is needed if there are no bounds refs
        // and the service reference is optional.
        replacementNeeded = mgr.IsOptional();
      }
    }
  }

  auto notifySatisfied = ShouldNotifySatisfied();
  std::vector<RefChangeNotification> notifications;
  if (replacementNeeded) {
    Log("Notify UNSATISFIED for reference " + mgr.metadata.name);
    notifications.emplace_back(
      mgr.metadata.name, RefEvent::BECAME_UNSATISFIED, reference);
    // The following "clear and copy" strategy is sufficient for
    // updating the boundRefs for static binding policy
    if (serviceToUnbind) {
      ClearBoundRefs();
    }
    notifySatisfied = mgr.UpdateBoundRefs();
  }
  if (notifySatisfied) {
    Log("Notify SATISFIED for reference " + mgr.metadata.name);
    notifications.emplace_back(
      mgr.metadata.name, RefEvent::BECAME_SATISFIED, reference);
  }
  mgr.BatchNotifyAllListeners(notifications);
}

void ReferenceManagerBaseImpl::BindingPolicyStaticGreedy::ServiceRemoved(
  const ServiceReferenceBase& reference)
{
  StaticRemoveService(reference);
}

}
}
