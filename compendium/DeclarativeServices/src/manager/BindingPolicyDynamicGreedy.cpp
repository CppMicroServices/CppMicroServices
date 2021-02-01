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

namespace cppmicroservices {
namespace scrimpl {

/**
 * If no service is bound, bind to better target service.
 * If the service is higher ranking, bind the better target service
 *  and then unbind the bound service.
 * 
 */
void ReferenceManagerBaseImpl::BindingPolicyDynamicGreedy::ServiceAdded(
  const ServiceReferenceBase& reference)
{
  std::vector<RefChangeNotification> notifications;
  if (!reference) {
    Log("BindingPolicyDynamicGreedy::ServiceAdded called with an invalid "
        "service reference");
    return;
  }

  auto notifySatisfied = false;

  if (!mgr.IsSatisfied()) {
    notifySatisfied =
      mgr.UpdateBoundRefs(); // becomes satisfied if return value is true
  } else {
    // previously satisfied
    // either the service has a bound target service or not (in the case of optional cardinality)
    // rebind to the new target service if it is higher service rank.

    // is the new service ref a better target service?
    // if it is or there are no bound refs, rebind
    // otherwise, do nothing
    bool needRebind = false;
    ServiceReference<void> svcRefToUnBind;
    {
      auto boundRefsHandle = mgr.boundRefs.lock(); // acquires lock on boundRefs
      if (boundRefsHandle->empty()) {
        notifications.emplace_back(
          mgr.metadata.name, RefEvent::REBIND, reference);
      } else { // there are bound refs, determine whether to rebind
        svcRefToUnBind = *(boundRefsHandle->begin());
        needRebind = svcRefToUnBind < reference;
      }
    }

    ClearBoundRefs();
    mgr.UpdateBoundRefs();

    if (needRebind) {
      // The bind notification must happen before the unbind notification
      // to eliminate any gaps between unbinding the current bound target service
      // and binding to the new bound target service.
      notifications.push_back(RefChangeNotification {
        mgr.metadata.name, RefEvent::REBIND, reference, svcRefToUnBind });
    }
  }

  if (notifySatisfied) {
    Log("Notify SATISFIED for reference " + mgr.metadata.name);
    notifications.emplace_back(mgr.metadata.name, RefEvent::BECAME_SATISFIED);
  }
  mgr.BatchNotifyAllListeners(notifications);
}

void ReferenceManagerBaseImpl::BindingPolicyDynamicGreedy::ServiceRemoved(
  const ServiceReferenceBase& reference)
{
  DynamicRemoveService(reference);
}

}
}
