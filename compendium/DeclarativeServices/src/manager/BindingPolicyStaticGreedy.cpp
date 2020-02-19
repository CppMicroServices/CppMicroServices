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

void ReferenceManagerImpl::BindingPolicyStaticGreedy::ServiceAdded(ReferenceManagerImpl& mgr
                                                                   , const ServiceReferenceBase& reference)
{
  if (!reference) {
    mgr.logger->Log(SeverityLevel::LOG_DEBUG
                    , "ServiceAdded: service with id "
                    + std::to_string(GetServiceId(reference))
                    + " has already been unregistered, no-op");
    return;
  }
  
  auto replacementNeeded = false;
  auto notifySatisfied = false;
  auto serviceIdToUnbind = -1;
  if (!mgr.IsSatisfied()) {
    notifySatisfied = mgr.UpdateBoundRefs(); // becomes satisfied if return value is true
  } else {
    auto boundRefsHandle = mgr.boundRefs.lock(); // acquire lock on boundRefs
    if (boundRefsHandle->find(reference) == boundRefsHandle->end()) {
      if (!boundRefsHandle->empty()) {
        const ServiceReferenceBase& minBound = *(boundRefsHandle->begin());
        if (minBound < reference) {
          replacementNeeded = true;
          serviceIdToUnbind = GetServiceId(minBound);
        }
      } else {
        replacementNeeded = mgr.IsOptional();
      }
    }
  }

  std::vector<RefChangeNotification> notifications;
  if (replacementNeeded) {
    mgr.logger->Log(SeverityLevel::LOG_DEBUG, "Notify UNSATISFIED for reference " + mgr.metadata.name);
    notifications.push_back({ mgr.metadata.name, RefEvent::BECAME_UNSATISFIED, reference });
    // The following "clear and copy" strategy is sufficient for
    // updating the boundRefs for static binding policy
    if(0 < serviceIdToUnbind) {
      auto boundRefsHandle = mgr.boundRefs.lock();
      boundRefsHandle->clear();
    }
    notifySatisfied = mgr.UpdateBoundRefs();
  }
  if (notifySatisfied) {
    mgr.logger->Log(SeverityLevel::LOG_DEBUG, "Notify SATISFIED for reference " + mgr.metadata.name);
    notifications.push_back({mgr.metadata.name, RefEvent::BECAME_SATISFIED, reference });
  }
  mgr.BatchNotifyAllListeners(notifications);
}

void ReferenceManagerImpl::BindingPolicyStaticGreedy::ServiceRemoved(ReferenceManagerImpl& mgr
                                                                     , const ServiceReferenceBase& reference)
{
  if (RemoveBoundRef(mgr, reference)) {
    mgr.logger->Log(SeverityLevel::LOG_DEBUG, "Notify UNSATISFIED for reference " + mgr.metadata.name);

    std::vector<RefChangeNotification> notifications;
    notifications.push_back({ mgr.metadata.name, RefEvent::BECAME_UNSATISFIED, reference });
    {
      auto boundRefsHandle = mgr.boundRefs.lock();
      boundRefsHandle->clear();
    }
    
    if(mgr.UpdateBoundRefs()) {
      mgr.logger->Log(SeverityLevel::LOG_DEBUG, "Notify SATISFIED for reference " + mgr.metadata.name);
      notifications.push_back({ mgr.metadata.name, RefEvent::BECAME_SATISFIED, reference });
    }
    mgr.BatchNotifyAllListeners(notifications);
  }
}

}}
