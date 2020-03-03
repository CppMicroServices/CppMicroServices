/**
 * @file      BindingPolicyStaticGreedy.cpp
 * @copyright Copyright 2020 MathWorks, Inc. 
 */ 

#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "cppmicroservices/logservice/LogService.hpp"
#include "ReferenceManagerImpl.hpp"

namespace cppmicroservices { namespace scrimpl {

using namespace cppmicroservices::logservice;

void ReferenceManagerBaseImpl::BindingPolicyStaticGreedy::ServiceAdded(const ServiceReferenceBase& reference)
{
  if (!reference) {
    Log("ServiceAdded: service with id "
        + std::to_string(GetServiceId(reference))
        + " has already been unregistered, no-op");
    
    return;
  }
  
  auto replacementNeeded = false;
  ServiceReference<void> serviceToUnbind;
  if (mgr.IsSatisfied()) {
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

  auto notifySatisfied = ShouldNotifySatisfied();
  std::vector<RefChangeNotification> notifications;
  if (replacementNeeded) {
    Log("Notify UNSATISFIED for reference " + mgr.metadata.name);
    notifications.push_back({ mgr.metadata.name, RefEvent::BECAME_UNSATISFIED, reference });
    // The following "clear and copy" strategy is sufficient for
    // updating the boundRefs for static binding policy
    if (serviceToUnbind) {
      ClearBoundRefs();
    }
    notifySatisfied = mgr.UpdateBoundRefs();
  }
  if (notifySatisfied) {
    Log("Notify SATISFIED for reference " + mgr.metadata.name);
    notifications.push_back({mgr.metadata.name, RefEvent::BECAME_SATISFIED, reference });
  }
  mgr.BatchNotifyAllListeners(notifications);
}

void ReferenceManagerBaseImpl::BindingPolicyStaticGreedy::ServiceRemoved(const ServiceReferenceBase& reference)
{
  auto notifications = RemoveService(reference);
  mgr.BatchNotifyAllListeners(notifications);
}

}}
