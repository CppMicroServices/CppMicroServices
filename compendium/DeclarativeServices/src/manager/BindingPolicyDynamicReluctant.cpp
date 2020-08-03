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

  auto notifySatisfied = false;

  if (!mgr.IsSatisfied()) {
    notifySatisfied =
      mgr.UpdateBoundRefs(); // becomes satisfied if return value is true
  } else {
    // previously satisfied
    // If the service was previously satisfied then either there is
    // nothing to do or a rebind needs to happen if the cardinality
    // is optional and there are no bound refs.
    if (0 == mgr.GetBoundReferences().size()) {
      Log("Notify BIND for reference " + mgr.metadata.name);

      ClearBoundRefs();
      mgr.UpdateBoundRefs();

      notifications.emplace_back(
        mgr.metadata.name, RefEvent::REBIND, reference);
    }
  }

  if (notifySatisfied) {
    Log("Notify SATISFIED for reference " + mgr.metadata.name);
    notifications.emplace_back(mgr.metadata.name, RefEvent::BECAME_SATISFIED);
  }
  mgr.BatchNotifyAllListeners(notifications);
}

void ReferenceManagerBaseImpl::BindingPolicyDynamicReluctant::ServiceRemoved(
  const ServiceReferenceBase& reference)
{
  DynamicRemoveService(reference);
}
}
}
