/**
 * @file      BindingPolicyDynamicGreedy.cpp
 * @copyright Copyright 2020 MathWorks, Inc. 
 */ 

#include "ReferenceManagerImpl.hpp"

namespace cppmicroservices { namespace scrimpl {

void ReferenceManagerBaseImpl::BindingPolicyDynamicGreedy::ServiceAdded(const ServiceReferenceBase& reference)
{
  if (!reference) {
    Log("ServiceAdded: service with id "
        + std::to_string(GetServiceId(reference))
        + " has already been unregistered, no-op");
    
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
      }
      else
      {
        // not sure this is right.
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

void ReferenceManagerBaseImpl::BindingPolicyDynamicGreedy::ServiceRemoved(const ServiceReferenceBase& reference)
{
  auto notifications = RemoveService(reference);
  mgr.BatchNotifyAllListeners(notifications);
}

}}
