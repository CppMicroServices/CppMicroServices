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
  // check cardinality... if it's 1..1 or 0..1, do the same thing as StaticReluctant
  // else...
  //   update bound refs,
  //   perform bind/unbind callbacks
  std::vector<RefChangeNotification> notifications;
  if (1 == mgr.metadata.maxCardinality) {
    notifications = ReluctantServiceAdded(reference);
  } else // maxCardinality is greater than 1
  {
    ServiceReference<void> serviceToUnbind;
    bool replacementNeeded(false);
    {
      auto boundRefsHandle = mgr.boundRefs.lock(); // acquire lock on boundRefs
      if (boundRefsHandle->find(reference) == boundRefsHandle->end()) {
        if (!boundRefsHandle->empty()) {
          const ServiceReferenceBase& minBound = *(boundRefsHandle->begin());
          if (minBound < reference) {
            replacementNeeded = true;
            serviceToUnbind = minBound;
          }
        } else {
          replacementNeeded = mgr.IsOptional();
        }
      }
    }

    if (replacementNeeded) {
      mgr.UpdateBoundRefs();
      // bind reference
      notifications.push_back(
        { mgr.metadata.name, RefEvent::BIND_REFERENCE, reference });
      // unbind serviceToUnbind
      notifications.push_back(
        { mgr.metadata.name, RefEvent::UNBIND_REFERENCE, serviceToUnbind });
    }
  }
  mgr.BatchNotifyAllListeners(notifications);
}

void ReferenceManagerBaseImpl::BindingPolicyDynamicReluctant::ServiceRemoved(
  const ServiceReferenceBase& reference)
{
  std::vector<RefChangeNotification> notifications;
  if (1 == mgr.metadata.maxCardinality) {
    notifications = RemoveService(reference);
  } else {
    // do we need to replace a boundRef
    auto boundRefsHandle = mgr.boundRefs.lock();
    bool needsUnbind =
      (boundRefsHandle->find(reference) != boundRefsHandle->end());
    if (needsUnbind) {
      auto matchedRefsHandle = mgr.matchedRefs.lock();
      std::set<cppmicroservices::ServiceReferenceBase> notBound;

      std::set_difference(matchedRefsHandle->begin(),
                          matchedRefsHandle->end(),
                          boundRefsHandle->begin(),
                          boundRefsHandle->end(),
                          std::inserter(notBound, notBound.begin()));
      auto serviceToBind = *notBound.rbegin(); // best match not in boundRefs;

      // What do I do in the case that notBound is empty? That is, there's no more
      // services left leaving the current service in an incomplete state (that is,
      // there's not enough matching services to deal with the cardinality.
      boundRefsHandle->insert(serviceToBind);
      boundRefsHandle->erase(reference);
      notifications.push_back(
        { mgr.metadata.name, RefEvent::BIND_REFERENCE, serviceToBind });
      notifications.push_back(
        { mgr.metadata.name, RefEvent::UNBIND_REFERENCE, reference });
    }
  }
  mgr.BatchNotifyAllListeners(notifications);
}
}
}
