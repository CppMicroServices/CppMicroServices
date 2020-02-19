/**
 * @file      BindingPolicyStaticReluctant.cpp
 * @copyright Copyright 2020 MathWorks, Inc. 
 */ 

#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "cppmicroservices/logservice/LogService.hpp"
#include "ReferenceManagerImpl.hpp"

namespace cppmicroservices { namespace scrimpl {

using namespace cppmicroservices::logservice;

void ReferenceManagerImpl::BindingPolicyStaticReluctant::ServiceAdded(ReferenceManagerImpl& mgr
                                                                      , const ServiceReferenceBase& reference)
{
  if(!reference)
  {
    mgr.logger->Log(SeverityLevel::LOG_DEBUG, "ServiceAdded: service with id " + std::to_string(GetServiceId(reference)) + " has already been unregistered, no-op");
    return;
  }
  auto notifySatisfied = false;

  if(!mgr.IsSatisfied())
  {
    notifySatisfied = mgr.UpdateBoundRefs(); // becomes satisfied if return value is true
  }

  std::vector<RefChangeNotification> notifications;
  if(notifySatisfied)
  {
    mgr.logger->Log(SeverityLevel::LOG_DEBUG, "Notify SATISFIED for reference " + mgr.metadata.name);
    notifications.push_back({ mgr.metadata.name, RefEvent::BECAME_SATISFIED, reference });
  }
  mgr.BatchNotifyAllListeners(notifications);
}

void ReferenceManagerImpl::BindingPolicyStaticReluctant::ServiceRemoved(ReferenceManagerImpl& mgr
                                                                        , const ServiceReferenceBase& reference)
{
  if(RemoveBoundRef(mgr, reference)) {
    mgr.logger->Log(SeverityLevel::LOG_DEBUG, "Notify UNSATISFIED for reference " + mgr.metadata.name);
    std::vector<RefChangeNotification> notifications;
    notifications.push_back({ mgr.metadata.name, RefEvent::BECAME_UNSATISFIED, reference });
    {
      auto boundRefsHandle = mgr.boundRefs.lock();
      boundRefsHandle->clear();
    }
    auto notifySatisfied = mgr.UpdateBoundRefs();
    if (notifySatisfied)
    {
      mgr.logger->Log(SeverityLevel::LOG_DEBUG, "Notify SATISFIED for reference " + mgr.metadata.name);
      notifications.push_back({ mgr.metadata.name, RefEvent::BECAME_SATISFIED, reference });
    }
    mgr.BatchNotifyAllListeners(notifications);
  }
}

}} // namespaces
