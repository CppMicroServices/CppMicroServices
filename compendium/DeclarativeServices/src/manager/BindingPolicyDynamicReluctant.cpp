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

using namespace cppmicroservices::logservice;

void ReferenceManagerBaseImpl::BindingPolicyDynamicReluctant::ServiceAdded(
  const ServiceReferenceBase& reference)
{
  std::vector<RefChangeNotification> notifications;
  if (!reference) {
    Log("BindingPolicyDynamicReluctant::ServiceAdded called with an invalid "
        "service reference");
    return;
  }

  auto replacementNeeded = false;
  auto notifySatisfied = false;

  if (!mgr.IsSatisfied()) {
    notifySatisfied =
      mgr.UpdateBoundRefs(); // becomes satisfied if return value is true
  } else {
    // previously satisfied
    // If the service was previously satisfied then either there is
    // nothing to do or a rebind needs to happen if the cardinality
    // is optional and there are no bound refs.
    if (mgr.IsOptional() && 0 == mgr.GetBoundReferences().size()) {
      Log("Notify BIND for reference " + mgr.metadata.name);

      ClearBoundRefs();
      mgr.UpdateBoundRefs();

      RefChangeNotification notification{ mgr.metadata.name,
                                          RefEvent::BIND,
                                          reference };
      notifications.push_back(std::move(notification));
    }
  }

  if (notifySatisfied) {
    Log("Notify SATISFIED for reference " + mgr.metadata.name);
    RefChangeNotification notification{ mgr.metadata.name,
                                        RefEvent::BECAME_SATISFIED };
    notifications.push_back(std::move(notification));
  }
  mgr.BatchNotifyAllListeners(notifications);
}

/**
 *  If the removed service is found in the #boundRefs
 *   clear the #boundRefs member
 *   copy #matchedRefs to #boundRefs
 *   If #matchedRefs is not empty
 *     send a BIND notification to listeners
 *     send an UNBIND notification to listeners
 *   else if the service reference is not optional
 *     send an UNSATISFIED notification to listeners
 *   endif
 * endif
 */
void ReferenceManagerBaseImpl::BindingPolicyDynamicReluctant::ServiceRemoved(
  const ServiceReferenceBase& reference)
{
  // OSGi Compendium Release 7 section 112.5.12 Bound Service Replacement
  //  If an active component configuration has a dynamic reference with unary
  //  cardinality and the bound service is modified or unregistered and ceases
  //  to be a target service, or the policy-option is greedy and a better
  //  target service becomes available then SCR must attempt to replace the
  //  bound service with a new bound service.
  auto removeBoundRef = false;
  std::vector<RefChangeNotification> notifications;

  { // acquire lock on boundRefs
    auto boundRefsHandle = mgr.boundRefs.lock();
    auto itr = boundRefsHandle->find(reference);
    removeBoundRef = (itr != boundRefsHandle->end());
  } // end lock on boundRefs

  if (removeBoundRef) {
    ClearBoundRefs();

    auto notifyRebind = mgr.UpdateBoundRefs();

    if (notifyRebind) {
      // The bind notification must happen before the unbind notification
      // to eliminate any gaps between unbinding the current bound target service
      // and binding to the new bound target service.
      bool needRebind = false;
      ServiceReference<void> svcRefToBind;
      {
        auto boundRefsHandle = mgr.boundRefs.lock(); // acquires lock on boundRefs
        if (0 < boundRefsHandle->size()) {
          svcRefToBind = *(boundRefsHandle->begin());
          needRebind = true;
        }
      }

      if (needRebind) {
        Log("Notify BIND for reference " + mgr.metadata.name);
        notifications.push_back(RefChangeNotification{
          mgr.metadata.name, RefEvent::BIND, svcRefToBind });
      }

      Log("Notify UNBIND for reference " + mgr.metadata.name);
        notifications.push_back(RefChangeNotification{
        mgr.metadata.name, RefEvent::UNBIND, reference });
    } else if (!mgr.IsSatisfied()) {
      Log("Notify UNSATISFIED for reference " + mgr.metadata.name);
      RefChangeNotification notification{ mgr.metadata.name,
                                          RefEvent::BECAME_UNSATISFIED };
      notifications.push_back(std::move(notification));
    }
    mgr.BatchNotifyAllListeners(notifications);
  }
}
}
}
